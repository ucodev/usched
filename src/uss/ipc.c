/**
 * @file ipc.c
 * @brief uSched
 *        Inter-Process Communication interface - Stat
 *
 * Date: 12-05-2015
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
#include <unistd.h>

#include <pipc/pipc.h>

#include "config.h"
#include "debug.h"
#include "runtime.h"
#include "log.h"
#include "ipc.h"

int ipc_stat_init(void) {
	int errsv = 0;

	/* TODO: Create key */

	/* Create IPC descriptor */
	if (!(runs.pipcd = pipc_slave_register(runs.pipck, IPC_USS_ID, runs.config.ipc.msgmax, runs.config.ipc.msgsize, 0600)))
		errsv = errno;
		log_warn("ipc_stat_init(): pipc_slave_register(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* All good */
	return 0;
}

void ipc_stat_destroy(void) {
	if (pipc_slave_unregister(runs.pipcd) < 0)
		log_warn("ipc_stat_destroy(): pipc_slave_unregister(): %s\n", strerror(errno));
}

