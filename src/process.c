/**
 * @file process.c
 * @brief uSched
 *        Data Processing interface
 *
 * Date: 13-07-2014
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

	debug_printf(DEBUG_INFO, "PAYLOAD: %s\n", entry->payload);

	/* The payload of a NEW entry is the entry subject. */
	if (entry_set_subj(entry, entry->payload, entry->psize)) {
		errsv = errno;
		log_warn("_process_recv_update_op_new(): entry_set_subj(): %s\n", strerror(errno));
		goto _update_op_new_failure_1;
	}

	/* Clear payload information */
	entry_unset_payload(entry);

	/* Pop the entry from the pool */
	pthread_mutex_lock(&rund.mutex_rpool);
	entry = rund.rpool->pope(rund.rpool, entry); 
	pthread_mutex_unlock(&rund.mutex_rpool);

	/* Ensure that entry is still valid */
	if (!entry) {
		log_warn("_process_recv_update_op_new(): entry == NULL after rund.rpool->pope().\n");
		goto _update_op_new_failure_1;
	}

	/* We're done. Now we need to install and set a global and unique id for this entry */
	if (schedule_entry_create(entry) < 0) {
		errsv = errno;
		log_warn("_process_recv_update_op_new(): schedule_entry_create(): %s\n", strerror(errno));

		/* NOTE: This is a special case: The current entry is no longer in the rpool and wasn't inserted into
		 * the active pool (apool) due to errors.
		 * Since the entry was already pop'd, we don't need to do it here.
		 */

		goto _update_op_new_failure_1;
	}

	/* NOTE: After schedule_entry_create() success, a new and unique entry->id is now set. */

	/* Reuse 'aop' to reply the entry->id to the client */
	mm_free((void *) aop->data);

	memset(aop, 0, sizeof(struct async_op));

	aop->fd = cur_fd;
	aop->count = sizeof(entry->id);
	aop->priority = 0;
	aop->timeout.tv_sec = CONFIG_USCHED_CONN_TIMEOUT;

	if (!(aop->data = mm_alloc(sizeof(entry->id)))) {
		errsv = errno;
		log_warn("_process_recv_update_op_new(): aop->data = mm_alloc(%zu): %s\n", sizeof(entry->id), strerror(errno));

		goto _update_op_new_failure_2;
	}

	memcpy((void *) aop->data, (uint64_t [1]) { htonll(entry->id) }, sizeof(entry->id));

	debug_printf(DEBUG_INFO, "Delivering entry id: %llu\n", entry->id);

	/* Report the unique Entry ID back to the client */
	if (rtsaio_write(aop) < 0) {
		errsv = errno;
		log_warn("_process_recv_update_op_new(): rtsaio_write(): %s\n", strerror(errno));

		goto _update_op_new_failure_2;
	}

	return 0;

