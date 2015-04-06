/**
 * @file stat.c
 * @brief uSched
 *        Status and Statistics Module Main Component
 *
 * Date: 06-04-2015
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

#include <stdlib.h>
#include <unistd.h>

#include "config.h"
#include "debug.h"
#include "runtime.h"
#include "log.h"
#include "bitops.h"
#include "ipc.h"

static int _use_process(char *msg) {
	int errsv = 0;
	struct ipc_uss_hdr *hdr = (struct ipc_uss_hdr *) msg;
	char *outdata = msg + sizeof(struct ipc_uss_hdr);

	/* Sanity checks */
	if (!msg) {
		log_crit("_use_process(): Message buffer is NULL.\n");
		/* TODO: Set runtime fatal flag here as this is really critical */
		errno = EINVAL;
		return -1;
	}

	/* Wait for IPC message */
	if (ipc_recv(runs.ipcd_use_ro, msg, runs.config.exec.ipc_msgsize) < 0) {
		errsv = errno;
		log_warn("_use_process(): ipc_recv(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Validate output data size */
	if (hdr->outdata_len >= (runs.config.exec.ipc_msgsize - sizeof(struct ipc_uss_hdr))) {
		errsv = errno;
		log_crit("_use_process(): IPC message too long (%u bytes). Entry ID: 0x%016llX\n", hdr->outdata_len, hdr->id);
		errno = errsv;
		return -1;
	}

	/* Grant NULL termination on output data buffer */
	outdata[hdr->outdata_len] = 0;

	debug_printf(DEBUG_INFO, "_use_process(): hdr->id: 0x%016llX, hdr->status: %lu, hdr->pid: %lu, hdr->outdata_len: %lu, outdata: %s\n", hdr->id, hdr->status, hdr->pid, hdr->outdata_len, outdata);

	/* TODO */
	return 0;
}

static int _usd_dispatch(void) {
	return -1;
}

static int _stat_process(void) {
	int errsv = 0;
	char *buf = NULL, *outdata = NULL;
	struct ipc_uss_hdr *hdr = NULL;

	/* Allocate uss IPC message memory */
	if (!(buf = mm_alloc(runs.config.exec.ipc_msgsize))) {
		errsv = errno;
		log_warn("_stat_process(): mm_alloc(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Set header address */
	hdr = (struct ipc_uss_hdr *) buf;

	/* Set output data address */
	outdata = buf + sizeof(struct ipc_uss_hdr);

	/* Process IPC messages */
	for (;;) {
		/* Check if runtime was interrupted */
		if (runtime_stat_interrupted())
			break;

		/* Reset message memory */
		memset(buf, 0, runs.config.exec.ipc_msgsize);

		/* Process data incoming from use */
		if (_use_process(buf) < 0) {
			log_warn("_stat_process(): _use_process(): %s\n", strerror(errno));

			continue;
		}

		/* TODO */
		_usd_dispatch();
	}

	/* Release buffer memory */
	mm_free(buf);

	/* No errors... just an interrupt */
	return 0;
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

