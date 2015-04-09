/**
 * @file notify.c
 * @brief uSched
 *        I/O Notification interface
 *
 * Date: 09-04-2015
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
#include <errno.h>
#include <pthread.h>
#include <time.h>

#include <rtsaio/rtsaio.h>

#include "config.h"
#include "debug.h"
#include "bitops.h"
#include "runtime.h"
#include "mm.h"
#include "notify.h"
#include "log.h"
#include "entry.h"
#include "conn.h"
#include "process.h"
#include "gc.h"
#include "marshal.h"

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

			/* NOTE: No need to re-insert the entry into the remote connections pool.
			 * If process_daemon_recv_update() was successful, then the entry was
			 * inserted into the active pool and will now become completed (its current
			 * state is PROGRESS).
			 */
		} else {
			if (!(entry = process_daemon_recv_create(aop))) {
				log_warn("notify_read(): process_daemon_recv_create(): %s\n", strerror(errno));
				goto _read_failure;
			}

			/* Re-insert the entry into the rpool before any I/O */
			pthread_mutex_lock(&rund.mutex_rpool);

			if (rund.rpool->insert(rund.rpool, entry) < 0) {
				pthread_mutex_unlock(&rund.mutex_rpool);
				log_warn("notify_read(): rund.rpool->insert(): %s\n", strerror(errno));
				goto _read_failure;
			}

			pthread_mutex_unlock(&rund.mutex_rpool);
		}

		/* If we've reached this point without errors and the 'entry' pointer is still valid,
		 * then it either bolongs to apool (completed) or it's in progress.
		 */
		if (entry_has_flag(entry, USCHED_ENTRY_FLAG_FINISH)) {
			/* If the entry is in finishing state and we reached this point, it means
			 * it is now in a completed state.
			 */
			entry_unset_flag(entry, USCHED_ENTRY_FLAG_PROGRESS);
			entry_set_flag(entry, USCHED_ENTRY_FLAG_COMPLETE);

			/* Cleanup all cryptographic data */
			entry_cleanup_crypto(entry);

			/* Cleanup all session data */
			entry_cleanup_session(entry);

			/* Set the entry creation time */
			entry->create_time = time(NULL);

			/* This is a complete entry */
			log_info("notify_read(): Request from file descriptor %d successfully processed.\n", aop->fd);

#if CONFIG_USCHED_SERIALIZE_ON_REQ == 1
			if (bit_test(&rund.flags, USCHED_RUNTIME_FLAG_SERIALIZE)) {
				pthread_mutex_lock(&rund.mutex_marshal);

				/* Signal the marshal monitor to start serializing data */
				pthread_cond_signal(&rund.cond_marshal);

				pthread_mutex_unlock(&rund.mutex_marshal);
			}
#endif

			/* If this is a GET or DEL request, we must destroy the entry after
			 * completion since it wasn't placed in the active pool.
			 */
			if (entry_has_flag(entry, USCHED_ENTRY_FLAG_GET) || entry_has_flag(entry, USCHED_ENTRY_FLAG_DEL))
				entry_destroy(entry);
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

	/* If we find an entry by it's file descriptor, it's safe to delete it because it doesn't
	 * reside in the active pool. The ID of the entries on the active pool are no longer the
	 * file descriptor value, but a unique identifier.
	 */
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

		/* To prevent further leaks, if any, we'll force the daemon to be restarted by
		 * uSched monitor (usm) by setting the FATAL runtime flag.
		 */
		runtime_daemon_fatal();
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

		/* Prepare the aop for another read */
		memset(aop, 0, sizeof(struct async_op));

		aop->fd = cur_fd;
		aop->priority = 0;
		aop->timeout.tv_sec = rund.config.network.conn_timeout;

		/* Based on entry status, read the necessary amount of data */
		if (!entry) {
			/* If we've performed a rtsaio_write() and we can't find the entry in the
			 * remote connections pool, then the entry was already completed and resides
			 * only on the active pool. This also means that we now should perform
			 * another read on the file descriptor in order to wait for more entries
			 * from the client.
			 */

			/* Do another rtsaio_read() of usched_entry_hdr_size() bytes in order
			 * to wait for another entry
			 */
			aop->count = usched_entry_hdr_size();

			debug_printf(DEBUG_INFO, "Performing another entry read (aop->count: %u)...\n", aop->count);
		} else if (entry_has_flag(entry, USCHED_ENTRY_FLAG_PROGRESS)) {
			/* Re-insert the entry into the rpool before any I/O */
			pthread_mutex_lock(&rund.mutex_rpool);

			if (rund.rpool->insert(rund.rpool, entry) < 0) {
				pthread_mutex_unlock(&rund.mutex_rpool);
				log_warn("notify_write(): rund.rpool->insert(): %s\n", strerror(errno));
				goto _write_failure;
			}

			pthread_mutex_unlock(&rund.mutex_rpool);

			/* Request the amount of data present on entry->psize (payload size) */
			aop->count = sizeof(entry->session) + entry->psize;
		} else {
			/* Unexpected entry state */
			log_warn("notify_write(): Unexpected entry state.\n");
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

	/* If we find an entry by it's file descriptor, it's safe to delete it because it doesn't
	 * reside in the active pool. The ID of the entries on the active pool are no longer the
	 * file descriptor value, but a unique identifier.
	 */
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

		/* To prevent further leaks, if any, we'll force the daemon to be restarted by
		 * uSched monitor (usm) by setting the FATAL runtime flag.
		 */
		runtime_daemon_fatal();
	}
}

