/**
 * @file process.c
 * @brief uSched
 *        Data Processing interface
 *
 * Date: 12-07-2014
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

#include <rtsaio/rtsaio.h>

#include "config.h"
#include "debug.h"
#include "mm.h"
#include "runtime.h"
#include "entry.h"
#include "log.h"
#include "schedule.h"
#include "conn.h"

static int _process_recv_create_op_new(struct async_op *aop, struct usched_entry *entry) {
	int errsv = 0;

	debug_printf(DEBUG_INFO, "psize: %u\n", entry->psize);

	if (rund.rpool->insert(rund.rpool, entry) < 0) {
		errsv = errno;
		log_warn("process_recv_create(): rund.rpool->insert(): %s\n", strerror(errno));

		/* NOTE: In this special case, we do not need to pop the entry from the rpool as we were unable
		 * to insert it.
		 */
		errno = errsv;

		return -1;
	}

	mm_free((void *) aop->data);

	if (!entry->psize) {
		/* A NEW entry shall contain payload (for a command or for authentication).
		 * If no payload is present at this stage, this entry is invalid.
		 */
		errsv = errno;
		log_warn("_process_recv_create_op_new(): entry->psize == 0.\n");

		/* Pop the entry from rpool, but do not delete it. Entry should be free'd in the original allocation
		 * context.
		 */
		pthread_mutex_lock(&rund.mutex_rpool);
		rund.rpool->pope(rund.rpool, entry); 
		pthread_mutex_unlock(&rund.mutex_rpool);

		errno = errsv;

		return -1;
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

		/* Pop the entry from rpool, but do not delete it. Entry should be free'd in the original allocation
		 * context.
		 */
		pthread_mutex_lock(&rund.mutex_rpool);
		rund.rpool->pope(rund.rpool, entry); 
		pthread_mutex_unlock(&rund.mutex_rpool);

		errno = errsv;

		return -1;
	}

	memset((void *) aop->data, 0, aop->count);

	/* Request the entry command */
	if (rtsaio_read(aop) < 0) {
		errsv = errno;
		log_warn("process_recv_create(): rtsaio_read(): %s\n", strerror(errno));

		/* Pop the entry from rpool, but do not delete it. Entry should be free'd in the original allocation
		 * context.
		 */
		pthread_mutex_lock(&rund.mutex_rpool);
		rund.rpool->pope(rund.rpool, entry); 
		pthread_mutex_unlock(&rund.mutex_rpool);

		errno = errsv;

		return -1;
	}

	return 0;
}

static int _process_recv_create_op_del(struct async_op *aop, struct usched_entry *entry) {
	/* TODO: To be implemented */
	return -1;
}

static int _process_recv_create_op_get(struct async_op *aop, struct usched_entry *entry) {
	/* TODO: To be implemented */
	return -1;
}

