/**
 * @file conn.c
 * @brief uSched
 *        Connections interface
 *
 * Date: 27-06-2014
 * 
 * Copyright 2014 Pedro A. Hortas (pah@ucodev.org)
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

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <rtsaio/rtsaio.h>
#include <panet/panet.h>

#include "debug.h"
#include "config.h"
#include "mm.h"
#include "runtime.h"
#include "notify.h"
#include "conn.h"
#include "log.h"

int conn_client_init(void) {
	if ((runc.fd = panet_client_unix(CONFIG_USCHED_CONN_USER_NAMED_SOCKET, PANET_PROTO_UNIX_STREAM)) < 0) {
		log_crit("conn_client_init(): panet_client_unix(): %s\n", strerror(errno));
		return -1;
	}

	return 0;
}

int conn_client_process(void) {
	struct usched_entry *cur = NULL;
	size_t cmd_len = 0;

	while ((cur = runc.epool->pop(runc.epool))) {
		cmd_len = cur->cmd_size;

		/* Convert endianness to network by order */
		cur->id = htonll(cur->id);
		cur->flags = htonl(cur->flags);
		cur->uid = htonl(cur->uid);
		cur->gid = htonl(cur->gid);
		cur->trigger = htonl(cur->trigger);
		cur->step = htonl(cur->step);
		cur->expire = htonl(cur->expire);
		cur->cmd_size = htonl(cur->cmd_size);

		if (write(runc.fd, cur, usched_entry_hdr_size()) != usched_entry_hdr_size()) {
			log_crit("conn_client_process(): write() != %d: %s\n", usched_entry_hdr_size(), strerror(errno));
			entry_destroy(cur);
			return -1;
		}

		if (write(runc.fd, cur->cmd, cmd_len) != cmd_len) {
			log_crit("conn_client_process(): write() != cmd_len: %s\n", strerror(errno));
			entry_destroy(cur);
			return -1;
		}

		/* Read the response in order to obtain the entry id */
		if (read(runc.fd, &cur->id, sizeof(cur->id)) != sizeof(cur->id)) {
			log_crit("conn_client_process(): read() != %d: %s\n", sizeof(cur->id), strerror(errno));
			entry_destroy(cur);
			return -1;
		}

		cur->id = ntohll(cur->id);

		debug_printf(DEBUG_INFO, "Received Entry ID: %llu\n", cur->id);

		entry_destroy(cur);
	}

	return 0;
}

void conn_client_destroy(void) {
	panet_safe_close(runc.fd);
}

int conn_daemon_init(void) {
	if (rtsaio_init(-5, SCHED_OTHER, 0, &notify_write, &notify_read) < 0) {
		log_crit("conn_daemon_init(): rtsaio_init(): %s\n", strerror(errno));
		return -1;
	}

	if ((rund.fd = panet_server_unix(CONFIG_USCHED_CONN_USER_NAMED_SOCKET, PANET_PROTO_UNIX_STREAM, 10)) < 0) {
		log_crit("conn_daemon_init(): panet_server_unix(): %s\n", strerror(errno));
		return -1;
	}

	if (chmod(CONFIG_USCHED_CONN_USER_NAMED_SOCKET, 0666) < 0) {
		log_crit("conn_daemon_init(): chmod(): %s\n", strerror(errno));
		return -1;
	}

	return 0;
}

void conn_daemon_process(void) {
	sock_t fd = 0;
	struct async_op *aop = NULL;

	for (;;) {
		/* Check for runtime interruptions */
		if (runtime_daemon_interrupted())
			break;

		/* Accept client connection */
		if ((fd = accept(rund.fd, NULL, NULL)) < 0) {
			log_warn("conn_daemon_process(): accept(): %s\n", strerror(errno));
			continue;
		}

		if (!(aop = mm_alloc(sizeof(struct async_op)))) {
			log_warn("conn_daemon_process(): mm_alloc(): %s\n", strerror(errno));
			panet_safe_close(fd);
			continue;
		}

		memset(aop, 0, sizeof(struct async_op));

		aop->fd = fd;
		/* id(4), flags(4), uid(4), gid(4), trigger(4), step(4), expire(4), cmd_size(4) */
		aop->count = usched_entry_hdr_size();
		aop->priority = 0;
		aop->timeout.tv_sec = CONFIG_USCHED_CONN_TIMEOUT;

		if (!(aop->data = mm_alloc(aop->count))) {
			log_warn("conn_daemon_process(): mm_alloc(): %s\n", strerror(errno));
			mm_free(aop);
			panet_safe_close(fd);
			continue;
		}

		memset((void *) aop->data, 0, aop->count);

		if (rtsaio_read(aop) < 0) {
			log_warn("conn_daemon_process(): rtsaio_read(): %s\n", strerror(errno));
			mm_free((void *) aop->data);
			mm_free(aop);
			panet_safe_close(fd);
			continue;
		}
	}
}

void conn_daemon_destroy(void) {
	panet_safe_close(rund.fd);
}

int conn_is_local(int fd) {
	int ret = panet_info_sock_family(fd);

	if (ret < 0) {
		log_warn("conn_is_local(): panet_info_sock_family(): %s\n", strerror(errno));
		return -1;
	}

	return (ret == AF_UNIX);
}

int conn_is_remote(int fd) {
	int ret = panet_info_sock_family(fd);

	if (ret < 0) {
		log_warn("conn_is_remote(): panet_info_sock_family(): %s\n", strerror(errno));
		return -1;
	}

	return (ret != AF_UNIX);
}

