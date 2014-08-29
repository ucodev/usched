/**
 * @file notify.c
 * @brief uSched
 *        I/O Notification interface
 *
 * Date: 29-08-2014
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
#include "conn.h"
#include "process.h"
#include "gc.h"

void notify_read(struct async_op *aop) {
	struct usched_entry *entry = NULL;

	if ((rtsaio_status(aop) == ASYNCOP_STATUS_COMPLETE) && (rtsaio_count(aop) == aop->count)) {
		/* Successfully received possible valid data */

		/* Search for an existing entry. If found, the received data belongs to the entry command */
		pthread_mutex_lock(&rund.mutex_rpool);
		entry = rund.rpool->pope(rund.rpool, usched_entry_id(aop->fd));
		pthread_mutex_unlock(&rund.mutex_rpool);

		if (entry) {
			if (process_daemon_recv_update(aop, entry) < 0) {
				log_warn("notify_read(): process_daemon_recv_update(): %s\n", strerror(errno));
				goto _read_failure;
			}
		} else if (!(entry = process_daemon_recv_create(aop))) {
			log_warn("notify_read(): process_daemon_recv_create(): %s\n", strerror(errno));
			goto _read_failure;
		}

		/* Re-insert the entry into the rpool before any I/O */
		pthread_mutex_lock(&rund.mutex_rpool);

		if (rund.rpool->insert(rund.rpool, entry) < 0) {
			log_warn("notify_read(): rund.rpool->insert(): %s\n", strerror(errno));
			goto _read_failure;
		}

		pthread_mutex_unlock(&rund.mutex_rpool);

		/* If we've reached this point without errors and the 'entry' pointer is still valid,
		 * then it either bolongs to apool (completed) or it's in progress.
		 */
		if (entry_has_flag(entry, USCHED_ENTRY_FLAG_FINISH)) {
			/* If the entry is in finishing state and we reached this point, it means
			 * it is now in a completed state.
			 */
			entry_unset_flag(entry, USCHED_ENTRY_FLAG_PROGRESS);
			entry_set_flag(entry, USCHED_ENTRY_FLAG_COMPLETE);

			/* This is a complete entry */
			log_info("notify_read(): Request from file descriptor %d successfully processed.\n", aop->fd);
		} else if (entry_has_flag(entry, USCHED_ENTRY_FLAG_PROGRESS) && !entry_has_flag(entry, USCHED_ENTRY_FLAG_AUTHORIZED)) {
			/* This is another acceptable state. A new entry was received but is still
			 * unauthenticated and is in progress. Authentication is performed during the
			 * update stage of the entry, when relevant authorization data is received
			 * along with the entry payload.
			 *
			 * At this point, we have nothing to do but accept the state and mark this
			 * entry as initialized.
			 */

			entry_set_flag(entry, USCHED_ENTRY_FLAG_INIT);
		} else {
			/* Entry state not recognized. This is an error state. */
			log_warn("notify_read(): Entry state not recognized.\n", strerror(errno));
			goto _read_failure;
		}

		/* Write the aop */
		if (rtsaio_write(aop) < 0) {
			log_warn("notify_read(): rtsaio_write(): %s\n", strerror(errno));
			goto _read_failure;
		}

		return;
	}

_read_failure:
	/* Discard connection */
	log_info("notify_read(): Terminating connection from file descriptor %d\n", aop->fd);

	/* Entries in the remote connections pool (rpool) are always identified by its file descriptor.
	 * Although the entry's handling functions will remove invalid connections from this pool (rpool),
	 * this last search will grant that no residual entries will be kept in the pool in the case of a
	 * connection timeout occur.
	 */
	pthread_mutex_lock(&rund.mutex_rpool);

	if ((entry = rund.rpool->search(rund.rpool, usched_entry_id(aop->fd))))
		rund.rpool->del(rund.rpool, entry);

	pthread_mutex_unlock(&rund.mutex_rpool);

	conn_daemon_client_close(aop->fd);

	if (aop->data)
		mm_free((void *) aop->data);

	/* 'aop' pointer should not be free()'d inside the notification function to avoid issues
	 * with early free()'d while calling rtsaio_cancel().
	 * 
	 * The 'aop' pointer will be inserted into a garbage collector that will prevent the release
	 * of these pointers while the rtsaio_cancel() is being executed.
	 * 
	 */
	if (gc_insert(aop) < 0) {
		/* Ooppss... this will cause a memory leak... but it is preferable to leak memory
		 * than causing the rtsaio_cancel() to access some invalid memory region.
		 */
		log_warn("notify_read(): gc_insert(): %s\n", strerror(errno));
	}
}

