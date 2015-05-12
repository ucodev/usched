/**
 * @file ipc.c
 * @brief uSched
 *        IPC Controller Main Component
 *
 * Date: 29-04-2015
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
#include <errno.h>

#include <pipc/pipc.h>

#include "config.h"
#include "debug.h"
#include "runtime.h"
#include "log.h"
#include "bitops.h"
#include "ipc.h"


static void _init(int argc, char **argv) {
	if (runtime_ipc_init(argc, argv) < 0) {
		log_crit("_init(): runtime_stat_init(): %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	/* TODO */
}

static void _destroy(void) {
	/* Destroy runtime environment */
	runtime_ipc_destroy();
}

static void _loop(int argc, char **argv) {
	_init(argc, argv);

	for (;;) {
		/* Wait for runtime interruption */
		pause();

		debug_printf(DEBUG_INFO, "_loop(): Interrupted...\n");

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

int ipc_ipc_init(void) {
	int errsv = 0;

	/* TODO: Create key */

	/* Create IPC descriptor */
	if (!(runi.pipcd = pipc_master_register(runi.pipck, IPC_USI_ID, runi.config.ipc.msgmax, runi.config.ipc.msgsize, 0600)))
		errsv = errno;
		log_warn("ipc_ipc_init(): pipc_master_register(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* All good */
	return 0;
}

void ipc_ipc_destroy(void) {
	if (pipc_master_unregister(runi.pipcd) < 0)
		log_warn("ipc_ipc_destroy(): pipc_master_unregister(): %s\n", strerror(errno));
}

