/**
 * @file ipc.c
 * @brief uSched
 *        Inter-Process Communication interface - Stat
 *
 * Date: 16-04-2015
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
#include "debug.h"
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

static int _ipc_init_use_ro(void) {
#if CONFIG_USE_IPC_PMQ == 1
	char *ipc_auth = NULL;
	int errsv = 0;

	/* Allocate IPC authentication buffer */
	if (!(ipc_auth = mm_alloc(runs.config.exec.ipc_msgsize + 1))) {
		errsv = errno;
		log_crit("_ipc_init_use_ro(): mm_alloc(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Reset IPC authentication buffer */
	memset(ipc_auth, 0, runs.config.exec.ipc_msgsize + 1);

	/* Initialize IPC PMQ interface */
	if (pmq_stat_use_init() < 0) {
		errsv = errno;
		log_crit("_ipc_init_use_ro(): pmq_stat_use_init(): %s\n", strerror(errno));
		mm_free(ipc_auth);
		errno = errsv;
		return -1;
	}

	debug_printf(DEBUG_INFO, "[IPC]: Wait for uSched Exec (use) authentication...\n");

	/* Wait for IPC authentication string from uSched Exec (use) */
	if (mq_receive(runs.ipcd_use_ro, ipc_auth, (size_t) runs.config.exec.ipc_msgsize, 0) < 0) {
		errsv = errno;
		log_crit("_ipc_init_use_ro(): mq_receive(): %s\n", strerror(errno));
		mm_free(ipc_auth);
		errno = errsv;
		return -1;
	}

	debug_printf(DEBUG_INFO, "[IPC]: Authentication received.\n");

	/* Safe to use strcmp(). ipc_auth will always be NULL terminated (granted by memset() + 1) */
	if (strcmp(ipc_auth, runs.config.exec.ipc_key)) {
		log_crit("_ipc_init_use_ro(): IPC authentication failed.\n");
		mm_free(ipc_auth);
		errno = EINVAL;
		return -1;
	}

	debug_printf(DEBUG_INFO, "_ipc_init_use_ro(): IPC authentication successful.\n");

	/* Reset IPC authentication buffer (again) */
	memset(ipc_auth, 0, runs.config.exec.ipc_msgsize + 1);

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
	if ((runs.ipc_bind_fd = panet_server_unix(runs.config.exec.ipc_name, PANET_PROTO_UNIX_STREAM, runs.config.exec.ipc_msgmax)) == (sock_t) -1) {
		errsv = errno;
		log_warn("_ipc_init_use_ro(): panet_server_unix(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Grant that it's owned by UID 0 and GID 0 */
	if (chown(runs.config.exec.ipc_name, 0, 0) < 0) {
		errsv = errno;
		log_warn("_ipc_init_use_ro(): chown(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Grant that it's only accessible by UID 0 */
	if (chmod(runs.config.exec.ipc_name, 0600) < 0) {
		errsv = errno;
		log_warn("_ipc_init_use_ro(): chmod(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Wait for the client connection */
	if ((runs.ipcd_use_ro = (sock_t) accept(runs.ipc_bind_fd, (struct sockaddr *) (struct sockaddr_un [1]) { { 0 } }, (socklen_t [1]) { sizeof(struct sockaddr_un) })) == (sock_t) -1) {
		errsv = errno;
		log_warn("_ipc_init_use_ro(): accept(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

 #elif CONFIG_USE_IPC_INET == 1
	if ((runs.ipc_bind_fd = panet_server_ipv4(CONFIG_USE_IPC_INET_BINDADDR, runs.config.exec.ipc_name, PANET_PROTO_TCP, runs.config.exec.ipc_msgmax)) == (sock_t) -1) {
		errsv = errno;
		log_warn("_ipc_init_use_ro(): panet_server_ipv4(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Wait for the client connection */
	if ((runs.ipcd_use_ro = (sock_t) accept(runs.ipc_bind_fd, (struct sockaddr *) (struct sockaddr_in [1]) { { 0 } }, (socklen_t [1]) { sizeof(struct sockaddr_in) })) == (sock_t) -1) {
		errsv = errno;
		log_warn("_ipc_init_use_ro(): accept(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}
 #else
  #error "No IPC mechanism defined."
 #endif

	/* TODO: For INET connections, a privileged port (either source and dest ports) must be used.
	 *       Any connection using unprivileged ports (>= 1024) shall be discarded.
	 */

	/* Close bound fd */
	panet_safe_close(runs.ipc_bind_fd);
	runs.ipc_bind_fd = (sock_t) -1;

	debug_printf(DEBUG_INFO, "[IPC]: Wait for uSched Exec (use) authentication...\n");

	/* Wait for authentication string */
	if (panet_read(runs.ipcd_use_ro, ipc_auth, (size_t) sizeof(ipc_auth) - 1) != (ssize_t) sizeof(ipc_auth) - 1) {
		errsv = errno;
		log_warn("_ipc_init_use_ro(): panet_read(): Unable to read IPC authentication string: (%s)\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	debug_printf(DEBUG_INFO, "[IPC]: Authentication received.\n");

	/* Safe to use strcmp(). ipc_auth will always be NULL terminated (granted by memset() + 1) */
	if (strcmp(ipc_auth, runs.config.exec.ipc_key)) {
		log_crit("_ipc_init_use_ro(): IPC authentication failed.\n");
		errno = EINVAL;
		return -1;
	}

	debug_printf(DEBUG_INFO, "_ipc_init_use_ro(): IPC authentication successful.\n");

	/* All good */
	return 0;
#else
	errno = ENOSYS;
	return -1;
#endif
}

static int _ipc_init_usd_wo(void) {
#if CONFIG_USE_IPC_PMQ == 1
	char *ipc_auth = NULL;
	int errsv = 0;

	if (!(ipc_auth = mm_alloc(runs.config.stat.ipc_msgsize + 1))) {
		errsv = errno;
		log_warn("_ipc_init_usd_wo(): mm_alloc(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Reset IPC authentication buffer */
	memset(ipc_auth, 0, runs.config.stat.ipc_msgsize + 1);

	/* Initialize IPC PMQ interface */
	if (pmq_stat_usd_init() < 0) {
		errsv = errno;
		log_warn("_ipc_init_usd_wo(): pmq_stat_usd_init(): %s\n", strerror(errno));
		mm_free(ipc_auth);
		errno = errsv;
		return -1;
	}

	/* Craft IPC authentication string */
	strncpy(ipc_auth, runs.config.stat.ipc_key, runs.config.stat.ipc_msgsize);

	debug_printf(DEBUG_INFO, "[IPC]: Send authentication to uSched Daemon (usd)...\n");

	/* Send IPC authentication string */
	if (mq_send(runs.ipcd_usd_wo, ipc_auth, (size_t) runs.config.stat.ipc_msgsize, 0) < 0) {
		errsv = errno;
		log_crit("_ipc_init_usd_wo(): mq_send(): %s\n", strerror(errno));
		mm_free(ipc_auth);
		errno = errsv;
		return -1;
	}

	debug_printf(DEBUG_INFO, "[IPC]: Authentication sent.\n");

	/* Reset IPC authentication buffer (again) */
	memset(ipc_auth, 0, runs.config.stat.ipc_msgsize + 1);

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
	if ((runs.ipcd_usd_wo = panet_client_unix(runs.config.stat.ipc_name, PANET_PROTO_UNIX_STREAM)) < 0) {
		errsv = errno;
		log_warn("_ipc_init_usd_wo(): panet_client_unix(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}
 #elif CONFIG_USE_IPC_INET == 1
	if ((runs.ipcd_usd_wo = panet_client_ipv4(CONFIG_USE_IPC_INET_BINDADDR, runs.config.stat.ipc_name, PANET_PROTO_TCP, 5)) < 0) {
		errsv = errno;
		log_warn("_ipc_init_usd_wo(): panet_client_ipv4(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}
 #endif

	/* Craft IPC authentication string */
	strncpy(ipc_auth, runs.config.stat.ipc_key, CONFIG_USCHED_AUTH_IPC_SIZE);

	debug_printf(DEBUG_INFO, "[IPC]: Send authentication to uSched Daemon (usd)...\n");

	if (panet_write(runs.ipcd_usd_wo, ipc_auth, (size_t) sizeof(ipc_auth) - 1) != (ssize_t) sizeof(ipc_auth) - 1) {
		errsv = errno;
		log_crit("_ipc_init_usd_wo(): panet_write(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	debug_printf(DEBUG_INFO, "[IPC]: Authentication sent.\n");

	/* All good */
	return 0;
#else
	errno = ENOSYS;
	return -1;
#endif

}

int ipc_stat_init(void) {
	int errsv = 0;

	/* Initialize IPC interface with uSched Executer (use) */
	if (_ipc_init_use_ro() < 0) {
		errsv = errno;
		log_warn("ipc_stat_init(): _ipc_init_use_ro(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Initialize IPC interface with uSched Daemon (usd) */
	if (_ipc_init_usd_wo() < 0) {
		errsv = errno;
		log_warn("ipc_stat_init(): _ipc_init_usd_wo(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* All good */
	return 0;
}

void ipc_stat_destroy(void) {
#if CONFIG_USE_IPC_PMQ == 1
	pmq_stat_usd_destroy();
	pmq_stat_use_destroy();
#elif CONFIG_USE_IPC_UNIX == 1 || CONFIG_USE_IPC_INET == 1
	panet_safe_close(runs.ipcd_usd_wo);
	panet_safe_close(runs.ipcd_use_ro);
	panet_safe_close(runs.ipc_bind_fd);
#else
	return;
#endif
}

