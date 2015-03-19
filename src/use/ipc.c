/**
 * @file ipc.c
 * @brief uSched
 *        Inter-Process Communication interface - Exec
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
#include "runtime.h"
#include "log.h"
#include "ipc.h"
#if CONFIG_USE_IPC_PMQ == 1
 #include "pmq.h"
#elif CONFIG_USE_IPC_SOCK == 1
 #include <panet/panet.h>
#endif

int ipc_exec_init(void) {
#if CONFIG_USE_IPC_PMQ == 1
	int errsv = 0;

	if (pmq_exec_init() < 0) {
		errsv = errno;
		log_crit("ipc_exec_init(): pmq_exec_init(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	return 0;
#elif CONFIG_USE_IPC_SOCK == 1
	int errsv = 0;

	if ((rune.ipcd = panet_server_unix(rune.config.core.ipc_name, PANET_PROTO_UNIX_STREAM, rune.config.core.ipc_msgmax)) < 0) {
		errsv = errno;
		log_warn("ipc_exec_init(): panet_server_unix(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	if (chown(rune.config.core.ipc_name, 0, 0) < 0) {
		errsv = errno;
		log_warn("ipc_exec_init(): chown(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	if (chmod(rune.config.core.ipc_name, 0600) < 0) {
		errsv = errno;
		log_warn("ipc_exec_init(): chmod(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	return 0;
#else
	errno = ENOSYS;
	return -1;
#endif
}

void ipc_exec_destroy(void) {
#if CONFIG_USE_IPC_PMQ == 1
	pmq_exec_destroy();
#elif CONFIG_USE_IPC_SOCK == 1
	panet_safe_close(rune.ipcd);
#else
	return;
#endif
}

