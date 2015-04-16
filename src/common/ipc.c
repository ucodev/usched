/**
 * @file ipc.c
 * @brief uSched
 *        Inter-Process Communication interface - Common
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

#include "config.h"
#include "ipc.h"
#include "log.h"

#if CONFIG_USE_IPC_PMQ == 1
 #include <time.h>
 #include <mqueue.h>
#elif CONFIG_USE_IPC_UNIX == 1 || CONFIG_USE_IPC_INET == 1
 #include <panet/panet.h>
#else
 #error "No IPC mechanism defined."
#endif

/* TODO: Implement generic calls to create and authenticate the IPC interface */

int ipc_timedsend(ipcd_t ipcd, const char *msg, size_t count, const struct timespec *timeout) {
	int errsv = 0;
#if CONFIG_USE_IPC_PMQ == 1
	if (mq_timedsend(ipcd, msg, count, 0, timeout) < 0) {
		errsv = errno;
		log_warn("ipc_send(): mq_timedsend(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}
#elif CONFIG_USE_IPC_UNIX == 1 || CONFIG_USE_IPC_INET == 1
	if (panet_write(ipcd, msg, count) != (ssize_t) count) {
		errsv = errno;
		log_warn("ipc_send(): panet_write(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}
#else
 #error "No IPC mechanism defined."
#endif
	return 0;
}

int ipc_recv(ipcd_t ipcd, char *msg, size_t count) {
	int errsv = 0;
#if CONFIG_USE_IPC_PMQ == 1
	if (mq_receive(ipcd, msg, count, 0) < 0) {
		errsv = errno;
		log_warn("ipc_recv(): mq_receive(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}
#elif CONFIG_USE_IPC_UNIX == 1 || CONFIG_USE_IPC_INET == 1
	if (panet_read(ipcd, msg, count) != (ssize_t) count) {
		errsv = errno;
		log_warn("ipc_recv(): panet_read(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}
#else
 #error "No IPC mechanism defined."
#endif
	return 0;
}

size_t ipc_pending(ipcd_t ipcd) {
	size_t nmemb = 0;
#if CONFIG_USE_IPC_PMQ == 1
	int errsv = 0;
	struct mq_attr mqattr;

	memset(&mqattr, 0, sizeof(struct mq_attr));

	if (mq_getattr(ipcd, &mqattr) < 0) {
		errsv = errno;
		log_warn("ipc_pending(): mq_getattr(): %s\n", strerror(errno));
		errno = errsv;
		nmemb = 0;
	} else {
		nmemb = mqattr.mq_curmsgs;
	}
#endif
	return nmemb;
}

void ipc_close(ipcd_t ipcd) {
#if CONFIG_USE_IPC_PMQ == 1
	mq_close(ipcd);
#elif CONFIG_USE_IPC_UNIX == 1 || CONFIG_USE_IPC_INET == 1
	panet_safe_close(ipcd);
#else
 #error "No IPC mechanism defined."
#endif
}

