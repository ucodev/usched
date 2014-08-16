/**
 * @file process.c
 * @brief uSched
 *        Data Processing interface - Daemon
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
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>

#include <rtsaio/rtsaio.h>

#include "config.h"
#include "debug.h"
#include "mm.h"
#include "runtime.h"
#include "entry.h"
#include "log.h"
#include "schedule.h"
#include "conn.h"

static int _process_recv_update_op_new(struct async_op *aop, struct usched_entry *entry) {
	int errsv = 0;
	int cur_fd = aop->fd;
	struct usched_entry *entry_new = NULL;

	debug_printf(DEBUG_INFO, "PAYLOAD: %s\n", entry->payload);

	/* The payload of a NEW entry is the entry subject. */
	if (entry_set_subj(entry, entry->payload, entry->psize)) {
		errsv = errno;
		log_warn("_process_recv_update_op_new(): entry_set_subj(): %s\n", strerror(errno));
		goto _update_op_new_failure_1;
	}

	/* Clear payload information */
	entry_unset_payload(entry);

	/* Alloc memory for entry duplication */
	if (!(entry_new = mm_alloc(sizeof(struct usched_entry)))) {
		errsv = errno;
		log_warn("_process_recv_update_op_new(): mm_alloc(): %s\n", strerror(errno));
		goto _update_op_new_failure_1;
	}

	/* Duplicate the entry */
	if (entry_copy(entry_new, entry) < 0) {
		errsv = errno;
		log_warn("_process_recv_update_op_new(): entry_copy(): %s\n", strerror(errno));
		mm_free(entry_new);
		goto _update_op_new_failure_1;
	}

	/* We're done. Now we need to install and set a global and unique id for this entry */
	if (schedule_entry_create(entry_new) < 0) {
		errsv = errno;
		log_warn("_process_recv_update_op_new(): schedule_entry_create(): %s\n", strerror(errno));

		/* NOTE: This is a special case: The current entry is no longer in the rpool and wasn't inserted into
		 * the active pool (apool) due to errors.
		 * Since the entry was already pop'd, we don't need to do it here.
		 */

		goto _update_op_new_failure_1;
	}

	/* Update entry->id in order to report back the Entry ID to the client */
	entry->id = entry_new->id;

	/* NOTE: After schedule_entry_create() success, a new and unique entry->id is now set. */

	/* Reuse 'aop' to reply the entry->id to the client */
	mm_free((void *) aop->data);

	memset(aop, 0, sizeof(struct async_op));

	aop->fd = cur_fd;
	aop->count = sizeof(entry->id);
	aop->priority = 0;
	aop->timeout.tv_sec = rund.config.network.conn_timeout;

	if (!(aop->data = mm_alloc(sizeof(entry->id)))) {
		errsv = errno;
		log_warn("_process_recv_update_op_new(): aop->data = mm_alloc(%zu): %s\n", sizeof(entry->id), strerror(errno));

		goto _update_op_new_failure_2;
	}

	memcpy((void *) aop->data, (uint64_t [1]) { htonll(entry->id) }, sizeof(entry->id));

	debug_printf(DEBUG_INFO, "Delivering entry id: %llu\n", entry->id);

	return 0;

_update_op_new_failure_2:
	/* If we're unable to comunicate with the client, the scheduled entry should be disabled and pop'd from apool */
	if (!schedule_entry_disable(entry_new)) {
		/* This is critical and should never happen. This means that a race condition occured that allowed
		 * the user to operate over a unfinished entry. We'll abort here in order to prevent further damage.
		 */
		abort();
	}

_update_op_new_failure_1:
	errno = errsv;

	return -1;
}