void notify_write(struct async_op *aop) {
	struct usched_entry *entry = NULL;
	int cur_fd = aop->fd;

	if ((rtsaio_status(aop) == ASYNCOP_STATUS_COMPLETE) && (rtsaio_count(aop) == aop->count)) {
		/* Free aop->data memory */
		if (aop->data) {
			mm_free((void *) aop->data);
			aop->data = NULL;
		}

		/* Search for an existing entry. */
		pthread_mutex_lock(&rund.mutex_rpool);
		entry = rund.rpool->pope(rund.rpool, usched_entry_id(aop->fd));
		pthread_mutex_unlock(&rund.mutex_rpool);

		if (!entry)
			goto _write_failure;

		/* Prepare the aop for another read */
		memset(aop, 0, sizeof(struct async_op));

		aop->fd = cur_fd;
		aop->priority = 0;
		aop->timeout.tv_sec = rund.config.network.conn_timeout;

		/* Based on entry status, read the necessary amount of data */
		if (entry_has_flag(entry, USCHED_ENTRY_FLAG_COMPLETE)) {
			/* As the entry is complete, we need to destroy it. */
			entry_destroy(entry);

			/* Do another rtsaio_read() of usched_entry_hdr_size() bytes in order
			 * to wait for another entry
			 */
			aop->count = usched_entry_hdr_size();
		} else if (entry_has_flag(entry, USCHED_ENTRY_FLAG_PROGRESS)) {
			/* Re-insert the entry into the rpool before any I/O */
			pthread_mutex_lock(&rund.mutex_rpool);

			if (rund.rpool->insert(rund.rpool, entry) < 0) {
				log_warn("notify_write(): rund.rpool->insert(): %s\n", strerror(errno));
				goto _write_failure;
			}

			pthread_mutex_unlock(&rund.mutex_rpool);

			/* Request the amount of data present on entry->psize (payload size) */
			aop->count = sizeof(entry->session) + entry->psize;
		} else {
			/* Unexpected entry state */
			log_warn("notify_write(): Unexpected entry state.");
			goto _write_failure;
		}

		/* Allocate enough memory on data buffer */
		if (!(aop->data = mm_alloc(aop->count))) {
			log_warn("notify_write(): aop->data = mm_alloc(): %s\n", strerror(errno));

			goto _write_failure;
		}

		/* Reset the memory */
		memset((void *) aop->data, 0, aop->count);

		/* Perform the asynchronous read */
		if (rtsaio_read(aop) < 0) {
			log_warn("notify_write(): rtsaio_read(): %s\n", strerror(errno));

			goto _write_failure;
		}

		return;
	}

_write_failure:
	/* Discard connection */
	log_info("notify_write(): Terminating connection from file descriptor %d\n", aop->fd);

	/* Entries in the remote connections pool (rpool) are always identified by its file descriptor.
	 * Although the entry's handling functions will remove invalid connections from this pool (rpool),
	 * this last search will grant that no residual entries will be kept in the pool in the case of a
	 * connection timeout occur.
	 */
	pthread_mutex_lock(&rund.mutex_rpool);

	if ((entry = rund.rpool->search(rund.rpool, usched_entry_id(aop->fd))))
		rund.rpool->del(rund.rpool, entry);

	pthread_mutex_unlock(&rund.mutex_rpool);

	conn_daemon_client_close(aop->fd);

	if (aop->data)
		mm_free((void *) aop->data);

	/* 'aop' pointer should not be free()'d inside the notification function to avoid issues
	 * with early free()'d while calling rtsaio_cancel().
	 * 
	 * The 'aop' pointer will be inserted into a garbage collector that will prevent the release
	 * of these pointers while the rtsaio_cancel() is being executed.
	 * 
	 */
	if (gc_insert(aop) < 0) {
		/* Ooppss... this will cause a memory leak... but it is preferable to leak memory
		 * than causing the rtsaio_cancel() to access some invalid memory region.
		 */
		log_warn("notify_read(): gc_insert(): %s\n", strerror(errno));
	}
}

