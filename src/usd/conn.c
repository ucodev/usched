/**
 * @file conn.c
 * @brief uSched
 *        Connections interface - Daemon
 *
 * Date: 11-08-2014
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
#include <pthread.h>

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

int conn_daemon_init(void) {
	int errsv = 0;

	if (rtsaio_init(-rund.config.core.thread_workers, SCHED_OTHER, 0, &notify_write, &notify_read) < 0) {
		errsv = errno;
		log_crit("conn_daemon_init(): rtsaio_init(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	if ((rund.fd_unix = panet_server_unix(rund.config.network.sock_named, PANET_PROTO_UNIX_STREAM, 10)) < 0) {
		errsv = errno;
		log_crit("conn_daemon_init(): panet_server_unix(\"%s\", ...): %s\n", rund.config.network.sock_named, strerror(errno));
		errno = errsv;
		return -1;
	}

	if ((rund.fd_remote = panet_server_ipv4(rund.config.network.bind_addr, rund.config.network.bind_port, PANET_PROTO_TCP, rund.config.network.conn_limit)) < 0) {
		errsv = errno;
		log_crit("conn_daemon_init(): panet_server_ipv4(\"%s\", \"%s\", ...): %s\n", rund.config.network.bind_addr, rund.config.network.bind_port, strerror(errno));
		errno = errsv;
		return -1;
	}

	if (chmod(rund.config.network.sock_named, 0666) < 0) {
		errsv = errno;
		log_crit("conn_daemon_init(): chmod(\"%s\", 0666): %s\n", rund.config.network.sock_named, strerror(errno));
		errno = errsv;
		return -1;
	}

	return 0;
}

static int _conn_daemon_process_fd(int fd) {
	struct async_op *aop = NULL;

	if (!(aop = mm_alloc(sizeof(struct async_op)))) {
		log_warn("conn_daemon_process(): mm_alloc(): %s\n", strerror(errno));
		panet_safe_close(fd);
		return -1;
	}

	memset(aop, 0, sizeof(struct async_op));

	aop->fd = fd;
	/* id(4), flags(4), uid(4), gid(4), trigger(4), step(4), expire(4), psize(4) */
	aop->count = usched_entry_hdr_size();
	aop->priority = 0;
	aop->timeout.tv_sec = rund.config.network.conn_timeout;

	if (!(aop->data = mm_alloc(aop->count))) {
		log_warn("conn_daemon_process(): mm_alloc(): %s\n", strerror(errno));
		mm_free(aop);
		panet_safe_close(fd);
		return -1;
	}

	memset((void *) aop->data, 0, aop->count);

	if (rtsaio_read(aop) < 0) {
		log_warn("conn_daemon_process(): rtsaio_read(): %s\n", strerror(errno));
		mm_free((void *) aop->data);
		mm_free(aop);
		panet_safe_close(fd);
		return -1;
	}

	return 0;
}

static void *_conn_daemon_process_accept_unix(void *arg) {
	sock_t fd = 0;

	for (;;) {
		/* Check for runtime interruptions */
		if (runtime_daemon_interrupted())
			break;

		/* Accept client connection */
		if ((fd = accept(rund.fd_unix, NULL, NULL)) < 0) {
			log_warn("conn_daemon_process_accept_unix(): accept(): %s\n", strerror(errno));
			continue;
		}

		if (_conn_daemon_process_fd(fd) < 0) {
			log_warn("conn_daemon_process_accept_unix(): _conn_daemon_process_fd(): %s\n", strerror(errno));
			continue;
		}
	}

	pthread_exit(NULL);

	return NULL;
}

static void *_conn_daemon_process_accept_remote(void *arg) {
	sock_t fd = 0;

	for (;;) {
		/* Check for runtime interruptions */
		if (runtime_daemon_interrupted())
			break;

		/* Accept client connection */
		if ((fd = accept(rund.fd_remote, NULL, NULL)) < 0) {
			log_warn("conn_daemon_process_accept_remote(): accept(): %s\n", strerror(errno));
			continue;
		}

		if (_conn_daemon_process_fd(fd) < 0) {
			log_warn("conn_daemon_process_accept_remote(): _conn_daemon_process_fd(): %s\n", strerror(errno));
			continue;
		}
	}

	pthread_exit(NULL);

	return NULL;
}

int conn_daemon_process_all(void) {
	int errsv = 0;

	if (pthread_create(&rund.t_unix, NULL, _conn_daemon_process_accept_unix, NULL)) {
		errsv = errno;
		log_crit("conn_daemon_process_all(): pthread_create(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	if (pthread_create(&rund.t_remote, NULL, _conn_daemon_process_accept_remote, NULL)) {
		errsv = errno;
		log_crit("conn_daemon_process_all(): pthread_create(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	pthread_join(rund.t_unix, NULL);
	pthread_join(rund.t_remote, NULL);

	return 0;
}

void conn_daemon_destroy(void) {
	panet_safe_close(rund.fd_unix);
	panet_safe_close(rund.fd_remote);
	rtsaio_destroy();
}

