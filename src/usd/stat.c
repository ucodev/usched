/**
 * @file stat.c
 * @brief uSched
 *        Status and Statistics worker interface - Daemon
 *
 * Date: 15-04-2015
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

#include "config.h"
#include "debug.h"
#include "bitops.h"
#include "runtime.h"
#include "mm.h"
#include "log.h"
#include "ipc.h"
#include "stat.h"
#include "entry.h"

static int _stat_daemon_process(char *msg) {
	int errsv = 0;
	struct ipc_usd_hdr *hdr = (struct ipc_usd_hdr *) msg;
	char *outdata = msg + sizeof(struct ipc_usd_hdr);
	struct usched_entry *entry = NULL;

	/* Validate outdata length */
	if ((hdr->outdata_len + sizeof(struct ipc_usd_hdr)) >= rund.config.stat.ipc_msgsize) {
		errsv = errno = EINVAL;
		log_warn("_stat_daemon_process(): hdr->outdata_len is too long: %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Grant that outdata is NULL terminated */
	outdata[hdr->outdata_len] = 0;

	/* Print debug information */
	debug_printf(DEBUG_INFO, "[STAT RECEIVED]: Entry ID: 0x%016llX, PID: %u, exec_time: %.3fus, latency: %.3fus, status: %u, outdata_len: %u\n", hdr->id, hdr->pid, (hdr->exec_time / 1000.0), (hdr->latency / 1000.0), hdr->status, hdr->outdata_len);

	/* Acquire active pool mutex */
	pthread_mutex_lock(&rund.mutex_apool);

	/* Search for an existing entry on active pool that matches the received ID */
	if (!(entry = rund.apool->search(rund.apool, (struct usched_entry [1]) { { hdr->id, } }))) {
		pthread_mutex_unlock(&rund.mutex_apool);
		errsv = errno = EINVAL;
		log_warn("_stat_daemon_process(): Cannot find Entry ID 0x%016llX on active pool: %s\n", hdr->id, strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Grant that outdata length doesn't exceed entry outdata buffer size */
	if (hdr->outdata_len >= sizeof(entry->outdata))
		hdr->outdata_len = sizeof(entry->outdata) - 1;

	/* Update entry status and statistical data */
	entry->pid = hdr->pid;
	entry->status = hdr->status;
	entry->exec_time = hdr->exec_time;
	entry->latency = hdr->latency;
	entry->outdata_len = hdr->outdata_len;
	memcpy(entry->outdata, outdata, hdr->outdata_len);
	entry->outdata[hdr->outdata_len] = 0;

	/* Release active pool mutex */
	pthread_mutex_unlock(&rund.mutex_apool);

	debug_printf(DEBUG_INFO, "_stat_daemon_process(): Entry ID 0x%016llX updated.\n", hdr->id);

	/* All good */
	return 0;
}

static void *_stat_daemon_worker(void *arg) {
	char *msg = NULL;
	arg = NULL;

	for (;;) {
		/* Check for runtime interruptions */
		if (runtime_daemon_interrupted()) {
			if (!ipc_pending(rund.ipcd_uss_ro))
				break;
		}

		/* Allocate message size */
		if (!(msg = mm_alloc((size_t) rund.config.stat.ipc_msgsize + 1))) {
			log_warn("_stat_daemon_worker(): msg = mm_alloc(): %s\n", strerror(errno));
			continue;
		}

		/* Reset message memory */
		memset(msg, 0, (size_t) rund.config.stat.ipc_msgsize + 1);

		/* Wait for IPC message */
		if (ipc_recv(rund.ipcd_uss_ro, msg, (size_t) rund.config.stat.ipc_msgsize) < 0) {
			log_warn("_stat_daemon_worker(): ipc_recv(): %s\n", strerror(errno));
			mm_free(msg);
			continue;
		}

		/* Process incoming message */
		if (_stat_daemon_process(msg) < 0) {
			log_warn("_stat_daemon_worker(): _stat_daemon_process(): %s\n", strerror(errno));
			mm_free(msg);
			continue;
		}

		/* Free msg memory */
		mm_free(msg);
	}

	/* All good */
	pthread_exit(NULL);

	return NULL;
}

int stat_daemon_init(void) {
	int errsv = 0;

	/* Initialize stat worker */
	if ((errno = pthread_create(&rund.t_stat, NULL, &_stat_daemon_worker, NULL))) {
		errsv = errno;
		log_warn("stat_daemon_process_init(): pthread_create(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* All good */
	return 0;
}

void stat_daemon_destroy(void) {
	/* FIXME: Thread may contain alloc'd data and it's now being handled on cancel cleanups */
	pthread_cancel(rund.t_stat);
	pthread_join(rund.t_stat, NULL);
}

