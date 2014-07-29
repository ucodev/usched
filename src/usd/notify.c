/**
 * @file notify.c
 * @brief uSched
 *        I/O Notification interface
 *
 * Date: 30-07-2014
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

void notify_read(struct async_op *aop) {
	struct usched_entry *entry = NULL;

	if ((rtsaio_status(aop) == ASYNCOP_STATUS_COMPLETE) && (rtsaio_count(aop) == aop->count)) {
		/* Successfully received possible valid data */

		/* Search for an existing entry. If found, the received data belongs to the entry command */
		pthread_mutex_lock(&rund.mutex_rpool);
		entry = rund.rpool->pope(rund.rpool, usched_entry_id(aop->fd));
		pthread_mutex_unlock(&rund.mutex_rpool);

		if (entry) {
			if (process_daemon_recv_update(aop, entry) < 0)
				goto _read_failure;
		} else if (!(entry = process_daemon_recv_create(aop))) {
			goto _read_failure;
		}

		/* If we've reached this point without errors, the 'entry' pointer is still valid
		 * and now belongs to the rund.apool list.
		 */
		if (entry_has_flag(entry, USCHED_ENTRY_FLAG_COMPLETE)) {
			/* Destroy this entry */
			entry_destroy(entry);

			/* This is a complete entry */
			log_info("notify_read(): Request from file descriptor %d successfully processed.\n", aop->fd);
		} else if (entry_has_flag(entry, USCHED_ENTRY_FLAG_INIT) && !entry_has_flag(entry, USCHED_ENTRY_FLAG_AUTHORIZED)) {
			/* This is a newly created entry */
			pthread_mutex_lock(&rund.mutex_rpool);

			if (rund.rpool->insert(rund.rpool, entry) < 0) {
				log_warn("notify_read(): rund.rpool->insert(): %s\n", strerror(errno));
				pthread_mutex_unlock(&rund.mutex_rpool);
				goto _read_failure;
			}

			pthread_mutex_unlock(&rund.mutex_rpool);
		} else {
			/* Entry state not recognized. This is an error state. */
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

	panet_safe_close(aop->fd);

	if (aop->data)
		mm_free((void *) aop->data);

	mm_free(aop);
}

void notify_write(struct async_op *aop) {
	struct usched_entry *entry = NULL;
	int cur_fd = aop->fd;

	if ((rtsaio_status(aop) == ASYNCOP_STATUS_COMPLETE) && (rtsaio_count(aop) == aop->count)) {
		if (aop->data)
			mm_free((void *) aop->data);

		/* Do another rtsaio_read() of usched_entry_hdr_size() bytes in order to wait for another entry */
		memset(aop, 0, sizeof(struct async_op));

		aop->fd = cur_fd;
		aop->count = usched_entry_hdr_size();
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

	panet_safe_close(aop->fd);

	if (aop->data)
		mm_free((void *) aop->data);

	mm_free(aop);
}