static int _process_recv_update_op_new(struct async_op *aop, struct usched_entry *entry) {
	int errsv = 0;
	int cur_fd = aop->fd;

	/* Process the received entry command */
	debug_printf(DEBUG_INFO, "psize: %u, aop->count: %zu\n", entry->psize, aop->count);

	/* Grant that the received data does not exceed the expected size */
	if (aop->count != entry->psize) {
		errsv = errno;
		log_warn("process_recv_update(): aop->count != entry->psize\n");

		/* Pop the entry from rpool, but do not delete it. Entry should be free'd in the original allocation
		 * context.
		 */
		pthread_mutex_lock(&rund.mutex_rpool);
		rund.rpool->pope(rund.rpool, entry); 
		pthread_mutex_unlock(&rund.mutex_rpool);

		errno = errsv;

		return -1;
	}

	/* Set the received entry command */
	if (entry_set_payload(entry, (char *) aop->data, entry->psize) < 0) {
		errsv = errno;
		log_warn("process_recv_update(): entry_set_payload(): %s\n", strerror(errno));

		/* Pop the entry from rpool, but do not delete it. Entry should be free'd in the original allocation
		 * context.
		 */
		pthread_mutex_lock(&rund.mutex_rpool);
		rund.rpool->pope(rund.rpool, entry); 
		pthread_mutex_unlock(&rund.mutex_rpool);

		errno = errsv;

		return -1;
	}

	debug_printf(DEBUG_INFO, "CMD: %s\n", entry->payload);

	/* Pop the entry from the pool */
	pthread_mutex_lock(&rund.mutex_rpool);
	rund.rpool->pope(rund.rpool, entry); 
	pthread_mutex_unlock(&rund.mutex_rpool);

	/* We're done. Now we need to install and set a global and unique id for this entry */
	if (schedule_entry_create(entry) < 0) {
		errsv = errno;
		log_warn("process_recv_update(): schedule_entry_create(): %s\n", strerror(errno));

		/* NOTE: This is a special case: The current entry is no longer in the rpool and wasn't inserted into
		 * the active pool (apool) due to errors.
		 * Since the entry was already pop'd, we don't need to do it here.
		 */

		errno = errsv;

		return -1;
	}

	/* NOTE: After schedule_entry_create() success, a new and unique entry->id is now set. */

	/* Reuse 'aop' to reply the entry->id to the client */
	mm_free((void *) aop->data);

	memset(aop, 0, sizeof(struct async_op));

	aop->fd = cur_fd;
	aop->count = sizeof(entry->id);
	aop->priority = 0;
	aop->timeout.tv_sec = CONFIG_USCHED_CONN_TIMEOUT;

	if (!(aop->data = mm_alloc(4))) {
		errsv = errno;
		log_warn("process_recv_update(): aop->data = mm_alloc(4): %s\n", strerror(errno));
		/* If we're unable to comunicate with the client, the scheduled entry should be destroyed */
		schedule_entry_delete(entry->id);
		errno = errsv;

		return -1;
	}

	memcpy((void *) aop->data, (uint64_t [1]) { htonll(entry->id) }, sizeof(entry->id));

	debug_printf(DEBUG_INFO, "Delivering entry id: %llu\n", entry->id);

	/* Report the unique Entry ID back to the client */
	if (rtsaio_write(aop) < 0) {
		errsv = errno;
		log_warn("process_recv_update(): rtsaio_write(): %s\n", strerror(errno));
		/* If we're unable to comunicate with the client, the scheduled entry should be destroyed */
		schedule_entry_delete(entry->id);
		errno = errsv;

		return -1;
	}

	return 0;
}

static int _process_recv_update_op_del(struct async_op *aop, struct usched_entry *entry) {
	/* TODO: To be implemented */
	return -1;
}

static int _process_recv_update_op_get(struct async_op *aop, struct usched_entry *entry) {
	/* TODO: To be implemented */
	return -1;
}

struct usched_entry *process_recv_create(struct async_op *aop) {
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

	/* Set the initialization flag as this is a new entry */
	entry_set_flag(entry, USCHED_ENTRY_FLAG_INIT);

	/* Process specific operation types */
	if (entry_has_flag(entry, USCHED_ENTRY_FLAG_NEW)) {
		if (_process_recv_create_op_new(aop, entry) < 0) {
			errsv = errno;
			log_warn("process_recv_create(): _process_recv_create_op_new(): %s\n", strerror(errno));
			/* Destroy the current entry */
			entry_destroy(entry);
			errno = errsv;

			return NULL;
		}
	} else if (entry_has_flag(entry, USCHED_ENTRY_FLAG_DEL)) {
		if (_process_recv_create_op_del(aop, entry) < 0) {
			errsv = errno;
			log_warn("process_recv_create(): _process_recv_create_op_del(): %s\n", strerror(errno));
			/* Destroy the current entry */
			entry_destroy(entry);
			errno = errsv;

			return NULL;
		}
	} else if (entry_has_flag(entry, USCHED_ENTRY_FLAG_GET)) {
		if (_process_recv_create_op_get(aop, entry) < 0) {
			errsv = errno;
			log_warn("process_recv_create(): _process_recv_create_op_get(): %s\n", strerror(errno));
			/* Destroy the current entry */
			entry_destroy(entry);
			errno = errsv;

			return NULL;
		}
	} else {
		log_warn("process_recv_create(): The requested operation is invalid.\n");

		/* Destroy the current entry */
		entry_destroy(entry);

		return NULL;
	}

