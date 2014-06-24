/**
 * @file notify.c
 * @brief uSched
 *        I/O Notification interface
 *
 * Date: 24-06-2014
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
#include "runtime.h"
#include "mm.h"
#include "notify.h"
#include "log.h"
#include "entry.h"
#include "schedule.h"

void notify_read(struct async_op *aop) {
	struct usched_entry *entry = NULL;
	int cur_fd = aop->fd;
	int ret = 0;

	if ((rtsaio_status(aop) == ASYNCOP_STATUS_COMPLETE) && (rtsaio_count(aop) == aop->count)) {
		/* Successfully received possible valid data */

		/* Search for an existing entry. If found, the received data belongs to the entry command */
		pthread_mutex_lock(&rund.mutex_rpool);
		entry = rund.rpool->search(rund.rpool, usched_entry_id(aop->fd));
		pthread_mutex_unlock(&rund.mutex_rpool);

		if (entry) {
			/* Check if the entry is authorized. If not, authorize it and try to proceed. */
			if (!entry_has_flag(entry, USCHED_ENTRY_FLAG_AUTHORIZED) && ((ret = entry_authorize(entry, aop->fd)) < 0)) {
				log_warn("notify_read(): entry_authorize(): %s\n", strerror(errno));

				pthread_mutex_lock(&rund.mutex_rpool);
				rund.rpool->del(rund.rpool, entry);
				pthread_mutex_unlock(&rund.mutex_rpool);

				goto _read_failure;
			}

			/* Re-validate authorization. If not authorized, discard this entry */
			if (!entry_has_flag(entry, USCHED_ENTRY_FLAG_AUTHORIZED)) {
				log_warn("notify_read(): Unauthorized entry\n", strerror(errno));

				pthread_mutex_lock(&rund.mutex_rpool);
				rund.rpool->del(rund.rpool, entry);
				pthread_mutex_unlock(&rund.mutex_rpool);

				goto _read_failure;
			}

			/* Process the received entry command */
			debug_printf(DEBUG_INFO, "cmd_size: %u, aop->count: %zu\n", entry->cmd_size, aop->count);

			/* Grant that the received data does not exceed the expected size */
			if (aop->count != entry->cmd_size) {
				log_warn("notify_read(): aop->count != entry->cmd_size\n");

				pthread_mutex_lock(&rund.mutex_rpool);
				rund.rpool->del(rund.rpool, entry);
				pthread_mutex_unlock(&rund.mutex_rpool);

				goto _read_failure;
			}

			/* Set the received entry command */
			if (entry_set_cmd(entry, (char *) aop->data, entry->cmd_size) < 0) {
				log_warn("notify_read(): entry_set_cmd(): %s\n", strerror(errno));

				pthread_mutex_lock(&rund.mutex_rpool);
				rund.rpool->del(rund.rpool, entry);
				pthread_mutex_unlock(&rund.mutex_rpool);

				goto _read_failure;
			}

			debug_printf(DEBUG_INFO, "CMD: %s\n", entry->cmd);

			/* Pop the entry from the pool */
			pthread_mutex_lock(&rund.mutex_rpool);
			rund.rpool->pope(rund.rpool, entry); 
			pthread_mutex_unlock(&rund.mutex_rpool);

			/* We're done. Now we need to install and set a global and unique id for this entry */
			if (schedule_entry_create(entry) < 0) {
				log_warn("notify_read(): schedule_entry_create(): %s\n", strerror(errno));

				goto _read_failure;
			}

			/* NOTE: After schedule_entry_create() success, a new and unique entry->id is now set. */

			/* Reuse 'aop' to reply to the client */
			mm_free((void *) aop->data);

			memset(aop, 0, sizeof(struct async_op));

			aop->fd = cur_fd;
			aop->count = 4;
			aop->priority = 0;
			aop->timeout.tv_sec = CONFIG_USCHED_CONN_TIMEOUT;

			if (!(aop->data = mm_alloc(4))) {
				log_warn("notify_read(): aop->data = mm_alloc(4): %s\n", strerror(errno));

				pthread_mutex_lock(&rund.mutex_apool);
				rund.apool->del(rund.apool, entry);
				pthread_mutex_unlock(&rund.mutex_apool);

				goto _read_failure;
			}

			memcpy((void *) aop->data, (uint32_t [1]) { htonl(entry->id) }, 4);

			debug_printf(DEBUG_INFO, "Delivering entry id: %u\n", entry->id);

			/* Report the unique Entry ID back to the client */
			if (rtsaio_write(aop) < 0) {
				log_warn("notify_read(): rtsaio_write(): %s\n", strerror(errno));

				pthread_mutex_lock(&rund.mutex_apool);
				rund.apool->del(rund.apool, entry);
				pthread_mutex_unlock(&rund.mutex_apool);

				goto _read_failure;
			}
		} else {
			/* Process the received entry data */
			if (!(entry = mm_alloc(sizeof(struct usched_entry)))) {
				log_warn("notify_read(): entry = mm_alloc(): %s\n", strerror(errno));

				goto _read_failure;
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
				log_warn("notify_read(): rund.rpool->insert(): %s\n", strerror(errno));

				entry_destroy(entry);

				goto _read_failure;
			}

			mm_free((void *) aop->data);

			/* Reuse 'aop' for next read request: Receive the entry command */
			memset(aop, 0, sizeof(struct async_op));

			aop->fd = entry->id;
			aop->count = entry->cmd_size;
			aop->priority = 0;
			aop->timeout.tv_sec = CONFIG_USCHED_CONN_TIMEOUT;

			if (!(aop->data = mm_alloc(aop->count))) {
				log_warn("notify_read(): aop->data = mm_alloc(): %s\n", strerror(errno));

				pthread_mutex_lock(&rund.mutex_rpool);
				rund.rpool->del(rund.rpool, entry);
				pthread_mutex_unlock(&rund.mutex_rpool);

				goto _read_failure;
			}

			memset((void *) aop->data, 0, aop->count);

			/* Request the entry command */
			if (rtsaio_read(aop) < 0) {
				log_warn("notify_read(): rtsaio_read(): %s\n", strerror(errno));

				pthread_mutex_lock(&rund.mutex_rpool);
				rund.rpool->del(rund.rpool, entry);
				pthread_mutex_unlock(&rund.mutex_rpool);

				goto _read_failure;
			}
		}

		return;
	}

