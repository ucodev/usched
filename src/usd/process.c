/**
 * @file process.c
 * @brief uSched
 *        Data Processing interface - Daemon
 *
 * Date: 29-06-2015
 * 
 * Copyright 2014-2015 Pedro A. Hortas (pah@ucodev.org)
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
#include "bitops.h"
#include "mm.h"
#include "runtime.h"
#include "entry.h"
#include "log.h"
#include "schedule.h"
#include "conn.h"
#include "usched.h"

static int _process_recv_update_op_new(struct async_op *aop, struct usched_entry *entry) {
	int errsv = 0;
	int cur_fd = aop->fd;

	debug_printf(DEBUG_INFO, "PAYLOAD: %s\n", entry->payload);

	/* The payload of a NEW entry is the entry subject. */
	if (entry_set_subj(entry, entry->payload, entry->psize)) {
		errsv = errno;
		log_warn("_process_recv_update_op_new(): entry_set_subj(): %s\n", strerror(errno));
		goto _update_op_new_failure_1;
	}

	/* Clear payload information */
	entry_unset_payload(entry);

	/* We're done. Now we need to install and set a global and unique id for this entry */
	if (schedule_entry_create(entry) < 0) {
		errsv = errno;
		log_warn("_process_recv_update_op_new(): schedule_entry_create(): %s\n", strerror(errno));

		/* Destroy the entry (see NOTE below) */
		entry_destroy(entry);

		/* NOTE: This is a special case: The current entry is no longer in the rpool and wasn't inserted into
		 * the active pool (apool) due to errors.
		 * Since the entry was already pop'd, we don't need to do it here.
		 */

		goto _update_op_new_failure_1;
	}

	/* Inform runtime that serialization is required */
	bit_set(&rund.flags, USCHED_RUNTIME_FLAG_SERIALIZE);

	/* NOTE: After schedule_entry_create() success, a new and unique entry->id is now set. */

	/* Set payload */
	if (entry_set_payload(entry, (const char *) (uint64_t [1]) { htonll(entry->id) }, 8) < 0) {
		errsv = errno;
		log_warn("_process_recv_update_op_new(): entry_set_payload(): %s\n", strerror(errno));
		errno = errsv;
		goto _update_op_new_failure_2;
	}

	/* Encrypt the payload */
	if (entry_payload_encrypt(entry, 4) < 0) {
		errsv = errno;
		log_warn("_process_recv_update_op_new(): entry_payload_encrypt(): %s\n", strerror(errno));
		entry_unset_payload(entry);
		errno = errsv;
		goto _update_op_new_failure_2;
	}

	/* Prepend the payload size */
	memcpy(entry->payload, (uint32_t [1]) { htonl(entry->psize) }, 4);

	/* Reuse 'aop' to reply the entry->id to the client */
	mm_free((void *) aop->data);

	memset(aop, 0, sizeof(struct async_op));

	aop->fd = cur_fd;
	aop->count = entry->psize;
	aop->priority = 0;
	aop->timeout.tv_sec = rund.config.network.conn_timeout;
	aop->data = entry->payload;

	/* Unset the payload without free()ing it */
	entry->payload = NULL;
	entry->psize = 0;

	debug_printf(DEBUG_INFO, "Delivering entry id: %llu\n", entry->id);

	return 0;

_update_op_new_failure_2:
	/* If we're unable to comunicate with the client, the scheduled entry should be disabled and pop'd from apool */
	if (!schedule_entry_disable(entry)) {
		/* This is critical and should never happen. This means that a race condition occured that allowed
		 * the user to operate over a unfinished entry. We'll abort here in order to prevent further damage.
		 */
		runtime_daemon_fatal();
	}

	/* Revert the entry id to its original file descriptor. This will allow the rpool cleanup
	 * routines be aware that this entry wasn't successfully inserted into the active pool.
	 */
	entry->id = (uint64_t) aop->fd;

_update_op_new_failure_1:
	errno = errsv;

	return -1;
}