_update_op_new_failure_2:
	/* If we're unable to comunicate with the client, the scheduled entry should be disabled and pop'd from apool */
	if (!schedule_entry_disable(entry)) {
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
	uint32_t entry_list_req_size = 0, entry_list_res_size = 0;

	/* Pop the entry from the pool */
	pthread_mutex_lock(&rund.mutex_rpool);
	entry = rund.rpool->pope(rund.rpool, entry);
	pthread_mutex_unlock(&rund.mutex_rpool);

	/* Ensure that entry is still valid */
	if (!entry) {
		log_warn("_process_recv_update_op_del(): entry == NULL after rund.rpool->pope()\n");
		goto _update_op_del_failure_1;
	}

	/* Check if the payload size if aligned with the entry->id size */
	if ((entry->psize % sizeof(entry->id))) {
		log_warn("_process_recv_update_op_del(): entry->psize %% sizeof(entry->id) != 0\n");
		goto _update_op_del_failure_1;
	}

	/* Gather the request entry list and respective list size */
	entry_list_req = (uint64_t *) entry->payload;
	entry_list_req_size = entry->psize / sizeof(entry->id);

	/* entry_list_req_size should never be 0 since we grant that entry->psize != 0 in the entry creation stage.
	 * Anyway, since we're dealing with integers and entry->psize < sizeof(entry->id) is accepted in the pre-checks,
	 * lets play safe and grant that entry_list_req_size != 0.
	 */
	if (!entry_list_req_size) {
		/* Someone is probably trying something nasty */
		log_warn("_process_recv_update_op_del(): entry_list_req_size == 0 (Possible exploit attempt)\n");
		goto _update_op_del_failure_1;
	}

	/* Iterate payload, ensure the user that's requesting the deletion is authorized to do so, and delete each of
	 * the valid entries through schedule_delete_entry().
	 */
	for (i = 0, entry_list_res_size = 0; i < entry_list_req_size; i ++) {
		if (schedule_entry_ownership_delete_by_id(entry_list_req[i], entry->uid) < 0) {
			log_warn("_process_recv_update_op_del(): schedule_entry_ownership_delete_by_id(): %s\n", strerror(errno));
			entry_list_res_size ++;

			if (!(entry_list_res = mm_realloc(entry_list_res, entry_list_res_size))) {
				errsv = errno;
				log_warn("_process_recv_update_op_del(): realloc(): %s\n", strerror(errno));
				goto _update_op_del_failure_1;
			}

			/* Set early network byte order, as this list won't be used locally */
			entry_list_res[entry_list_res_size - 1] = htonll(entry_list_req[i]);

			continue;
		}
	}

	/* Report back the failed entries. */

	/* Reuse 'aop' to reply the failed deleted entries to the client */
	mm_free((void *) aop->data);

	memset(aop, 0, sizeof(struct async_op));

	aop->fd = cur_fd;
	aop->count = (sizeof(entry->id) * entry_list_res_size) + sizeof(entry_list_res_size);
	aop->priority = 0;
	aop->timeout.tv_sec = CONFIG_USCHED_CONN_TIMEOUT;

	if (!(aop->data = mm_alloc(aop->count))) {
		errsv = errno;
		log_warn("_process_recv_update_op_del(): aop->data = mm_alloc(%zu): %s\n", aop->count, strerror(errno));

		goto _update_op_del_failure_1;
	}

	/* Craft the list size at the head of the packet. */
	memcpy((void *) aop->data, (uint32_t [1]) { htonl(entry_list_res_size) }, 4);
	/* Append the list contents right after the list size field */
	memcpy((void *) (((char *) aop->data) + 4), entry_list_res, aop->count - 4);

	debug_printf(DEBUG_INFO, "Delivering %lu entry ID's that failed to be deleted.\n", entry_list_res_size);

	/* Report back the failed entries to the client */
	if (rtsaio_write(aop) < 0) {
		errsv = errno;
		log_warn("_process_recv_update_op_del(): rtsaio_write(): %s\n", strerror(errno));

		goto _update_op_del_failure_1;
	}

	return 0;

_update_op_del_failure_1:
	errno = errsv;

	return -1;
}

static int _process_recv_update_op_get(struct async_op *aop, struct usched_entry *entry) {
	/* TODO: To be implemented */

	/* TODO: Check if the requested entries to be fetched are flagged as FINISH. Operations other than NEW over
	 * an unfinished entry shall be discarded and logged
	 */

	return -1;
}

struct usched_entry *process_recv_create(struct async_op *aop) {
	int errsv = 0;
	struct usched_entry *entry = NULL;

	/* Process the received entry data */
	if (!(entry = mm_alloc(sizeof(struct usched_entry)))) {
		errsv = errno;
		log_warn("process_recv_create(): entry = mm_alloc(): %s\n", strerror(errno));

		goto _create_failure_1;
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
		log_warn("process_recv_create(): The requested operation is invalid.\n");

		goto _create_failure_2;
	}

	/* Validate payload size */
	if (!entry->psize) {
		/* All entry requests expect a payload. If none is set, this entry request is invalid. */
		errsv = errno;
		log_warn("process_recv_create(): entry->psize == 0.\n");

		goto _create_failure_2;
	}

	debug_printf(DEBUG_INFO, "psize: %u\n", entry->psize);

	/* Insert this entry into the rpool */
	if (rund.rpool->insert(rund.rpool, entry) < 0) {
		errsv = errno;
		log_warn("process_recv_create(): rund.rpool->insert(): %s\n", strerror(errno));

		/* NOTE: In this special case, we do not need to pop the entry from the rpool as we were unable
		 * to insert it.
		 */
		goto _create_failure_2;
	}

	/* Reuse 'aop' for next read request: Receive the entry command */
	memset(aop, 0, sizeof(struct async_op));

	aop->fd = entry->id;
	aop->count = entry->psize;
	aop->priority = 0;
	aop->timeout.tv_sec = CONFIG_USCHED_CONN_TIMEOUT;

	if (!(aop->data = mm_alloc(aop->count))) {
		errsv = errno;
		log_warn("process_recv_create(): aop->data = mm_alloc(): %s\n", strerror(errno));

		goto _create_failure_3;
	}

	memset((void *) aop->data, 0, aop->count);

