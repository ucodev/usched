/**
 * @@file conn.c
 * @@brief uSched
 *        Connections interface - Daemon
 *
 * Date: 27-08-2014
 * 
 * Copyright 2014 Pedro A. Hortas (pah@@ucodev.org)
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

static int _conn_daemon_unix_init(void) {
	int errsv = 0;

	/* Initialize local connections manager */
	if ((rund.fd_unix = panet_server_unix(rund.config.network.sock_named, PANET_PROTO_UNIX_STREAM, 10)) < 0) {
		errsv = errno;
		log_crit("conn_daemon_unix_init(): panet_server_unix(\"%s\", ...): %s\n", rund.config.network.sock_named, strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Grant read/write privileges to everyone on the named socket */
	if (chmod(rund.config.network.sock_named, 0666) < 0) {
		errsv = errno;
		log_crit("conn_daemon_unix_init(): chmod(\"%s\", 0666): %s\n", rund.config.network.sock_named, strerror(errno));
		errno = errsv;
		return -1;
	}

	/* All good */
	return 0;
}

static int _conn_daemon_remote_init(void) {
	int errsv = 0;

	/* Check if remote connections are authorized */
	if (!rund.config.auth.users_remote) {
		log_info("_conn_daemon_remote_init(): Skipping remote connections manager.\n");
		return 0;
	}

	/* Initialize remote connections manager */
	if ((rund.fd_remote = panet_server_ipv4(rund.config.network.bind_addr, rund.config.network.bind_port, PANET_PROTO_TCP, rund.config.network.conn_limit)) < 0) {
		errsv = errno;
		log_crit("conn_daemon_remote_init(): panet_server_ipv4(\"%s\", \"%s\", ...): %s\n", rund.config.network.bind_addr, rund.config.network.bind_port, strerror(errno));
		errno = errsv;
		return -1;
	}

	/* All good */
	return 0;
}

int conn_daemon_init(void) {
	int errsv = 0;

	/* Initialize RTSAIO */
	if (rtsaio_init(-rund.config.core.thread_workers, SCHED_OTHER, rund.config.core.thread_priority, &notify_write, &notify_read) < 0) {
		errsv = errno;
		log_crit("conn_daemon_init(): rtsaio_init(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Initialize local connections handlers */
	if (_conn_daemon_unix_init() < 0) {
		errsv = errno;
		log_crit("conn_daemon_init(): _conn_daemon_unix_init(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Initialize remote connections handlers */
	if (_conn_daemon_remote_init() < 0) {
		errsv = errno;
		log_crit("conn_daemon_init(): _conn_daemon_remote_init(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* All good */
	return 0;
}

static int _conn_daemon_process_fd(int fd) {
	int errsv = 0;
	struct async_op *aop = NULL;

	/* Alloate enough memory for aop */
	if (!(aop = mm_alloc(sizeof(struct async_op)))) {
		errsv = errno;
		log_warn("conn_daemon_process(): mm_alloc(): %s\n", strerror(errno));
		conn_daemon_client_close(fd);
		errno = errsv;
		return -1;
	}

	/* Cleanup aop data */
	memset(aop, 0, sizeof(struct async_op));

	/* Setup aop */
	aop->fd = fd;
	/* id(8), flags(4), uid(4), gid(4), trigger(4), step(4), expire(4), psize(4) */
	aop->count = usched_entry_hdr_size();
	aop->priority = 0;
	aop->timeout.tv_sec = rund.config.network.conn_timeout;

	/* Allocate enough memory to received the entry header */
	if (!(aop->data = mm_alloc(aop->count))) {
		errsv = errno;
		log_warn("conn_daemon_process(): mm_alloc(): %s\n", strerror(errno));
		mm_free(aop);
		conn_daemon_client_close(fd);
		errno = errsv;
		return -1;
	}

	/* Cleanup aop->data for a fresh start */
	memset((void *) aop->data, 0, aop->count);

	/* Request a read operation of one entry header */
	if (rtsaio_read(aop) < 0) {
		errsv = errno;
		log_warn("conn_daemon_process(): rtsaio_read(): %s\n", strerror(errno));
		mm_free((void *) aop->data);
		mm_free(aop);
		conn_daemon_client_close(fd);
		errno = errsv;
		return -1;
	}

	/* All good */
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

		/* Process connection */
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

	/* Check if this thread deserves to live */
	if (!rund.config.auth.users_remote)
		pthread_exit(NULL);

	for (;;) {
		/* Check for runtime interruptions */
		if (runtime_daemon_interrupted())
			break;

		/* Accept client connection */
		if ((fd = accept(rund.fd_remote, NULL, NULL)) < 0) {
			log_warn("conn_daemon_process_accept_remote(): accept(): %s\n", strerror(errno));
			continue;
		}

		/* Increment the number of active connections */
		rund.conn_cur ++;

		/* Check if the current active connections exceeds the configuration limit */
		if (rund.conn_cur > rund.config.network.conn_limit) {
			log_warn("conn_daemon_process_accept_remote(): Maximum number of active connections exceedeed.\n");
			conn_daemon_client_close(fd);
			continue;
		}

		/* Process connection */
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

void conn_daemon_client_close(int fd) {
	panet_safe_close(fd);

	rund.conn_cur --;
}

void conn_daemon_destroy(void) {
	panet_safe_close(rund.fd_unix);
	panet_safe_close(rund.fd_remote);
	rtsaio_destroy();
}

