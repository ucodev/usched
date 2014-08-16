/**
 * @file entry.c
 * @brief uSched
 *        Entry handling interface - Daemon
 *
 * Date: 16-08-2014
 * 
 * Copyright 2014 Pedro A. Hortas (pah@ucodev.org)
 *
 * This file is part of usched.
 *
 * usched is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * usched is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with usched.  If not, see <http://www.gnu.org/licenses/>.
 *
 */


#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <mqueue.h>
#include <unistd.h>

#include <sys/types.h>

#include <psec/crypt.h>

#include "debug.h"
#include "config.h"
#include "usched.h"
#include "runtime.h"
#include "mm.h"
#include "entry.h"
#include "log.h"
#include "auth.h"
#include "conn.h"
#include "schedule.h"

static int _entry_daemon_authorize_local(struct usched_entry *entry, int fd) {
	int errsv = 0;

	if (auth_daemon_local(fd, &entry->uid, &entry->gid) < 0) {
		errsv = errno;
		log_warn("entry_daemon_authorize_local(): auth_local(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	entry_set_flag(entry, USCHED_ENTRY_FLAG_AUTHORIZED);

	return 1;
}

static int _entry_daemon_authorize_remote(struct usched_entry *entry, int fd) {
	int errsv = 0;

	/* Validate session data and compare user password hash */
	if (entry_daemon_remote_authorize_verify(entry) < 0) {
		errsv = errno;
		log_warn("_entry_daemon_authorize_remote(): entry_daemon_remote_authorize_verify(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	entry_set_flag(entry, USCHED_ENTRY_FLAG_AUTHORIZED);

	return 1;
}

int entry_daemon_authorize(struct usched_entry *entry, int fd) {
	int errsv = 0;
	int ret = -1;

	if ((ret = conn_is_local(fd)) < 0) {
		log_warn("entry_daemon_authorize(): conn_is_local(): %s\n", strerror(errno));
		return -1;
	} else if (ret == 1) {
		if ((ret = _entry_daemon_authorize_local(entry, fd)) < 0) {
			errsv = errno;
			log_warn("entry_daemon_authorize(): entry_daemon_authorize_local(): %s\n", strerror(errno));
			errno = errsv;
			return -1;
		}

		return ret;	/* ret == 1: Authorized, ret == 0: Not authorized (connection will timeout) */
	} else if ((ret = conn_is_remote(fd)) < 0) {
		errsv = errno;
		log_warn("entry_daemon_authorize(): conn_is_remote(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	} else if (ret == 1) {
		if ((ret = _entry_daemon_authorize_remote(entry, fd)) < 0) {
			errsv = errno;
			log_warn("entry_daemon_authorize(): entry_daemon_authorize_remote(): %s\n", strerror(errno));
			errno = errsv;
			return -1;
		}

		return ret;	/* ret == 1: Authorized, ret == 0: Not yet authorized (More data expected) */
	}

	errno = ENOSYS;

	return -1;	/* Not authorized. No authentication mechanism available */
}

int entry_daemon_remote_authorize_init(struct usched_entry *entry) {
	int errsv = 0;

	if (auth_daemon_remote_user_token_create(entry->username, entry->session, entry->nonce, entry->token) < 0) {
		errsv = errno;
		log_warn("entry_daemon_remoet_authorize_init(): auth_daemon_user_token_create(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	return 0;
}

int entry_daemon_remote_authorize_verify(struct usched_entry *entry) {
	int errsv = 0;

	if (auth_daemon_remote_user_token_verify(entry->username, entry->session, entry->nonce, entry->token, &entry->uid, &entry->gid) < 0) {
		errsv = errno;
		log_warn("entry_daemon_remote_authorize_verify(): auth_daemon_remote_user_token_verify(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	return 0;
}

int entry_daemon_remote_session_to_pubkey(struct usched_entry *entry) {
	if (sizeof(entry->session) < sizeof(entry->remote_key_pub)) {
		log_warn("entry_daemon_remote_session_to_pubkey(): sizeof(entry->session) < sizeof(entry->remote_key_pub)\n");
		errno = EINVAL;
		return -1;
	}

	memcpy(entry->remote_key_pub, entry->session, sizeof(entry->remote_key_pub));

	return 0;
}

int entry_daemon_remote_compute_shared_key(struct usched_entry *entry) {
	int errsv = 0;

	if (sec_daemon_compute_shared_key(entry->key_shr, entry->remote_key_pub) < 0) {
		errsv = errno;
		log_warn("entry_daemon_remote_compute_shared_key(): sec_daemon_compute_shared_key(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	return 0;
}

int entry_daemon_payload_decrypt(struct usched_entry *entry) {
	int errsv = 0;
	unsigned char *payload_dec = NULL;
	size_t out_len = 0;

	/* Alloc memory for decrypted payload */
	if (!(payload_dec = mm_alloc(entry->psize - CRYPT_EXTRA_SIZE_XSALSA20))) {
		errsv = errno;
		log_warn("entry_daemon_payload_decrypt(): mm_alloc(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Decrypt payload */
	if (!(crypt_decrypt_xsalsa20(payload_dec, &out_len, (unsigned char *) entry->payload, entry->psize, entry->nonce, entry->token))) {
		errsv = errno;
		log_warn("entry_daemon_payload_decrypt(): crypt_decrypt_xsalsa20(): %s\n", strerror(errno));
		mm_free(payload_dec);
		errno = errsv;
		return -1;
	}

	/* Set payload size */
	entry->psize = out_len;

	/* Free encrypted payload */
	mm_free(entry->payload);

	/* Set the new payload */
	entry->payload = (char *) payload_dec;

	/* All good */
	return 0;
}

void entry_daemon_pmq_dispatch(void *arg) {
	int errsv = 0;
	char *buf;
	struct usched_entry *entry = arg;

	if (!(buf = mm_alloc(rund.config.core.pmq_msgsize))) {
		errsv = errno;
		log_warn("entry_daemon_pmq_dispatch(): mm_alloc(): %s\n", strerror(errno));
		errno = errsv;
		goto _finish;
	}

	memset(buf, 0, rund.config.core.pmq_msgsize);

	/* Check if this entry is authorized */
	if (!entry_has_flag(entry, USCHED_ENTRY_FLAG_AUTHORIZED)) {
		log_warn("entry_daemon_pmq_dispatch(): Unauthorized entry found. Discarding...\n");
		errno = EACCES;
		goto _finish;
	}

	if ((strlen(entry->subj) + 17) > rund.config.core.pmq_msgsize) {
		log_warn("entry_daemon_pmq_dispatch(): msg_size > sizeof(buf)\n");
		errno = EMSGSIZE;
		goto _finish;
	}

	memcpy(buf, &entry->id, 8);
	memcpy(buf + 8, &entry->uid, 4);
	memcpy(buf + 12, &entry->gid, 4);
	memcpy(buf + 16, entry->subj, strlen(entry->subj));

	if (mq_send(rund.pmqd, buf, rund.config.core.pmq_msgsize, 0) < 0) {
		errsv = errno;
		log_warn("entry_daemon_pmq_dispatch(): mq_send(): %s\n", strerror(errno));
		errno = errsv;
		/* TODO: We should not delete this entry from active pool if we're unable to write the pqueue of
		 * the execution process. We should put this entries in some sort of local queue and wait for the
		 * pqueue to become available.
		 */
		goto _finish;
	}

	/* Update trigger, step and expire parameters of the entry based on psched library data */
	if (schedule_entry_update(entry) == 1) {
		mm_free(buf);
		/* Entry was successfully updated. */
		return;
	}

	/* Entry was not found. This means that it wasn't a recurrent entry (no step).
	 * It should be deleted from the active pool.
	 */
		
_finish:
	/* Remove the entry from active pool */
	pthread_mutex_lock(&rund.mutex_apool);
	rund.apool->del(rund.apool, entry);
	pthread_mutex_unlock(&rund.mutex_apool);

	mm_free(buf);
}

int entry_daemon_serialize(pall_fd_t fd, void *data) {
	int errsv = 0;
	struct usched_entry *entry = data;

	if (write(fd, &entry->id, sizeof(entry->id)) != sizeof(entry->id))
		goto _serialize_error;

	if (write(fd, &entry->flags, sizeof(entry->flags)) != sizeof(entry->flags))
		goto _serialize_error;

	if (write(fd, &entry->uid, sizeof(entry->uid)) != sizeof(entry->uid))
		goto _serialize_error;

	if (write(fd, &entry->gid, sizeof(entry->gid)) != sizeof(entry->gid))
		goto _serialize_error;

	if (write(fd, &entry->trigger, sizeof(entry->trigger)) != sizeof(entry->trigger))
		goto _serialize_error;

	if (write(fd, &entry->step, sizeof(entry->step)) != sizeof(entry->step))
		goto _serialize_error;

	if (write(fd, &entry->expire, sizeof(entry->expire)) != sizeof(entry->expire))
		goto _serialize_error;

	if (write(fd, &entry->subj_size, sizeof(entry->subj_size)) != sizeof(entry->subj_size))
		goto _serialize_error;

	if (write(fd, entry->subj, entry->subj_size) != entry->subj_size)
		goto _serialize_error;

	return 0;

_serialize_error:
	errsv = errno;

	log_warn("entry_daemon_serialize(): write(): %s\n", strerror(errno));

	errno = errsv;

	return -1;

}

void *entry_daemon_unserialize(pall_fd_t fd) {
	int errsv = 0;
	struct usched_entry *entry = NULL;

	if (!(entry = mm_alloc(sizeof(struct usched_entry)))) {
		errsv = errno;
		log_warn("entry_daemon_unserialize(): mm_alloc(): %s\n", strerror(errno));
		errno = errsv;
		return NULL;
	}

	memset(entry, 0, sizeof(struct usched_entry));

	if (read(fd, &entry->id, sizeof(entry->id)) != sizeof(entry->id))
		goto _unserialize_error;

	if (read(fd, &entry->flags, sizeof(entry->flags)) != sizeof(entry->flags))
		goto _unserialize_error;

	if (read(fd, &entry->uid, sizeof(entry->uid)) != sizeof(entry->uid))
		goto _unserialize_error;

	if (read(fd, &entry->gid, sizeof(entry->gid)) != sizeof(entry->gid))
		goto _unserialize_error;

	if (read(fd, &entry->trigger, sizeof(entry->trigger)) != sizeof(entry->trigger))
		goto _unserialize_error;

	if (read(fd, &entry->step, sizeof(entry->step)) != sizeof(entry->step))
		goto _unserialize_error;

	if (read(fd, &entry->expire, sizeof(entry->expire)) != sizeof(entry->expire))
		goto _unserialize_error;

	if (read(fd, &entry->subj_size, sizeof(entry->subj_size)) != sizeof(entry->subj_size))
		goto _unserialize_error;

	if (!(entry->subj = mm_alloc(entry->subj_size + 1))) {
		errsv = errno;
		log_warn("entry_daemon_unserialize(): mm_alloc(): %s\n", strerror(errno));
		entry_destroy(entry);
		errno = errsv;
		return NULL;
	}

	memset(entry->subj, 0, entry->subj_size + 1);

	if (read(fd, entry->subj, entry->subj_size) != entry->subj_size)
		goto _unserialize_error;

	return entry;

_unserialize_error:
	errsv = errno;

	log_warn("entry_daemon_unserialize(): read(): %s\n", strerror(errno));

	entry_destroy(entry);

	errno = errsv;

	return NULL;
}