	/* Request the entry payload */
	if (rtsaio_read(aop) < 0) {
		errsv = errno;
		log_warn("process_recv_create(): rtsaio_read(): %s\n", strerror(errno));

		goto _create_failure_3;
	}

	/* Set the initialization flag as this is a new entry */
	entry_set_flag(entry, USCHED_ENTRY_FLAG_INIT);

	return entry;

_create_failure_3:
	/* Pop the entry from rpool without destroying it */
	pthread_mutex_lock(&rund.mutex_rpool);
	entry = rund.rpool->pope(rund.rpool, entry);
	pthread_mutex_unlock(&rund.mutex_rpool);

	if (!entry) {
		log_warn("process_recv_create(): _create_failure_3: entry == NULL after rund.rpool->pope()\n");
		goto _create_failure_1;
	}

_create_failure_2:
	/* Destroy the entry */
	entry_destroy(entry);

_create_failure_1:
	errno = errsv;

	return NULL;
}

int process_recv_update(struct async_op *aop, struct usched_entry *entry) {
	int errsv = 0;

	/* Check if the entry is initialized */
	if (!entry_has_flag(entry, USCHED_ENTRY_FLAG_INIT)) {
		errsv = errno;
		log_warn("process_recv_update(): Trying to update an existing entry that wasn't initialized yet.\n");

		goto _update_failure_3;
	}

	/* Check if the entry is authorized. If not, authorize it and try to proceed. */
	if (!entry_has_flag(entry, USCHED_ENTRY_FLAG_AUTHORIZED) && (entry_authorize(entry, aop->fd) < 0)) {
		errsv = errno;
		log_warn("process_recv_update(): entry_authorize(): %s\n", strerror(errno));

		goto _update_failure_3;
	}

	/* Re-validate authorization. If not authorized, discard this entry */
	if (!entry_has_flag(entry, USCHED_ENTRY_FLAG_AUTHORIZED)) {
		errsv = errno;
		log_warn("process_recv_update(): Unauthorized entry\n", strerror(errno));

		goto _update_failure_3;
	}

	debug_printf(DEBUG_INFO, "psize: %u, aop->count: %zu\n", entry->psize, aop->count);

	/* Grant that the received data does not exceed the expected size */
	if (aop->count != entry->psize) {
		errsv = errno;
		log_warn("process_recv_update(): aop->count != entry->psize\n");

		goto _update_failure_3;
	}

	/* Set the received entry payload */
	if (entry_set_payload(entry, (char *) aop->data, entry->psize) < 0) {
		errsv = errno;
		log_warn("process_recv_update(): entry_set_payload(): %s\n", strerror(errno));

		goto _update_failure_3;
	}

	/* Process specific operation types */
	if (entry_has_flag(entry, USCHED_ENTRY_FLAG_NEW)) {
		if (_process_recv_update_op_new(aop, entry) < 0) {
			errsv = errno;
			log_warn("process_recv_update(): _process_recv_update_op_new(): %s\n", strerror(errno));

			goto _update_failure_2;
		}
	} else if (entry_has_flag(entry, USCHED_ENTRY_FLAG_DEL)) {
		if (_process_recv_update_op_del(aop, entry) < 0) {
			errsv = errno;
			log_warn("process_recv_update(): _process_recv_update_op_del(): %s\n", strerror(errno));

			goto _update_failure_2;
		}
	} else if (entry_has_flag(entry, USCHED_ENTRY_FLAG_GET)) {
		if (_process_recv_update_op_get(aop, entry) < 0) {
			errsv = errno;
			log_warn("process_recv_update(): _process_recv_update_op_get(): %s\n", strerror(errno));

			goto _update_failure_2;
		}
	} else {
		log_warn("process_recv_update(): The requested operation is invalid.\n");

		/* Since no _process_recv_update_op_*() function was called, the current entry is still present in
		 * the rpool. We need to delete it in this context.
		 */
		goto _update_failure_3;
	}

	/* Set the finishing flag as this entry is now fully processed. */
	entry_set_flag(entry, USCHED_ENTRY_FLAG_FINISH);

	return 0;

_update_failure_3:
	/* Pop the entry from rpool. */
	pthread_mutex_lock(&rund.mutex_rpool);
	entry = rund.rpool->pope(rund.rpool, entry);
	pthread_mutex_unlock(&rund.mutex_rpool);

	if (!entry) {
		log_warn("process_recv_update(): _update_failure_3: entry == NULL after rund.rpool->pope()\n");
		goto _update_failure_1;
	}

_update_failure_2:
	/* Destroy the current entry in this context, as it was already pop'd from the rpool */
	entry_destroy(entry);

_update_failure_1:
	errno = errsv;

	return -1;
}
