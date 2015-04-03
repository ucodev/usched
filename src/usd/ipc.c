/**
 * @file ipc.c
 * @brief uSched
 *        Inter-Process Communication interface - Daemon
 *
 * Date: 26-03-2015
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
 #include <mqueue.h>
 #include "pmq.h"
#elif CONFIG_USE_IPC_UNIX == 1 || CONFIG_USE_IPC_INET == 1
 #include <panet/panet.h>
#endif

int ipc_daemon_init(void) {
#if CONFIG_USE_IPC_PMQ == 1
	char *ipc_auth = NULL;
	int errsv = 0;

	if (!(ipc_auth = mm_alloc(rund.config.core.ipc_msgsize + 1))) {
		errsv = errno;
		log_warn("ipc_daemon_init(): mm_alloc(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Reset IPC authentication buffer */
	memset(ipc_auth, 0, rund.config.core.ipc_msgsize + 1);

	/* Initialize IPC PMQ interface */
	if (pmq_daemon_init() < 0) {
		errsv = errno;
		log_warn("ipc_daemon_init(): pmq_daemon_init(): %s\n", strerror(errno));
		mm_free(ipc_auth);
		errno = errsv;
		return -1;
	}

	/* Craft IPC authentication string */
	strncpy(ipc_auth, rund.config.core.ipc_key, rund.config.core.ipc_msgsize);

	/* Send IPC authentication string */
	if (mq_send(rund.ipcd, ipc_auth, (size_t) rund.config.core.ipc_msgsize, 0) < 0) {
		errsv = errno;
		log_crit("ipc_daemon_init(): mq_send(): %s\n", strerror(errno));
		mm_free(ipc_auth);
		errno = errsv;
		return -1;
	}

	/* Reset IPC authentication buffer (again) */
	memset(ipc_auth, 0, rund.config.core.ipc_msgsize + 1);

	/* Free IPC authentication buffer */
	mm_free(ipc_auth);

	/* All good */
	return 0;
#elif CONFIG_USE_IPC_UNIX == 1 || CONFIG_USE_IPC_INET == 1
	char ipc_auth[CONFIG_USCHED_AUTH_IPC_SIZE + 1];
	int errsv = 0;

	/* Reset IPC authentication buffer */
	memset(ipc_auth, 0, sizeof(ipc_auth));

 #if CONFIG_USE_IPC_UNIX == 1
	if ((rund.ipcd = panet_client_unix(rund.config.core.ipc_name, PANET_PROTO_UNIX_STREAM)) < 0) {
		errsv = errno;
		log_warn("ipc_daemon_init(): panet_client_unix(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}
 #elif CONFIG_USE_IPC_INET == 1
	if ((rund.ipcd = panet_client_ipv4("127.0.0.1", rund.config.core.ipc_name, PANET_PROTO_TCP, 5)) < 0) {
		errsv = errno;
		log_warn("ipc_daemon_init(): panet_client_ipv4(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}
 #endif

	/* Craft IPC authentication string */
	strncpy(ipc_auth, rund.config.core.ipc_key, CONFIG_USCHED_AUTH_IPC_SIZE);

	if (panet_write(rund.ipcd, ipc_auth, (size_t) sizeof(ipc_auth) - 1) != (ssize_t) sizeof(ipc_auth) - 1) {
		errsv = errno;
		log_crit("ipc_daemon_init(): panet_write(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* All good */
	return 0;
#else
	errno = ENOSYS;
	return -1;
#endif
}

void ipc_daemon_destroy(void) {
#if CONFIG_USE_IPC_PMQ == 1
	pmq_daemon_destroy();
#elif CONFIG_USE_IPC_UNIX == 1 || CONFIG_USE_IPC_INET == 1
	panet_safe_close(rund.ipcd);
#else
	return;
#endif
}

