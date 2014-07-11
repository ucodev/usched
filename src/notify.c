/**
 * @file notify.c
 * @brief uSched
 *        I/O Notification interface
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
		entry = rund.rpool->search(rund.rpool, usched_entry_id(aop->fd));
		pthread_mutex_unlock(&rund.mutex_rpool);

		if (entry) {
			if (process_recv_update(aop, entry) < 0)
				goto _read_failure;
		} else if (!(entry = process_recv_create(aop))) {
			goto _read_failure;
		}

		return;
	}

_read_failure:
	/* Discard connection */
	log_info("notify_read(): Discarding connection from file descriptor %d\n", aop->fd);

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
	log_info("notify_write(): Discarding connection from file descriptor %d\n", aop->fd);

	panet_safe_close(aop->fd);

	if (aop->data)
		mm_free((void *) aop->data);

	mm_free(aop);
}

