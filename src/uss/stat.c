/**
 * @file stat.c
 * @brief uSched
 *        Status and Statistics Module Main Component
 *
 * Date: 02-07-2015
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
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <pthread.h>

#include <sys/types.h>
#include <sys/wait.h>

#include "config.h"
#include "debug.h"
#include "runtime.h"
#include "log.h"
#include "bitops.h"
#include "stat.h"
#include "ipc.h"

static int _stat_entry_update(
	uint64_t id,
	uid_t uid,
	gid_t gid,
	pid_t pid,
	int status,
	struct timespec *trigger,
	struct timespec *start,
	struct timespec *end,
	size_t outdata_len,
	char *outdata)
{
	int errsv = 0;
	struct usched_stat_entry *s = NULL, *dpool_s = NULL;

	/* Sanity checks */
	if (outdata_len > CONFIG_USCHED_EXEC_OUTPUT_MAX) {
		log_warn("_stat_entry_update(): Output data too long.\n");
		errno = EINVAL;
		return -1;
	}

	/* Acquire spool lock */
	pthread_mutex_lock(&runs.mutex_spool);

	/* Check if the entry already exists */
	if (!(s = runs.spool->search(runs.spool, (struct usched_stat_entry [1]) { { id, } }))) {
		/* If not found, allocate it */
		if (!(s = mm_alloc(sizeof(struct usched_stat_entry)))) {
			errsv = errno;
			log_warn("_stat_entry_update(): mm_alloc(): %s\n", strerror(errno));
			pthread_mutex_unlock(&runs.mutex_spool);
			errno = errsv;
			return -1;
		}

		/* Reset the newly allocated memory */
		memset(s, 0, sizeof(struct usched_stat_entry));

		/* Set the entry id */
		s->id = id;

		/* Insert the newly created entry into the spool */
		if (runs.spool->insert(runs.spool, s) < 0) {
			errsv = errno;
			log_warn("_stat_entry_update(): runs.spool->insert(): %s\n", strerror(errno));
			pthread_mutex_unlock(&runs.mutex_spool);
			errno = errsv;
			return -1;
		}
	}

	/* Update stat entry data */
	s->current.uid = uid;
	s->current.gid = gid;
	s->current.pid = pid;
	s->current.status = WEXITSTATUS(status);
	memcpy(&s->current.trigger, trigger, sizeof(struct timespec));
	memcpy(&s->current.start, start, sizeof(struct timespec));
	memcpy(&s->current.end, end, sizeof(struct timespec));
	s->current.outdata_len = outdata_len;
	memset(s->current.outdata, 0, CONFIG_USCHED_EXEC_OUTPUT_MAX);
	memcpy(s->current.outdata, outdata, outdata_len);

	/* Validate exit status */
	if (WEXITSTATUS(status)) {
		memcpy(&s->error, &s->current, sizeof(struct usched_stat_exec));
		s->nr_fail ++;
	} else {
		s->nr_ok ++;
	}

	/* Update the number of times the entry was executed */
	s->nr_exec ++;

	/* Acquire dpool mutex */
	pthread_mutex_lock(&runs.mutex_dpool);

	/* Duplicate stat entry */
	if (!(dpool_s = stat_dup(s))) {
		errsv = errno;
		log_warn("_stat_entry_update(): stat_dup(): %s\n", strerror(errno));
		pthread_mutex_unlock(&runs.mutex_spool);
		errno = errsv;
		return -1;
	}

	/* Insert stat entry into dpool */
	if (runs.dpool->push(runs.dpool, dpool_s) < 0)
		log_warn("_stat_entry_update(): runs.dpool->push(): %s\n", strerror(errno));

	/* Signal the dpool worker to start processing the queue */
	pthread_cond_signal(&runs.cond_dpool);

	/* Release the dpool mutex */
	pthread_mutex_unlock(&runs.mutex_dpool);

	/* Release spool lock */
	pthread_mutex_unlock(&runs.mutex_spool);

	/* All good */
	return 0;
}

