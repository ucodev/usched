/**
 * @file ipc.c
 * @brief uSched
 *        Inter-Process Communication interface - Exec
 *
 * Date: 13-05-2015
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

int ipc_exec_init(void) {
	int errsv = 0;

	/* Create key */
	rune.pipck = (pipck_t) rune.config.ipc.id_key;

	/* Create IPC descriptor */
	if (!(rune.pipcd = pipc_slave_register(rune.pipck, IPC_USE_ID, rune.config.ipc.msg_max, rune.config.ipc.msg_size, 0600))) {
		errsv = errno;
		log_warn("ipc_exec_init(): pipc_slave_register(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* All good */
	return 0;
}

void ipc_exec_destroy(void) {
	if (pipc_slave_unregister(rune.pipcd) < 0)
		log_warn("ipc_exec_destroy(): pipc_slave_unregister(): %s\n", strerror(errno));
}