static int _process_recv_update_op_del(struct async_op *aop, struct usched_entry *entry) {
	int errsv = 0, i = 0, cur_fd = aop->fd;
	uint64_t *entry_list_req = NULL, *entry_list_res = NULL;
	uint32_t entry_list_req_nmemb = 0, entry_list_res_nmemb = 0;

	/* Check if the payload size if aligned with the entry->id size */
	if ((entry->psize % sizeof(entry->id))) {
		log_warn("_process_recv_update_op_del(): entry->psize %% sizeof(entry->id) != 0\n");
		errno = EINVAL;
		return -1;
	}

	/* Gather the request entry list and respective list size */
	entry_list_req = (uint64_t *) entry->payload;
	entry_list_req_nmemb = entry->psize / sizeof(entry->id);

	/* entry_list_req_size should never be 0 since we grant that entry->psize != 0 in the entry creation stage.
	 * Anyway, since we're dealing with integers and entry->psize < sizeof(entry->id) is accepted in the pre-checks,
	 * lets play safe and grant that entry_list_req_size != 0.
	 */
	if (!entry_list_req_nmemb) {
		/* Someone is probably trying something nasty */
		log_warn("_process_recv_update_op_del(): entry_list_req_nmemb == 0 (Possible exploit attempt)\n");
		errno = EINVAL;
		return -1;
	}

	/* Revert network to host byte order */
	for (i = 0; i < entry_list_req_nmemb; i ++)
		entry_list_req[i] = ntohll(entry_list_req[i]);

	/* If the number of requested elements is 1 and the requested entry id is 0, this means to
	 * delete all the entries that match the entry's uid
	 */
	if ((entry_list_req_nmemb == 1) && (!entry_list_req[0])) {
		mm_free(entry->payload);
		entry->payload = NULL;
		entry->psize = 0;
		entry_list_req = NULL;

		if (schedule_entry_get_by_uid(entry->uid, &entry_list_req, &entry_list_req_nmemb) < 0) {
			errsv = errno;
			log_warn("_process_recv_update_op_get(): schedule_entry_get_by_uid(): %s\n", strerror(errno));
			errno = errsv;
			return -1;
		}

		entry->psize = entry_list_req_nmemb * sizeof(uint64_t);
		entry->payload = (char *) entry_list_req;
	}

	/* Iterate payload, ensure the user that's requesting the deletion is authorized to do so, and delete each of
	 * the valid entries through schedule_delete_entry().
	 */
	for (i = 0, entry_list_res_nmemb = 0; i < entry_list_req_nmemb; i ++) {
		if (schedule_entry_ownership_delete_by_id(entry_list_req[i], entry->uid) < 0) {
			log_warn("_process_recv_update_op_del(): schedule_entry_ownership_delete_by_id(): %s\n", strerror(errno));
			continue;
		}

		entry_list_res_nmemb ++;

		if (!(entry_list_res = mm_realloc(entry_list_res, entry_list_res_nmemb * sizeof(entry->id)))) {
			errsv = errno;
			log_warn("_process_recv_update_op_del(): mm_realloc(): %s\n", strerror(errno));
			errno = errsv;

			return -1;
		}

		/* Set early network byte order, as this list won't be used locally */
		entry_list_res[entry_list_res_nmemb - 1] = htonll(entry_list_req[i]);
	}

	/* Report back the deleted entries. */

	/* Reuse 'aop' to reply the deleted entries to the client */
	mm_free((void *) aop->data);

	memset(aop, 0, sizeof(struct async_op));

	aop->fd = cur_fd;
	aop->count = (sizeof(entry->id) * entry_list_res_nmemb) + sizeof(entry_list_res_nmemb);
	aop->priority = 0;
	aop->timeout.tv_sec = rund.config.network.conn_timeout;

	if (!(aop->data = mm_alloc(aop->count))) {
		errsv = errno;
		log_warn("_process_recv_update_op_del(): aop->data = mm_alloc(%zu): %s\n", aop->count, strerror(errno));

		mm_free(entry_list_res);

		errno = errsv;

		return -1;
	}

	/* Craft the list size at the head of the packet. */
	memcpy((void *) aop->data, (uint32_t [1]) { htonl(entry_list_res_nmemb) }, 4);

	if (entry_list_res_nmemb) {
		/* Append the list contents right after the list size field */
		memcpy((void *) (((char *) aop->data) + 4), entry_list_res, aop->count - 4);
	}

	debug_printf(DEBUG_INFO, "Delivering %lu entry ID's that were successfully deleted.\n", entry_list_res_nmemb);

	/* Free entry list */
	mm_free(entry_list_res);

	return 0;
}