static int _use_incoming(void) {
	int errsv = 0;
	struct ipc_uss_hdr *hdr = NULL;
	char *outdata = NULL, *buf = NULL;

	/* Allocate uss IPC message memory */
	if (!(buf = mm_alloc(runs.config.ipc.msg_size))) {
		errsv = errno;
		log_warn("_use_process(): mm_alloc(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Reset message memory */
	memset(buf, 0, runs.config.ipc.msg_size);

	/* Set header address */
	hdr = (struct ipc_uss_hdr *) buf;

	/* Set output data address */
	outdata = buf + sizeof(struct ipc_uss_hdr);

	/* Wait for IPC message */
	if (ipc_recv(runs.pipcd, (long [1]) { IPC_USE_ID }, (long [1]) { IPC_USS_ID }, buf, runs.config.ipc.msg_size) < 0) {
		errsv = errno;
		log_warn("_use_process(): ipc_recv(): %s\n", strerror(errno));
		mm_free(buf);
		errno = errsv;

		/* Any of the following errno are a fatal condition and this module needs to
		 * be restarted by its monitor.
		 */
		if (errno == EACCES || errno == EFAULT || errno == EINVAL || errno == EIDRM || errno == ENOMEM)
			runtime_stat_fatal();

		errno = errsv;

		return -1;
	}

	/* Validate output data size */
	if ((hdr->outdata_len + sizeof(struct ipc_uss_hdr) + 1) > runs.config.ipc.msg_size) {
		errsv = errno;
		log_crit("_use_process(): IPC message too long (%u bytes). Entry ID: 0x%016llX\n", hdr->outdata_len, hdr->id);
		mm_free(buf);
		errno = errsv;
		return -1;
	}

	/* Grant NULL termination on output data buffer */
	outdata[hdr->outdata_len] = 0;

	debug_printf(DEBUG_INFO, "_use_process(): hdr->id: 0x%016llX, hdr->status: %lu, hdr->pid: %lu, hdr->outdata_len: %lu, outdata: %s\n", hdr->id, hdr->status, hdr->pid, hdr->outdata_len, outdata);

	/* Populate a usched_stat_entry structure and insert/update it on the entry stat pool */
	if (_stat_entry_update(hdr->id, hdr->uid, hdr->gid, hdr->pid, hdr->status, &hdr->t_trigger, &hdr->t_start, &hdr->t_end, hdr->outdata_len, outdata) < 0) {
		errsv = errno;
		log_warn("_use_process(): _stat_entry_update(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Free buffer memory */
	mm_free(buf);

	/* All good */
	return 0;
}

static int _usd_dispatch(void) {
	int errsv = 0;
	char *msg = NULL, *outdata = NULL;
	struct usched_stat_entry *s = NULL;
	struct ipc_usd_hdr *hdr = NULL;

	/* Allocate message size */
	if (!(msg = mm_alloc(runs.config.ipc.msg_size))) {
		errsv = errno;
		log_warn("_usd_dispatch(): mm_alloc(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Reset message memory */
	memset(msg, 0, runs.config.ipc.msg_size);

	/* Setup pointers */
	hdr = (struct ipc_usd_hdr *) msg;
	outdata = msg + sizeof(struct ipc_usd_hdr);

	/* Acquire dispatch pool mutex */
	pthread_mutex_lock(&runs.mutex_dpool);

	/* Fetch a stat entry to be dispatched */
	if (!(s = runs.dpool->pop(runs.dpool))) {
		log_warn("_usd_dispatch(): Dispatch worker was signaled, but queue is empty.\n");
		pthread_mutex_unlock(&runs.mutex_dpool);
		mm_free(msg);
		errno = ENODATA;
		return -1;
	}

	/* Release dispatch pool mutex */
	pthread_mutex_unlock(&runs.mutex_dpool);

	/* Setup header */
	hdr->id = s->id;
	hdr->exec_time = ((s->current.end.tv_sec * 1000000000) + s->current.end.tv_nsec) - ((s->current.start.tv_sec * 1000000000) + s->current.start.tv_nsec);
	hdr->latency = ((s->current.start.tv_sec * 1000000000) + s->current.start.tv_nsec) - ((s->current.trigger.tv_sec * 1000000000) + s->current.trigger.tv_nsec);
	hdr->pid = s->current.pid;
	hdr->status = s->current.status;
	hdr->outdata_len = strlen(s->current.outdata);

	/* Assert outdata length */
	if ((hdr->outdata_len + sizeof(struct ipc_usd_hdr) + 1) > runs.config.ipc.msg_size)
		hdr->outdata_len = (runs.config.ipc.msg_size - sizeof(struct ipc_usd_hdr) - 1);

	/* Set outdata */
	memcpy(outdata, s->current.outdata, hdr->outdata_len);

	/* Destroy dpool stat entry */
	stat_destroy(s);

	/* Print extended debug information */
	debug_printf(DEBUG_INFO, "[DISPATCH]: Entry ID: 0x%016llX, PID: %u, exec_time: %.3fus, latency: %.3fus, status: %u, outdata_len: %u\n", hdr->id, hdr->pid, (hdr->exec_time / 1000.0), (hdr->latency / 1000.0), hdr->status, hdr->outdata_len);

	/* Dispatch message to uSched Daemon */
	if (ipc_send_nowait(runs.pipcd, IPC_USS_ID, IPC_USD_ID, msg, (size_t) runs.config.ipc.msg_size) < 0) {
		errsv = errno;
		log_warn("_usd_dispatch(): ipc_send_nowait(): %s\n", strerror(errno));
		mm_free(msg);
		errno = errsv;

		/* Any of the following errno are a fatal condition and this module needs to
		 * be restarted by its monitor.
		 */
		if (errno == EACCES || errno == EFAULT || errno == EINVAL || errno == EIDRM || errno == ENOMEM)
			runtime_stat_fatal();

		errno = errsv;

		return -1;
	}

	/* Free message memory */
	mm_free(msg);

	/* All good */
	return 0;
}

static void *_worker_incoming(void *arg) {
	/* Process incoming IPC messages */
	for (;;) {
		/* Check if runtime was interrupted */
		if (runtime_stat_interrupted())
			break;

		/* Process messages from uSched Executer */
		if (_use_incoming() < 0)
			log_warn("_worker_incoming(): _use_incoming(): %s\n", strerror(errno));
	}

	debug_printf(DEBUG_INFO, "_worker_incoming(): Terminating...\n");

	/* Runtime was interrupted */
	pthread_exit(NULL);

	return NULL;
}

static void *_worker_dispatch(void *arg) {
	/* Monitor and dispatch rpool entries */
	for (;;) {
		/* Check if runtime was interupted */
		if (runtime_stat_interrupted())
			break;

		pthread_mutex_lock(&runs.mutex_dpool);

		/* Wait for something to be dispatched */
		if (!runs.dpool->count(runs.dpool)) {
			pthread_cond_wait(&runs.cond_dpool, &runs.mutex_dpool);
			pthread_mutex_unlock(&runs.mutex_dpool);
			continue;
		}

		pthread_mutex_unlock(&runs.mutex_dpool);

		/* Process dpool */
		if (_usd_dispatch() < 0)
			log_warn("_worker_dispatch(): _usd_dispatch(): %s\n", strerror(errno));
	}

	debug_printf(DEBUG_INFO, "_worker_dispatch(): Terminating...\n");

	/* Runtime was interrupted */
	pthread_exit(NULL);

	return NULL;
}

static void _init(int argc, char **argv) {
	if (runtime_stat_init(argc, argv) < 0) {
		log_crit("_init(): runtime_stat_init(): %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	/* Create workers */
	if ((errno = pthread_create(&runs.tid_incoming, NULL, &_worker_incoming, NULL))) {
		log_crit("_init(): pthread_create(): %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	if ((errno = pthread_create(&runs.tid_dispatch, NULL, &_worker_dispatch, NULL))) {
		log_crit("_init(): pthread_create(): %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
}

static void _destroy(void) {
	/* Wait for workers to terminate */
	pthread_cancel(runs.tid_incoming);
	pthread_join(runs.tid_incoming, NULL);
	pthread_mutex_lock(&runs.mutex_dpool);
	pthread_cond_signal(&runs.cond_dpool);
	pthread_mutex_unlock(&runs.mutex_dpool);
	pthread_join(runs.tid_dispatch, NULL);

	/* Destroy runtime environment */
	runtime_stat_destroy();
}

static int _loop(int argc, char **argv) {
	int ret = 0;

	_init(argc, argv);

	for (;;) {
		/* Wait for runtime interruption */
		pause();

		debug_printf(DEBUG_INFO, "_loop(): Interrupted...\n");

		/* Check for runtime interruptions */
		if (bit_test(&runs.flags, USCHED_RUNTIME_FLAG_FATAL)) {
			ret = 1; /* This module must be restarted by its monitor */
			break;
		}

		if (bit_test(&runs.flags, USCHED_RUNTIME_FLAG_TERMINATE))
			break;

		if (bit_test(&runs.flags, USCHED_RUNTIME_FLAG_RELOAD)) {
			_destroy();
			_init(argc, argv);
		}

		/* Clear the interrupt flag if we reach this point */
		if (bit_test(&runs.flags, USCHED_RUNTIME_FLAG_INTERRUPT)) {
			pthread_mutex_lock(&runs.mutex_interrupt);
			bit_clear(&runs.flags, USCHED_RUNTIME_FLAG_INTERRUPT);
			pthread_mutex_unlock(&runs.mutex_interrupt);
		}
	}

	_destroy();

	return ret;
}

int main(int argc, char **argv) {
	return _loop(argc, argv);
}

int stat_compare(const void *s1, const void *s2) {
	const struct usched_stat_entry *ps1 = (struct usched_stat_entry *) s1, *ps2 = (struct usched_stat_entry *) s2;

	if (ps1->id > ps2->id)
		return 1;

	if (ps1->id < ps2->id)
		return -1;

	return 0;
}

struct usched_stat_entry *stat_dup(const struct usched_stat_entry *s) {
	int errsv = 0;
	struct usched_stat_entry *d = NULL;

	/* Allocate the entry memory */
	if (!(d = mm_alloc(sizeof(struct usched_stat_entry)))) {
		errsv = errno;
		log_warn("stat_dup(): mm_alloc(): %s\n", strerror(errno));
		errno = errsv;
		return NULL;
	}

	/* Copy stat entry contents */
	memcpy(d, s, sizeof(struct usched_stat_entry));

	/* All good */
	return d;
}

void stat_zero(struct usched_stat_entry *s) {
	memset(s, 0, sizeof(struct usched_stat_entry));
}

void stat_destroy(void *elem) {
	struct usched_stat_entry *s = elem;

	stat_zero(s);

	mm_free(s);
}

