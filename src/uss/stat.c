/**
 * @file stat.c
 * @brief uSched
 *        Status and Statistics Module Main Component
 *
 * Date: 08-04-2015
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
	struct usched_stat_entry *s = NULL;

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

	/* Release spool lock */
	pthread_mutex_unlock(&runs.mutex_spool);

	/* All good */
	return 0;
}

static int _use_process(void) {
	int errsv = 0;
	struct ipc_uss_hdr *hdr = NULL;
	char *outdata = NULL, *buf = NULL;

	/* Allocate uss IPC message memory */
	if (!(buf = mm_alloc(runs.config.exec.ipc_msgsize))) {
		errsv = errno;
		log_warn("_use_process(): mm_alloc(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Reset message memory */
	memset(buf, 0, runs.config.exec.ipc_msgsize);

	/* Set header address */
	hdr = (struct ipc_uss_hdr *) buf;

	/* Set output data address */
	outdata = buf + sizeof(struct ipc_uss_hdr);

	/* Wait for IPC message */
	if (ipc_recv(runs.ipcd_use_ro, buf, runs.config.exec.ipc_msgsize) < 0) {
		errsv = errno;
		log_warn("_use_process(): ipc_recv(): %s\n", strerror(errno));
		mm_free(buf);
		errno = errsv;
		return -1;
	}

	/* Validate output data size */
	if (hdr->outdata_len >= (runs.config.exec.ipc_msgsize - sizeof(struct ipc_uss_hdr))) {
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
	return -1;
}

static int _stat_process(void) {
	/* Process IPC messages */
	for (;;) {
		/* Check if runtime was interrupted */
		if (runtime_stat_interrupted())
			break;

		/* Process data incoming from use */
		if (_use_process() < 0) {
			log_warn("_stat_process(): _use_process(): %s\n", strerror(errno));

			continue;
		}

		/* TODO */
		if (_usd_dispatch() < 0) {
			log_warn("_stat_process(): _usd_dispatch(): %s\n", strerror(errno));

			continue;
		}
	}

	/* No errors... just an interrupt */
	return 0;
}

int stat_compare(const void *s1, const void *s2) {
	const struct usched_stat_entry *ps1 = (struct usched_stat_entry *) s1, *ps2 = (struct usched_stat_entry *) s2;

	if (ps1->id > ps2->id)
		return 1;

	if (ps1->id < ps2->id)
		return -1;

	return 0;
}

void stat_zero(struct usched_stat_entry *s) {
	memset(s, 0, sizeof(struct usched_stat_entry));
}

void stat_destroy(void *elem) {
	struct usched_stat_entry *s = elem;

	stat_zero(s);

	mm_free(s);
}

static void _init(int argc, char **argv) {
	if (runtime_stat_init(argc, argv) < 0) {
		log_crit("_init(): runtime_stat_init(): %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
}

static void _destroy(void) {
	runtime_stat_destroy();
}

static void _loop(int argc, char **argv) {
	_init(argc, argv);

	for (;;) {
		/* Process status and statistics requests */
		if (_stat_process() < 0) {
			log_crit("_loop(): _stat_process(): %s\n", strerror(errno));
			break;
		}

		/* Check for runtime interruptions */
		if (bit_test(&runs.flags, USCHED_RUNTIME_FLAG_TERMINATE))
			break;

		if (bit_test(&runs.flags, USCHED_RUNTIME_FLAG_RELOAD)) {
			_destroy();
			_init(argc, argv);
		}
	}

	_destroy();
}

int main(int argc, char **argv) {
	_loop(argc, argv);

	return 0;
}

