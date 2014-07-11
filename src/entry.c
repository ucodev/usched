/**
 * @file entry.c
 * @brief uSched
 *        Entry handling interface
 *
 * Date: 11-07-2014
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
#include <time.h>
#include <pthread.h>
#include <mqueue.h>
#include <unistd.h>

#include <sys/types.h>

#include <pall/cll.h>

#include "config.h"
#include "usched.h"
#include "runtime.h"
#include "mm.h"
#include "entry.h"
#include "bitops.h"
#include "log.h"
#include "auth.h"
#include "conn.h"
#include "schedule.h"

struct usched_entry *entry_init(uid_t uid, gid_t gid, time_t trigger, char *payload) {
	struct usched_entry *entry = NULL;

	if (!(entry = mm_alloc(sizeof(struct usched_entry))))
		return NULL;

	memset(entry, 0, sizeof(struct usched_entry));

	entry_set_uid(entry, uid);
	entry_set_gid(entry, gid);
	entry_set_trigger(entry, trigger);

	if (entry_set_payload(entry, payload, strlen(payload) + 1) < 0) {
		mm_free(entry);
		return NULL;
	}

	return entry;
}

void entry_set_id(struct usched_entry *entry, uint32_t id) {
	entry->id = id;
}

void entry_set_flags(struct usched_entry *entry, uint32_t flags) {
	entry->flags = flags;
}

void entry_unset_flags_local(struct usched_entry *entry) {
	/* Clear all local flags */
	entry_unset_flag(entry, USCHED_ENTRY_FLAG_INIT);
	entry_unset_flag(entry, USCHED_ENTRY_FLAG_AUTHORIZED);
	entry_unset_flag(entry, USCHED_ENTRY_FLAG_FINISH);
}

int entry_has_flag(struct usched_entry *entry, usched_entry_flag_t flag) {
	return bit_test(&entry->flags, flag);
}

void entry_set_flag(struct usched_entry *entry, usched_entry_flag_t flag) {
	bit_set(&entry->flags, flag);
}

void entry_unset_flag(struct usched_entry *entry, usched_entry_flag_t flag) {
	bit_clear(&entry->flags, flag);
}

void entry_set_uid(struct usched_entry *entry, uid_t uid) {
	entry->uid = uid;
}

void entry_set_gid(struct usched_entry *entry, gid_t gid) {
	entry->gid = gid;
}

void entry_set_trigger(struct usched_entry *entry, time_t trigger) {
	entry->trigger = (uint32_t) trigger;
}

void entry_set_step(struct usched_entry *entry, time_t step) {
	entry->step = (uint32_t) step;
}

void entry_set_expire(struct usched_entry *entry, time_t expire) {
	entry->expire = (uint32_t) expire;
}

void entry_set_psize(struct usched_entry *entry, size_t size) {
	entry->psize = (uint32_t) size;
}


int entry_set_payload(struct usched_entry *entry, char *payload, size_t len) {
	if (!len)
		len = strlen(payload) + 1;

	if (!(entry->payload = mm_alloc(len)))
		return -1;

	memset(entry->payload, 0, len);

	memcpy(entry->payload, payload, len);

	entry_set_psize(entry, len);

	return 0;
}

int entry_compare(const void *e1, const void *e2) {
	const struct usched_entry *pe1 = (struct usched_entry *) e1, *pe2 = (struct usched_entry *) e2;

	if (pe1->id > pe2->id)
		return 1;

	if (pe1->id < pe2->id)
		return -1;

	return 0;
}

static int _entry_authorize_local(struct usched_entry *entry, int fd) {
	if (auth_local(fd, &entry->uid, &entry->gid) < 0) {
		log_warn("entry_authorize_local(): auth_local(): %s\n", strerror(errno));
		return -1;
	}

	entry_set_flag(entry, USCHED_ENTRY_FLAG_AUTHORIZED);

	return 1;
}

static int _entry_authorize_remote(struct usched_entry *entry, int fd) {
	/* TODO: To be implemented */

	/* rtsaio_read() ... */
	/* Retrieve user/pass */
	/* auth_remote() ... */

	errno = ENOSYS;

	return -1;
}