	return entry;
}

int process_recv_update(struct async_op *aop, struct usched_entry *entry) {
	int errsv = 0;

	/* Check if the entry is initialized */
	if (!entry_has_flag(entry, USCHED_ENTRY_FLAG_INIT)) {
		errsv = errno;
		log_warn("process_recv_update(): Trying to update an existing entry that wasn't initialized yet.\n");

		/* Delete the entry from rpool. */
		pthread_mutex_lock(&rund.mutex_rpool);
		rund.rpool->del(rund.rpool, entry);
		pthread_mutex_unlock(&rund.mutex_rpool);

		errno = errsv;

		return -1;
	}

	/* Check if the entry is authorized. If not, authorize it and try to proceed. */
	if (!entry_has_flag(entry, USCHED_ENTRY_FLAG_AUTHORIZED) && (entry_authorize(entry, aop->fd) < 0)) {
		errsv = errno;
		log_warn("process_recv_update(): entry_authorize(): %s\n", strerror(errno));

		/* Delete the entry from rpool. */
		pthread_mutex_lock(&rund.mutex_rpool);
		rund.rpool->del(rund.rpool, entry);
		pthread_mutex_unlock(&rund.mutex_rpool);

		errno = errsv;

		return -1;
	}

	/* Re-validate authorization. If not authorized, discard this entry */
	if (!entry_has_flag(entry, USCHED_ENTRY_FLAG_AUTHORIZED)) {
		errsv = errno;
		log_warn("process_recv_update(): Unauthorized entry\n", strerror(errno));

		/* Delete the entry from rpool. */
		pthread_mutex_lock(&rund.mutex_rpool);
		rund.rpool->del(rund.rpool, entry);
		pthread_mutex_unlock(&rund.mutex_rpool);

		errno = errsv;

		return -1;
	}

	/* Process specific operation types */
	if (entry_has_flag(entry, USCHED_ENTRY_FLAG_NEW)) {
		if (_process_recv_update_op_new(aop, entry) < 0) {
			errsv = errno;
			log_warn("process_recv_update(): _process_recv_update_op_new(): %s\n", strerror(errno));
			/* Destroy the current entry in this context, as it was already pop'd from the rpool */
			entry_destroy(entry);
			errno = errsv;

			return -1;
		}
	} else if (entry_has_flag(entry, USCHED_ENTRY_FLAG_DEL)) {
		if (_process_recv_update_op_del(aop, entry) < 0) {
			errsv = errno;
			log_warn("process_recv_update(): _process_recv_update_op_del(): %s\n", strerror(errno));
			/* Destroy the current entry in this context, as it was already pop'd from the rpool */
			entry_destroy(entry);
			errno = errsv;

			return -1;
		}
	} else if (entry_has_flag(entry, USCHED_ENTRY_FLAG_GET)) {
		if (_process_recv_update_op_get(aop, entry) < 0) {
			errsv = errno;
			log_warn("process_recv_update(): _process_recv_update_op_get(): %s\n", strerror(errno));
			/* Destroy the current entry in this context, as it was already pop'd from the rpool */
			entry_destroy(entry);
			errno = errsv;

			return -1;
		}
	} else {
		log_warn("process_recv_update(): The requested operation is invalid.\n");

		/* Since no _process_recv_update_op_*() function was called, the current entry is still present in
		 * the rpool. We need to delete it in this context.
		 */
		pthread_mutex_lock(&rund.mutex_rpool);
		rund.rpool->del(rund.rpool, entry);
		pthread_mutex_unlock(&rund.mutex_rpool);

		return -1;
	}

	return 0;
}

