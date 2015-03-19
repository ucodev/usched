/**
 * @file ipc.c
 * @brief uSched
 *        Inter-Process Communication interface - Daemon
 *
 * Date: 19-03-2015
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

#include <string.h>
#include <errno.h>

#include "config.h"
#include "log.h"
#include "ipc.h"
#if CONFIG_USE_IPC_PMQ == 1
 #include "pmq.h"
#endif

int ipc_daemon_init(void) {
#if CONFIG_USE_IPC_PMQ == 1
	int errsv = 0;

	if (pmq_daemon_init() < 0) {
		errsv = errno;
		log_warn("ipc_daemon_init(): pmq_daemon_init(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	return 0;
#else
	errno = ENOSYS;
	return -1;
#endif
}

void ipc_daemon_destroy(void) {
#if CONFIG_USE_IPC_PMQ == 1
	pmq_daemon_destroy();
#else
	return;
#endif
}

