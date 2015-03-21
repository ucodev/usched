/**
 * @file ipc.c
 * @brief uSched
 *        Inter-Process Communication interface - Exec
 *
 * Date: 21-03-2015
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

#include <sys/stat.h>

#include "config.h"
#include "runtime.h"
#include "log.h"
#include "ipc.h"
#if CONFIG_USE_IPC_PMQ == 1
 #include <mqueue.h>
 #include "mm.h"
 #include "pmq.h"
#elif CONFIG_USE_IPC_UNIX == 1 || CONFIG_USE_IPC_INET == 1
 #include <sys/types.h>
 #include <sys/socket.h>
 #include <panet/panet.h>
#endif

int ipc_exec_init(void) {
#if CONFIG_USE_IPC_PMQ == 1
	char *ipc_auth = NULL;
	int errsv = 0;

	/* Allocate IPC authentication buffer */
	if (!(ipc_auth = mm_alloc(rune.config.core.ipc_msgsize + 1))) {
		errsv = errno;
		log_crit("ipc_exec_init(): mm_alloc(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Reset IPC authentication buffer */
	memset(ipc_auth, 0, rune.config.core.ipc_msgsize + 1);

	/* Initialize IPC PMQ interface */
	if (pmq_exec_init() < 0) {
		errsv = errno;
		log_crit("ipc_exec_init(): pmq_exec_init(): %s\n", strerror(errno));
		mm_free(ipc_auth);
		errno = errsv;
		return -1;
	}

	/* Wait for IPC authentication string */
	if (mq_receive(rune.ipcd, ipc_auth, (size_t) rune.config.core.ipc_msgsize, 0) < 0) {
		errsv = errno;
		log_crit("ipc_exec_init(): mq_receive(): %s\n", strerror(errno));
		mm_free(ipc_auth);
		errno = errsv;
		return -1;
	}

	/* Safe to use strcmp(). ipc_auth will always be NULL terminated (granted by memset() + 1) */
	if (strcmp(ipc_auth, rune.config.core.ipc_key)) {
		log_crit("IPC authentication failed.\n");
		mm_free(ipc_auth);
		errno = EINVAL;
		return -1;
	}

	/* Reset IPC authentication buffer (again) */
	memset(ipc_auth, 0, rune.config.core.ipc_msgsize + 1);

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
	/* Create the UNIX socket */
	if ((rune.ipc_bind_fd = panet_server_unix(rune.config.core.ipc_name, PANET_PROTO_UNIX_STREAM, rune.config.core.ipc_msgmax)) == (sock_t) -1) {
		errsv = errno;
		log_warn("ipc_exec_init(): panet_server_unix(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Grant that it's owned by UID 0 and GID 0 */
	if (chown(rune.config.core.ipc_name, 0, 0) < 0) {
		errsv = errno;
		log_warn("ipc_exec_init(): chown(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Grant that it's only accessible by UID 0 */
	if (chmod(rune.config.core.ipc_name, 0600) < 0) {
		errsv = errno;
		log_warn("ipc_exec_init(): chmod(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}
 #endif /* CONFIG_USE_IPC_UNIX */
 #if CONFIG_USE_IPC_INET == 1
	if ((rune.ipc_bind_fd = panet_server_ipv4("127.0.0.1", rune.config.core.ipc_name, PANET_PROTO_TCP, rune.config.core.ipc_msgmax)) == (sock_t) -1) {
		errsv = errno;
		log_warn("ipc_exec_init(): panet_server_ipv4(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}
 #endif /* CONFIG_USE_IPC_INET */

	/* Wait for the client connection */
	if ((rune.ipcd = (sock_t) accept(rune.ipc_bind_fd, NULL, NULL)) == (sock_t) -1) {
		errsv = errno;
		log_warn("ipc_exec_init(): accept(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* TODO: For INET connections, a privileged port (either source and dest ports) must be used.
	 *       Any connection using unprivileged ports (>= 1024) shall be discarded.
	 */

	/* Close bound fd */
	panet_safe_close(rune.ipc_bind_fd);
	rune.ipc_bind_fd = (sock_t) -1;

	/* Wait for authentication string */
	if (panet_read(rune.ipcd, ipc_auth, (size_t) sizeof(ipc_auth) - 1) != (ssize_t) sizeof(ipc_auth) - 1) {
		errsv = errno;
		log_warn("panet_read(): Unable to read IPC authentication string: (%s)\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Safe to use strcmp(). ipc_auth will always be NULL terminated (granted by memset() + 1) */
	if (strcmp(ipc_auth, rune.config.core.ipc_key)) {
		log_crit("IPC authentication failed.\n");
		errno = EINVAL;
		return -1;
	}

	/* All good */
	return 0;
#else
	errno = ENOSYS;
	return -1;
#endif
}

void ipc_exec_destroy(void) {
#if CONFIG_USE_IPC_PMQ == 1
	pmq_exec_destroy();
#elif CONFIG_USE_IPC_UNIX == 1 || CONFIG_USE_IPC_INET == 1
	panet_safe_close(rune.ipcd);
	panet_safe_close(rune.ipc_bind_fd);
#else
	return;
#endif
}