_read_failure:
	/* Discard connection */
	panet_safe_close(aop->fd);

	if ((entry = rund.rpool->search(rund.rpool, usched_entry_id(aop->fd)))) {
		pthread_mutex_lock(&rund.mutex_rpool);
		rund.rpool->del(rund.rpool, entry);
		pthread_mutex_unlock(&rund.mutex_rpool);
	}

	if (aop->data)
		mm_free((void *) aop->data);

	mm_free(aop);
}

void notify_write(struct async_op *aop) {
	int cur_fd = aop->fd;

	if ((rtsaio_status(aop) == ASYNCOP_STATUS_COMPLETE) && (rtsaio_count(aop) == aop->count)) {
		if (aop->data)
			mm_free((void *) aop->data);

		/* Do another rtsaio_read() of 32 bytes in order to wait for another entry */
		memset(aop, 0, sizeof(struct async_op));

		aop->fd = cur_fd;
		aop->count = 32;
		aop->priority = 0;
		aop->timeout.tv_sec = CONFIG_USCHED_CONN_TIMEOUT;

		if (!(aop->data = mm_alloc(aop->count))) {
			log_warn("notify_write(): aop->data = mm_alloc(): %s\n", strerror(errno));

			goto _write_failure;
		}

		memset((void *) aop->data, 0, aop->count);

		if (rtsaio_read(aop) < 0) {
			log_warn("notify_write(): rtsaio_read(): %s\n", strerror(errno));

			goto _write_failure;
		}

		return;
	}

_write_failure:
	panet_safe_close(aop->fd);

	if (aop->data)
		mm_free((void *) aop->data);

	mm_free(aop);
}

