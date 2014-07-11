/**
 * @file process.c
 * @brief uSched
 *        Data Processing interface
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

struct usched_entry *process_recv_create(struct async_op *aop) {
	struct usched_entry *entry = NULL;

	/* Process the received entry data */
	if (!(entry = mm_alloc(sizeof(struct usched_entry)))) {
		log_warn("process_recv_create(): entry = mm_alloc(): %s\n", strerror(errno));

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
	entry_set_cmd_size(entry, ntohl(entry->cmd_size));

	/* Clear all local flags that the client have possibly set */
	entry_unset_flags_local(entry);

	debug_printf(DEBUG_INFO, "cmd_size: %u\n", entry->cmd_size);

	if (rund.rpool->insert(rund.rpool, entry) < 0) {
		log_warn("process_recv_create(): rund.rpool->insert(): %s\n", strerror(errno));

		entry_destroy(entry);

		return NULL;
	}

	mm_free((void *) aop->data);

	if (!entry->cmd_size) {
		/* TODO: If the entry size is zero, don't try to receive anything and process the
		 * the response at this point.
		 */
		//mm_free((void *) aop);
		//return;
	}

	/* Reuse 'aop' for next read request: Receive the entry command */
	memset(aop, 0, sizeof(struct async_op));

	aop->fd = entry->id;
	aop->count = entry->cmd_size;
	aop->priority = 0;
	aop->timeout.tv_sec = CONFIG_USCHED_CONN_TIMEOUT;

	if (!(aop->data = mm_alloc(aop->count))) {
		log_warn("process_recv_create(): aop->data = mm_alloc(): %s\n", strerror(errno));

		pthread_mutex_lock(&rund.mutex_rpool);
		rund.rpool->del(rund.rpool, entry);
		pthread_mutex_unlock(&rund.mutex_rpool);

		return NULL;
	}

	memset((void *) aop->data, 0, aop->count);

	/* Request the entry command */
	if (rtsaio_read(aop) < 0) {
		log_warn("process_recv_create(): rtsaio_read(): %s\n", strerror(errno));

		pthread_mutex_lock(&rund.mutex_rpool);
		rund.rpool->del(rund.rpool, entry);
		pthread_mutex_unlock(&rund.mutex_rpool);

		return NULL;
	}

	return entry;
}

int process_recv_update(struct async_op *aop, struct usched_entry *entry) {
	int cur_fd = aop->fd;

	/* Check if the entry is authorized. If not, authorize it and try to proceed. */
	if (!entry_has_flag(entry, USCHED_ENTRY_FLAG_AUTHORIZED) && (entry_authorize(entry, aop->fd) < 0)) {
		log_warn("process_recv_update(): entry_authorize(): %s\n", strerror(errno));

		pthread_mutex_lock(&rund.mutex_rpool);
		rund.rpool->del(rund.rpool, entry);
		pthread_mutex_unlock(&rund.mutex_rpool);

		return -1;
	}

	/* Re-validate authorization. If not authorized, discard this entry */
	if (!entry_has_flag(entry, USCHED_ENTRY_FLAG_AUTHORIZED)) {
		log_warn("process_recv_update(): Unauthorized entry\n", strerror(errno));

		pthread_mutex_lock(&rund.mutex_rpool);
		rund.rpool->del(rund.rpool, entry);
		pthread_mutex_unlock(&rund.mutex_rpool);

		return -1;
	}

	/* Process the received entry command */
	debug_printf(DEBUG_INFO, "cmd_size: %u, aop->count: %zu\n", entry->cmd_size, aop->count);

	/* Grant that the received data does not exceed the expected size */
	if (aop->count != entry->cmd_size) {
		log_warn("process_recv_update(): aop->count != entry->cmd_size\n");

		pthread_mutex_lock(&rund.mutex_rpool);
		rund.rpool->del(rund.rpool, entry);
		pthread_mutex_unlock(&rund.mutex_rpool);

		return -1;
	}

	/* Set the received entry command */
	if (entry_set_cmd(entry, (char *) aop->data, entry->cmd_size) < 0) {
		log_warn("process_recv_update(): entry_set_cmd(): %s\n", strerror(errno));

		pthread_mutex_lock(&rund.mutex_rpool);
		rund.rpool->del(rund.rpool, entry);
		pthread_mutex_unlock(&rund.mutex_rpool);

		return -1;
	}

	debug_printf(DEBUG_INFO, "CMD: %s\n", entry->cmd);

	/* Pop the entry from the pool */
	pthread_mutex_lock(&rund.mutex_rpool);
	rund.rpool->pope(rund.rpool, entry); 
	pthread_mutex_unlock(&rund.mutex_rpool);

	/* We're done. Now we need to install and set a global and unique id for this entry */
	if (schedule_entry_create(entry) < 0) {
		log_warn("process_recv_update(): schedule_entry_create(): %s\n", strerror(errno));

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
		log_warn("process_recv_update(): aop->data = mm_alloc(4): %s\n", strerror(errno));

		pthread_mutex_lock(&rund.mutex_apool);
		rund.apool->del(rund.apool, entry);
		pthread_mutex_unlock(&rund.mutex_apool);

		return -1;
	}

	memcpy((void *) aop->data, (uint64_t [1]) { htonll(entry->id) }, sizeof(entry->id));

	debug_printf(DEBUG_INFO, "Delivering entry id: %llu\n", entry->id);

	/* Report the unique Entry ID back to the client */
	if (rtsaio_write(aop) < 0) {
		log_warn("process_recv_update(): rtsaio_write(): %s\n", strerror(errno));

		pthread_mutex_lock(&rund.mutex_apool);
		rund.apool->del(rund.apool, entry);
		pthread_mutex_unlock(&rund.mutex_apool);

		return -1;
	}

	return 0;
}