static int _process_recv_update_op_del(struct async_op *aop, struct usched_entry *entry) {
	int errsv = 0, cur_fd = aop->fd;
	uint64_t *entry_list_req = NULL, *entry_list_res = NULL;
	uint32_t i = 0, entry_list_req_nmemb = 0, entry_list_res_nmemb = 0;

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

	/* If the number of requested elements is 1 and the requested entry id is USCHED_SUBJ_ALL,
	 * this means to delete all the entries that match the entry's uid
	 */
	if ((entry_list_req_nmemb == 1) && (entry_list_req[0] == USCHED_SUBJ_ALL)) {
		entry_unset_payload(entry);

		entry_list_req = NULL;

		if (schedule_entry_get_by_uid(entry->uid, &entry_list_req, &entry_list_req_nmemb) < 0) {
			errsv = errno;
			log_warn("_process_recv_update_op_del(): schedule_entry_get_by_uid(): %s\n", strerror(errno));
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

		/* Inform runtime that serialization is required */
		bit_set(&rund.flags, USCHED_RUNTIME_FLAG_SERIALIZE);

		/* Reallocate list memory to hold another deleted entry id */
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

	/* Unset the entry payload */
	entry_unset_payload(entry);

	/* Allocate enough memory to craft the payload */
	entry->psize = (sizeof(entry->id) * entry_list_res_nmemb) + sizeof(entry_list_res_nmemb);

	if (!(entry->payload = mm_alloc(entry->psize))) {
		errsv = errno;
		log_warn("_process_recv_update_op_del(): mm_alloc(): %s\n", strerror(errno));
		mm_free(entry_list_res);
		errno = errsv;
		return -1;
	}

	/* Craft the payload */
	if (entry_list_res)
		memcpy(entry->payload + sizeof(entry_list_res_nmemb), entry_list_res, sizeof(entry->id) * entry_list_res_nmemb);

	memcpy(entry->payload, (uint32_t [1]) { htonl(entry_list_res_nmemb) }, 4);

	/* Encrypt the payload */
	if (entry_payload_encrypt(entry, 4) < 0) {
		errsv = errno;
		log_warn("_process_recv_update_op_del(): entry_payload_encrypt(): %s\n", strerror(errno));
		entry_unset_payload(entry);
		mm_free(entry_list_res);
		errno = errsv;
		return -1;
	}

	/* Prepend the payload size to the payload itself */
	memcpy(entry->payload, (uint32_t [1]) { htonl(entry->psize) }, 4);

	/* Reuse 'aop' to reply the deleted entries to the client */
	mm_free((void *) aop->data);

	memset(aop, 0, sizeof(struct async_op));

	aop->fd = cur_fd;
	aop->count = entry->psize;
	aop->priority = 0;
	aop->timeout.tv_sec = rund.config.network.conn_timeout;
	aop->data = entry->payload;

	/* Unset the payload without free()ing it */
	entry->payload = NULL;
	entry->psize = 0;

	debug_printf(DEBUG_INFO, "Delivering %lu entry ID's that were successfully deleted.\n", entry_list_res_nmemb);

	/* Free entry list */
	mm_free(entry_list_res);

	/* All good */
	return 0;
}

static int _process_recv_update_op_get(struct async_op *aop, struct usched_entry *entry) {
	int errsv = 0, cur_fd = aop->fd;
	uint64_t *entry_list_req = NULL;
	uint32_t i = 0, entry_list_req_nmemb = 0, entry_list_res_nmemb = 0;
	size_t buf_offset = 0, len = 0;
	struct usched_entry *entry_c = NULL;
	char *buf = NULL;

	/* Transmission buffer 'buf' layout
	 *
	 * +=============+=================================+
	 * | Field       | Size                            |
	 * +=============+=================================+   --+
	 * | nmemb       | 32 bits                         |      > Header
	 * +-------------+---------------------------------+   --+
	 * | id          | 64 bits                         |     |
	 * | flags       | 32 bits                         |     |
	 * | uid         | 32 bits                         |     |
	 * | gid         | 32 bits                         |     |
	 * | trigger     | 32 bits                         |     |
	 * | step        | 32 bits                         |      > Serialized entry #1
	 * | expire      | 32 bits                         |     |
	 * | pid         | 32 bits                         |     |
	 * | status      | 32 bits                         |     |
	 * | exec_time   | 64 bits                         |     |
	 * | latency     | 64 bits                         |     |
	 * | outdata_len | 32 bits                         |     |
	 * | outdata     | CONFIG_USCHED_EXEC_OUTPUT_MAX   |     |
	 * | username    | CONFIG_USCHED_AUTH_USERNAME_MAX |     |
	 * | subj_size   | 32 bits                         |     |
	 * | subj        | subj_size                       |     |
	 * +-------------+---------------------------------+   --+
	 * |             |                                 |     |
	 * |    .....    |              ....               |     | 
	 * |             |                                 |      > Serialized entry #2
	 * .             .                                 .     .
         * .             .                                 .     .
         * .             .                                 .    ..
	 *
	 */

	/* Check if the payload size is aligned with the entry->id size */
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

	/* If the number of requested elements is 1 and the requested entry id is USCHED_SUBJ_ALL,
	 * this means to fetch all the entries that match the entry's uid
	 */
	if ((entry_list_req_nmemb == 1) && (entry_list_req[0] == USCHED_SUBJ_ALL)) {
		entry_unset_payload(entry);

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
		/* Search for the entry id in the active pool and get a copy of it. */
		if (!(entry_c = schedule_entry_get_copy(entry_list_req[i]))) {
			log_warn("_process_recv_update_op_get(): schedule_entry_get_copy(): %s\n", strerror(errno));
			continue;
		}

		/* Grant that the found entry belongs to the requesting uid */
		if (entry_c->uid != entry->uid) {
			log_warn("_process_recv_update_op_get(): entry_c->uid != entry->uid. (UID of the request: %u\n", entry->uid);

			entry_destroy(entry_c);

			continue;
		}

		/* Grant that outdata_len doesn't exceed the hardlimit */
		if ((entry_c->outdata_len >= CONFIG_USCHED_EXEC_OUTPUT_MAX) || (entry_c->outdata_len != strlen(entry_c->outdata))) {
			log_crit("_process_recv_update_op_get(): (entry_c->outdata_len >= CONFIG_USCHED_EXEC_OUTPUT_MAX) || (entry_c->outdata_len != strlen(entry_c->outdata))\n");

			entry_destroy(entry_c);

			continue;
		}

		/* Clear entry session data, so it won't be transmitted to client */
		memset(entry_c->session, 0, CONFIG_USCHED_AUTH_SESSION_MAX);

		/* Clear entry payload */
		entry_unset_payload(entry_c);

		/* Clear entry psched_id. The client should not be aware of this information */
		entry_c->reserved.psched_id = 0;

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
		entry_c->pid = htonl(entry_c->pid);
		entry_c->status = htonl(entry_c->status);
		entry_c->exec_time = htonll(entry_c->exec_time);
		entry_c->latency = htonll(entry_c->latency);
		entry_c->outdata_len = htonl(entry_c->outdata_len);
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

	/* Setup the payload to be encrypted */
	entry_unset_payload(entry);
	entry->payload = buf;
	entry->psize = buf_offset;

	/* Encrypt the payload and reserve 4 bytes on top of it to prepend the payload size later */
	if (entry_payload_encrypt(entry, 4) < 0) {
		errsv = errno;
		log_warn("_process_recv_update_op_get(): entry_payload_encrypt(): %s\n", strerror(errno));
		entry_unset_payload(entry);
		errno = errsv;
		return -1;
	}

	/* Prepend the payload size to the payload itself */
	memcpy(entry->payload, (uint32_t [1]) { htonl(entry->psize) }, 4);

	/* Reuse 'aop' to reply the contents of the requested entries */
	mm_free((void *) aop->data);

	memset(aop, 0, sizeof(struct async_op));

	aop->fd = cur_fd;
	aop->count = entry->psize;
	aop->priority = 0;
	aop->timeout.tv_sec = rund.config.network.conn_timeout;
	aop->data = entry->payload;

	/* Unset the payload without free()ing it. The new reference to the region was set to aop->data */
	entry->payload = NULL;
	entry->psize = 0;

	/* All good */
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
	entry_set_id(entry, (uint64_t) aop->fd);
	entry_set_flags(entry, ntohl(entry->flags));
	entry_set_uid(entry, ntohl(entry->uid)); /* Untrusted. Will be used for comparison only */
	entry_set_gid(entry, ntohl(entry->gid)); /* Untrusted. Will be used for comparison only */
	entry_set_trigger(entry, ntohl(entry->trigger));
	entry_set_step(entry, ntohl(entry->step));
	entry_set_expire(entry, ntohl(entry->expire));
	/* NOTE: pid, status, exec_time, latency, outdata_len and outdata are ignored here */
	entry_set_psize(entry, ntohl(entry->psize));

	/* Set the last byte of username field to 0, so it will always be NULL terminated */
	entry->username[sizeof(entry->username) - 1] = 0;

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

	/* If this is a new entry request, grant that subject fits in the mqueue message size */
	if (entry_has_flag(entry, USCHED_ENTRY_FLAG_NEW) && ((entry->psize + 21) > (size_t) rund.config.ipc.msg_size)) {
		log_warn("process_daemon_recv_create(): (entry->psize + 21) > rund,config.core.ipc.msgsize. This means that the subject is too long to be processed on this system.\n");
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
	} else if (conn_is_local(aop->fd)) {
		memset(entry->session, 0, sizeof(entry->session));
	} else {
		log_warn("process_daemon_recv_create(): Unable to determine connection type.\n");
		entry_destroy(entry);
		errno = EINVAL;
		return NULL;
	}

	/* Reuse 'aop' for next read request: Receive the entry command */
	memset(aop, 0, sizeof(struct async_op));

	aop->fd = (int) entry->id;
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

	debug_printf(DEBUG_INFO, "psize: %u, aop->count: %zu\n", entry->psize, aop->count);

	/* Grant that the received data match the expected size */
	if (aop->count != (sizeof(entry->session) + entry->psize)) {
		log_warn("process_daemon_recv_update(): aop->count != (sizeof(entry->session) + entry->psize). This isn\'t supposed to happen (psize: %u, aop->count: %zu).\n", entry->psize, aop->count);
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
		if (entry_payload_decrypt(entry) < 0) {
			errsv = errno;
			log_warn("process_daemon_recv_update(): entry_payload_decrypt(): %s\n", strerror(errno));
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