int entry_authorize(struct usched_entry *entry, int fd) {
	int ret = -1;

	if ((ret = conn_is_local(fd)) < 0) {
		log_warn("entry_authorize(): conn_is_local(): %s\n", strerror(errno));
		return -1;
	} else if (ret == 1) {
		if ((ret = _entry_authorize_local(entry, fd)) < 0) {
			log_warn("entry_authorize(): entry_authorize_local(): %s\n", strerror(errno));
			return -1;
		}

		return ret;	/* ret == 1: Authorized, ret == 0: Not authorized (connection will timeout) */
	} else if ((ret = conn_is_remote(fd) < 0)) {
		log_warn("entry_authorize(): conn_is_remote(): %s\n", strerror(errno));
		return -1;
	} else if (ret == 1) {
		if ((ret = _entry_authorize_remote(entry, fd)) < 0) {
			log_warn("entry_authorize(): entry_authorize_remote(): %s\n", strerror(errno));
			return -1;
		}

		return ret;	/* ret == 1: Authorized, ret == 0: Not yet authorized (More data expected) */
	}

	errno = ENOSYS;

	return -1;	/* Not authorized. No authentication mechanism available */
}

void entry_pmq_dispatch(void *arg) {
	char buf[CONFIG_USCHED_PMQ_MSG_SIZE];
	struct usched_entry *entry = arg;

	memset(buf, 0, sizeof(buf));

	/* Check if this entry is authorized */
	if (!entry_has_flag(entry, USCHED_ENTRY_FLAG_AUTHORIZED)) {
		log_warn("entry_pmq_dispatch(): Unauthorized entry found. Discarding...\n");
		goto _finish;
	}

	if ((strlen(entry->payload) + 9) > sizeof(buf)) {
		log_warn("entry_pmq_dispatch(): msg_size > sizeof(buf)\n");
		goto _finish;
	}

	memcpy(buf, &entry->uid, 4);
	memcpy(buf + 4, &entry->gid, 4);
	memcpy(buf + 8, entry->payload, strlen(entry->payload));

	if (mq_send(rund.pmqd, buf, sizeof(buf), 0) < 0) {
		log_warn("entry_pmq_dispatch(): mq_send(): %s\n", strerror(errno));
		goto _finish;
	}

	/* Update trigger, step and expire parameters of the entry based on psched library data */
	if (schedule_entry_update(entry) == 1) {
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
}

void entry_destroy(void *elem) {
	struct usched_entry *entry = elem;

	if (entry->payload)
		mm_free(entry->payload);

	mm_free(entry);
}

int entry_serialize(pall_fd_t fd, void *data) {
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

	if (write(fd, &entry->psize, sizeof(entry->psize)) != sizeof(entry->psize))
		goto _serialize_error;

	if (write(fd, entry->payload, entry->psize) != entry->psize)
		goto _serialize_error;

	if (write(fd, &entry->psched_id, sizeof(entry->psched_id)) != sizeof(entry->psched_id))
		goto _serialize_error;

	return 0;

_serialize_error:
	errsv = errno;

	log_warn("entry_serialize(): write(): %s\n", strerror(errno));

	errno = errsv;

	return -1;

}

void *entry_unserialize(pall_fd_t fd) {
	int errsv = 0;
	struct usched_entry *entry = NULL;

	if (!(entry = mm_alloc(sizeof(struct usched_entry)))) {
		errsv = errno;
		log_warn("entry_unserialize(): mm_alloc(): %s\n", strerror(errno));
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

	if (read(fd, &entry->psize, sizeof(entry->psize)) != sizeof(entry->psize))
		goto _unserialize_error;

	if (!(entry->payload = mm_alloc(entry->psize + 1))) {
		errsv = errno;
		log_warn("entry_unserialize(): mm_alloc(): %s\n", strerror(errno));
		mm_free(entry);
		errno = errsv;
		return NULL;
	}

	memset(entry->payload, 0, entry->psize + 1);

	if (read(fd, entry->payload, entry->psize) != entry->psize)
		goto _unserialize_error;

	if (read(fd, &entry->psched_id, sizeof(entry->psched_id)) != sizeof(entry->psched_id))
		goto _unserialize_error;


	return entry;

_unserialize_error:
	errsv = errno;

	log_warn("entry_unserialize(): read(): %s\n", strerror(errno));

	if (entry->payload)
		mm_free(entry->payload);

	mm_free(entry);

	errno = errsv;

	return NULL;
}