static int _process_recv_update_op_get(struct async_op *aop, struct usched_entry *entry) {
	int errsv = 0, i = 0, cur_fd = aop->fd;
	uint64_t *entry_list_req = NULL;
	uint32_t entry_list_req_nmemb = 0, entry_list_res_nmemb = 0;
	size_t buf_offset = 0, len = 0;
	struct usched_entry *entry_c = NULL;
	char *buf = NULL;

	/* Transmission buffer 'buf' layout
	 *
	 * +===========+=================================+
	 * | Field     | Size                            |
	 * +===========+=================================+   --+
	 * | nmemb     | 32 bits                         |      > Header
	 * +-----------+---------------------------------+   --+
	 * | id        | 64 bits                         |     |
	 * | flags     | 32 bits                         |     |
	 * | uid       | 32 bits                         |     |
	 * | gid       | 32 bits                         |     |
	 * | trigger   | 32 bits                         |     |
	 * | step      | 32 bits                         |      > Serialized entry #1
	 * | expire    | 32 bits                         |     |
	 * | username  | CONFIG_USCHED_AUTH_USERNAME_MAX |     |
	 * | subj_size | 32 bits                         |     |
	 * | subj      | subj_size                       |     |
	 * +-----------+---------------------------------+   --+
	 * |           |                                 |     |
	 * |   ....    |              ....               |     | 
	 * |           |                                 |      > Serialized entry #2
	 * .           .                                 .     .
         * .           .                                 .     .
         * .           .                                 .    ..
	 *
	 */

	/* Check if the payload size if aligned with the entry->id size */
	if ((entry->psize % sizeof(entry->id))) {
		log_warn("_process_recv_update_op_get(): entry->psize %% sizeof(entry->id) != 0\n");
		errno = EINVAL;
		return -1;
	}

	/* Gather the request entry list and respective list size */
	entry_list_req = (uint64_t *) entry->payload;
	entry_list_req_nmemb = entry->psize / sizeof(entry->id);

	/* entry_list_req_size should never be 0 since we grant that entry->psize != 0 in the entry creation stage.
	 * Anyway, since we're dealing with integers and entry->psize < sizeof(entry->id) is accepted in the pre-checks,
	 * lets play safe and grant that entry_list_req_size != 0.
	 */
	if (!entry_list_req_nmemb) {
		/* Someone is probably trying something nasty */
		log_warn("_process_recv_update_op_get(): entry_list_req_nmemb == 0 (Possible exploit attempt)\n");
		errno = EINVAL;
		return -1;
	}

	/* Revert network to host byte order */
	for (i = 0; i < entry_list_req_nmemb; i ++)
		entry_list_req[i] = ntohll(entry_list_req[i]);

	/* If the number of requested elements is 1 and the requested entry id is 0, this means to
	 * fetch all the entries that match the entry's uid
	 */
	if ((entry_list_req_nmemb == 1) && (!entry_list_req[0])) {
		mm_free(entry->payload);
		entry->payload = NULL;
		entry->psize = 0;
		entry_list_req = NULL;

		if (schedule_entry_get_by_uid(entry->uid, &entry_list_req, &entry_list_req_nmemb) < 0) {
			errsv = errno;
			log_warn("_process_recv_update_op_get(): schedule_entry_get_by_uid(): %s\n", strerror(errno));
			errno = errsv;
			return -1;
		}

		entry->psize = entry_list_req_nmemb * sizeof(uint64_t);
		entry->payload = (char *) entry_list_req;
	}

	/* Initialize transmission buffer (nmemb -> 32bits) */
	buf_offset = 4;

	if (!(buf = mm_alloc(buf_offset))) {
		errsv = errno;
		log_warn("_process_recv_update_op_get(): mm_alloc(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	memset(buf, 0, buf_offset);

	/* Iterate payload, ensure the user that's requesting a given entry id is authorized to do so. */
	for (i = 0; i < entry_list_req_nmemb; i ++) {
		/* Search for the entry id in the active pool */
		if (!(entry_c = schedule_entry_get_copy(entry_list_req[i]))) {
			log_warn("_process_recv_update_op_get(): schedule_entry_get_copy(): %s\n", strerror(errno));
			continue;
		}

		/* Grant that the found entry belongs to the requesting uid */
		if (entry_c->uid != entry->uid) {
			log_warn("_process_recv_update_op_get(): entry_c->uid != entry->uid. (UID of the request: %u\n", entry->uid);
			continue;
		}

		/* Clear entry session data, so it won't be transmitted to client */
		memset(entry_c->session, 0, CONFIG_USCHED_AUTH_SESSION_MAX);

		/* Clear entry payload */
		if (entry_c->payload) {
			mm_free(entry_c->payload);
			entry_c->payload = NULL;
			entry_c->psize = 0;
		}

		/* Clear entry psched_id. The client should not be aware of this information */
		entry_c->psched_id = 0;

		/* Calculate the next length for buf */
		len = buf_offset + offsetof(struct usched_entry, psize) + CONFIG_USCHED_AUTH_USERNAME_MAX + sizeof(entry_c->subj_size) + entry_c->subj_size + 1;

		/* Extend transmission buffer */
		if (!(buf = mm_realloc(buf, len))) {
			errsv = errno;
			log_warn("_process_recv_update_op_get(): mm_realloc(): %s\n", strerror(errno));
			entry_destroy(entry_c);
			errno = errsv;
			return -1;
		}

		/* Reset the extended memory region. */
		memset(buf + buf_offset, 0, len - buf_offset);

		/* Set entry contents endianess to network byte order */
		entry_c->id = htonll(entry_c->id);
		entry_c->flags = htonl(entry_c->flags);
		entry_c->uid = htonl(entry_c->uid);
		entry_c->gid = htonl(entry_c->gid);
		entry_c->trigger = htonl(entry_c->trigger);
		entry_c->step = htonl(entry_c->step);
		entry_c->expire = htonl(entry_c->expire);
		/* NOTE: We don't convert the entry_c->subj_size here as we need to handle sizes
		 * based on this value. It will be converted later
		 */

		/* Serialize entry contents into the transmission buffer */
		memcpy(buf + buf_offset, entry_c, offsetof(struct usched_entry, psize));
		buf_offset += offsetof(struct usched_entry, psize);
		memcpy(buf + buf_offset, entry_c->username, CONFIG_USCHED_AUTH_USERNAME_MAX);
		buf_offset += CONFIG_USCHED_AUTH_USERNAME_MAX;
		memcpy(buf + buf_offset, (uint32_t [1]) { htonl(entry_c->subj_size) }, sizeof(entry_c->subj_size));
		buf_offset += sizeof(entry_c->subj_size);
		memcpy(buf + buf_offset, entry_c->subj, entry_c->subj_size + 1);
		buf_offset += entry_c->subj_size + 1;

		/* Increment the number of elements in the buffer */
		entry_list_res_nmemb ++;

		/* Destroy the entry copy */
		entry_destroy(entry_c);
	}

	/* Update buffer header with the current number of entries */
	memcpy(buf, (uint32_t [1]) { htonl(entry_list_res_nmemb) }, 4);

	/* Reuse 'aop' to reply the contents of the requested entries */
	mm_free((void *) aop->data);

	memset(aop, 0, sizeof(struct async_op));

	aop->fd = cur_fd;
	aop->count = buf_offset;
	aop->priority = 0;
	aop->timeout.tv_sec = rund.config.network.conn_timeout;
	aop->data = buf;

	return 0;
}

struct usched_entry *process_daemon_recv_create(struct async_op *aop) {
	int errsv = 0;
	struct usched_entry *entry = NULL;

	/* Process the received entry data */
	if (!(entry = mm_alloc(sizeof(struct usched_entry)))) {
		errsv = errno;
		log_warn("process_recv_create(): entry = mm_alloc(): %s\n", strerror(errno));
		errno = errsv;
		return NULL;
	}

	memset(entry, 0, sizeof(struct usched_entry));

	memcpy(entry, (void *) aop->data, aop->count);

	/* Free aop data. We no longer need it */
	mm_free((void *) aop->data);
	aop->data = NULL;

	/* Setup received entry */
	entry_set_id(entry, aop->fd);
	entry_set_flags(entry, ntohl(entry->flags));
	entry_set_uid(entry, ntohl(entry->uid));
	entry_set_gid(entry, ntohl(entry->gid));
	entry_set_trigger(entry, ntohl(entry->trigger));
	entry_set_step(entry, ntohl(entry->step));
	entry_set_expire(entry, ntohl(entry->expire));
	entry_set_psize(entry, ntohl(entry->psize));

	/* Clear all local flags that the client have possibly set */
	entry_unset_flags_local(entry);

	/* Validate if this entry has at least one valid operation flag */
	if (!entry_has_flag(entry, USCHED_ENTRY_FLAG_NEW) && !entry_has_flag(entry, USCHED_ENTRY_FLAG_DEL) && !entry_has_flag(entry, USCHED_ENTRY_FLAG_GET)) {
		log_warn("process_daemon_recv_create(): The requested operation is invalid.\n");
		entry_destroy(entry);
		errno = EINVAL;
		return NULL;
	}

	/* Validate payload size */
	if (!entry->psize) {
		/* All entry requests expect a payload. If none is set, this entry request is invalid. */
		log_warn("process_daemon_recv_create(): entry->psize == 0.\n");
		entry_destroy(entry);
		errno = EINVAL;
		return NULL;
	}

	debug_printf(DEBUG_INFO, "psize: %u\n", entry->psize);
	debug_printf(DEBUG_INFO, "username: %s\n", entry->username);

	/* Set this entry state as in progress */
	entry_set_flag(entry, USCHED_ENTRY_FLAG_PROGRESS);

	/*
	 * If this is a remote connection:
	 *
	 *  - Send a ciphered session token in the session field which the encryption key is the
	 * hash of the user password in the users configuration structure. Append the user hash salt
	 * and encryption nonce value to the head of the ciphered result. All these steps are
	 * accomplished by entry_daemon_remote_session_create() function.
	 *
	 *
	 * If this is a local connection:
	 *
	 *  - Set the session field to all zeros.
	 *
	 */
	if (conn_is_remote(aop->fd)) {
		if (entry_daemon_remote_session_create(entry) < 0) {
			errsv = errno;
			log_warn("process_daemon_recv_create(): entry_daemon_remote_session_create(): %s\n", strerror(errno));
			entry_destroy(entry);
			errno = errsv;
			return NULL;
		}
	} else if (conn_is_local(entry->id)) {
		memset(entry->session, 0, sizeof(entry->session));
	} else {
		log_warn("process_daemon_recv_create(): Unable to determine connection type.\n");
		entry_destroy(entry);
		errno = EINVAL;
		return NULL;
	}

	/* Reuse 'aop' for next read request: Receive the entry command */
	memset(aop, 0, sizeof(struct async_op));

	aop->fd = entry->id;
	aop->count = sizeof(entry->session);
	aop->priority = 0;
	aop->timeout.tv_sec = rund.config.network.conn_timeout;

	if (!(aop->data = mm_alloc(aop->count))) {
		errsv = errno;
		log_warn("process_daemon_recv_create(): aop->data = mm_alloc(): %s\n", strerror(errno));
		entry_destroy(entry);
		errno = errsv;
		return NULL;
	}

	/* Copy the session field into aop data */
	memcpy((void *) aop->data, entry->session, aop->count);

	return entry;
}

int process_daemon_recv_update(struct async_op *aop, struct usched_entry *entry) {
	int errsv = 0;

	/* Check if the entry is initialized */
	if (!entry_has_flag(entry, USCHED_ENTRY_FLAG_INIT)) {
		log_warn("process_daemon_recv_update(): Trying to update an existing entry that wasn't initialized yet.\n");
		entry_destroy(entry);
		errno = EINVAL;
		return -1;
	}

	/* Check if the entry is marked as complete */
	if (entry_has_flag(entry, USCHED_ENTRY_FLAG_COMPLETE) || entry_has_flag(entry, USCHED_ENTRY_FLAG_FINISH)) {
		log_warn("process_daemon_recv_update(): Trying to update an existing entry that is already in a finish or complete state.\n");
		entry_destroy(entry);
		errno = EINVAL;
		return -1;
	}

	/* Copy the session authentication data into the entry->session field */
	memcpy(entry->session, (void *) aop->data, sizeof(entry->session));

	/* Check if the entry is authorized. If not, authorize it and try to proceed. */
	if (!entry_has_flag(entry, USCHED_ENTRY_FLAG_AUTHORIZED) && (entry_daemon_authorize(entry, aop->fd) < 0)) {
		errsv = errno;
		log_warn("process_daemon_recv_update(): entry_daemon_authorize(): %s\n", strerror(errno));
		entry_destroy(entry);

		if (!errsv)
			errno = EINVAL;

		return -1;
	}

	/* Check if the entry is on a finishing state */
	if (entry_has_flag(entry, USCHED_ENTRY_FLAG_FINISH)) {
		log_warn("process_daemon_recv_update(): Trying to update an entry that's in a FINISH state.\n");
		entry_destroy(entry);
		errno = EINVAL;
		return -1;
	}

	/* Set the FINISH flag as this entry is now on a finishing state. */
	entry_set_flag(entry, USCHED_ENTRY_FLAG_FINISH);

	/* Re-validate authorization. If not authorized, discard this entry */
	if (!entry_has_flag(entry, USCHED_ENTRY_FLAG_AUTHORIZED)) {
		errsv = errno;
		log_warn("process_daemon_recv_update(): Unauthorized entry\n", strerror(errno));
		entry_destroy(entry);
		errno = errsv;
		return -1;
	}

	debug_printf(DEBUG_INFO, "psize: %u, aop->count: %zu\n", entry->psize, aop->count);

	/* Grant that the received data does not exceed the expected size */
	if (aop->count != (sizeof(entry->session) + entry->psize)) {
		errsv = errno;
		log_warn("process_daemon_recv_update(): aop->count != (sizeof(entry->session) + entry->psize)\n");
		entry_destroy(entry);
		errno = errsv;
		return -1;
	}

	/* Set the received entry payload which is sizeof(entry->session) offset bytes from
	 * aop->data base pointer
	 */
	if (entry_set_payload(entry, (char *) aop->data + sizeof(entry->session), entry->psize) < 0) {
		errsv = errno;
		log_warn("process_daemon_recv_update(): entry_set_payload(): %s\n", strerror(errno));
		entry_destroy(entry);
		errno = errsv;
		return -1;
	}

	/* Decrypt payload if this is a remote connection */
	if (conn_is_remote(aop->fd)) {
		if (entry_daemon_payload_decrypt(entry) < 0) {
			errsv = errno;
			log_warn("process_daemon_recv_update(): entry_daemon_payload_decrypt(): %s\n", strerror(errno));
			entry_destroy(entry);
			errno = errsv;
			return -1;
		}
	}

	/* Process specific operation types */
	if (entry_has_flag(entry, USCHED_ENTRY_FLAG_NEW)) {
		if (_process_recv_update_op_new(aop, entry) < 0) {
			errsv = errno;
			log_warn("process_daemon_recv_update(): _process_recv_update_op_new(): %s\n", strerror(errno));

			entry_destroy(entry);
			errno = errsv;
			return -1;
		}
	} else if (entry_has_flag(entry, USCHED_ENTRY_FLAG_DEL)) {
		if (_process_recv_update_op_del(aop, entry) < 0) {
			errsv = errno;
			log_warn("process_daemon_recv_update(): _process_recv_update_op_del(): %s\n", strerror(errno));
			entry_destroy(entry);
			errno = errsv;
			return -1;
		}
	} else if (entry_has_flag(entry, USCHED_ENTRY_FLAG_GET)) {
		if (_process_recv_update_op_get(aop, entry) < 0) {
			errsv = errno;
			log_warn("process_daemon_recv_update(): _process_recv_update_op_get(): %s\n", strerror(errno));

			entry_destroy(entry);
			errno = errsv;
			return -1;
		}
	} else {
		log_warn("process_daemon_recv_update(): The requested operation is invalid.\n");
		entry_destroy(entry);
		errno = EINVAL;
		return -1;
	}

	return 0;
}

