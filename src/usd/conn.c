/**
 * @@file conn.c
 * @@brief uSched
 *        Connections interface - Daemon
 *
 * Date: 19-03-2015
 * 
 * Copyright 2014-2015 Pedro A. Hortas (pah@@ucodev.org)
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

#include <sys/select.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <rtsaio/rtsaio.h>
#include <panet/panet.h>

#include <unistd.h>

#include "debug.h"
#include "config.h"
#include "mm.h"
#include "runtime.h"
#include "notify.h"
#include "conn.h"
#include "log.h"
#include "gc.h"

static int _conn_daemon_unix_init(void) {
	int errsv = 0;

	/* Check if local connections are authorized */
	if (!rund.config.auth.local_use) {
		log_info("_conn_daemon_unix_init(): Skipping local connections manager.\n");
		return 0;
	}

	/* Initialize local connections manager */
	if ((rund.fd_unix = panet_server_unix(rund.config.network.sock_name, PANET_PROTO_UNIX_STREAM, 10)) < 0) {
		errsv = errno;
		log_crit("_conn_daemon_unix_init(): panet_server_unix(\"%s\", ...): %s\n", rund.config.network.sock_name, strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Grant read/write privileges to everyone on the named socket */
	if (chmod(rund.config.network.sock_name, 0666) < 0) {
		errsv = errno;
		log_crit("_conn_daemon_unix_init(): chmod(\"%s\", 0666): %s\n", rund.config.network.sock_name, strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Set non-blocking */
	if (conn_set_nonblock(rund.fd_unix) < 0) {
		errsv = errno;
		log_crit("_conn_daemon_unix_init(): conn_set_nonblock(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* All good */
	return 0;
}

static int _conn_daemon_remote_init(void) {
	int errsv = 0;

	/* Check if remote connections are authorized */
	if (!rund.config.auth.remote_users) {
		log_info("_conn_daemon_remote_init(): Skipping remote connections manager.\n");
		return 0;
	}

	/* Initialize remote connections manager */
	if ((rund.fd_remote = panet_server_ipv4(rund.config.network.bind_addr, rund.config.network.bind_port, PANET_PROTO_TCP, (int) rund.config.network.conn_limit)) < 0) {
		errsv = errno;
		log_crit("conn_daemon_remote_init(): panet_server_ipv4(\"%s\", \"%s\", ...): %s\n", rund.config.network.bind_addr, rund.config.network.bind_port, strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Set non-blocking */
	if (conn_set_nonblock(rund.fd_remote) < 0) {
		errsv = errno;
		log_crit("conn_daemon_remote_init(): conn_set_nonblock(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* All good */
	return 0;
}

int conn_daemon_init(void) {
	int errsv = 0;

	/* Initialize RTSAIO */
	if (rtsaio_init(-(int) rund.config.core.thread_workers, SCHED_OTHER, rund.config.core.thread_priority, &notify_write, &notify_read) < 0) {
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
	sigset_t si_cur, si_prev;
	fd_set fd_rset;

	/* Initialize signal sets */
	sigfillset(&si_cur);
	sigemptyset(&si_prev);

	/* Check if this thread deserves to live */
	if (!rund.config.auth.local_use)
		pthread_exit(NULL);

	for (;;) {
		/* Empty garbage collector */
		gc_cleanup();

		/* Block all signals */
		pthread_sigmask(SIG_SETMASK, &si_cur, &si_prev);

		/* Check for runtime interruptions */
		if (runtime_daemon_interrupted()) {
			log_info("_conn_daemon_process_accept_unix(): Runtime interruption detected...\n");
			pthread_sigmask(SIG_SETMASK, &si_prev, NULL);
			break;
		}

		/* Expect data only from the bound file descriptor */
		FD_ZERO(&fd_rset);
		FD_SET(rund.fd_unix, &fd_rset);

#if CONFIG_USE_SELECT == 0
		/* Wait for activity on the file descriptor, but resume execution when a signal is
		 * caught.
		 */
		if (pselect(rund.fd_unix + 1, &fd_rset, NULL, NULL, NULL, &si_prev) < 0) {
			log_warn("conn_daemon_process_accept_unix(): pselect(): %s\n", strerror(errno));
			pthread_sigmask(SIG_SETMASK, &si_prev, NULL);
			continue;
		}
#endif

		/* Signals can now be caught */
		pthread_sigmask(SIG_SETMASK, &si_prev, NULL);

#if CONFIG_USE_SELECT == 1
		/* NOTE: Systems not supporting pselect() will have a possible race here */

		/* Wait for activity on the file descriptor, but resume execution when a signal is
		 * caught.
		 */
		if (select(rund.fd_unix + 1, &fd_rset, NULL, NULL, NULL) < 0) {
			log_warn("conn_daemon_process_accept_unix(): select(): %s\n", strerror(errno));
			pthread_sigmask(SIG_SETMASK, &si_prev, NULL);
			continue;
		}
#endif

		/* Validate if we've actually received data for processing, or if the interruption
		 * was caused by a signal.
		 */
		if (!FD_ISSET(rund.fd_unix, &fd_rset))
			continue;

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

	log_info("_conn_daemon_process_accept_unix(): Thread exiting...\n");

	pthread_exit(NULL);

	return NULL;
}

static void *_conn_daemon_process_accept_remote(void *arg) {
	sock_t fd = 0;
	sigset_t si_cur, si_prev;
	fd_set fd_rset;

	/* Initialize signal sets */
	sigfillset(&si_cur);
	sigemptyset(&si_prev);

	/* Check if this thread deserves to live */
	if (!rund.config.auth.remote_users)
		pthread_exit(NULL);

	for (;;) {
		/* Empty garbage collector */
		gc_cleanup();

		/* Block all signals */
		pthread_sigmask(SIG_SETMASK, &si_cur, &si_prev);

		/* Check for runtime interruptions */
		if (runtime_daemon_interrupted()) {
			log_info("_conn_daemon_process_accept_remote(): Runtime interruption detected...\n");
			pthread_sigmask(SIG_SETMASK, &si_prev, NULL);
			break;
		}

		/* Expect data only from the bound file descriptor */
		FD_ZERO(&fd_rset);
		FD_SET(rund.fd_remote, &fd_rset);

#if CONFIG_USE_SELECT == 0
		/* Wait for activity on the file descriptor, but resume execution when a signal is
		 * caught.
		 */
		if (pselect(rund.fd_remote + 1, &fd_rset, NULL, NULL, NULL, &si_prev) < 0) {
			log_warn("conn_daemon_process_accept_remote(): pselect(): %s\n", strerror(errno));
			pthread_sigmask(SIG_SETMASK, &si_prev, NULL);
			continue;
		}
#endif

		/* Signals can now be caught */
		pthread_sigmask(SIG_SETMASK, &si_prev, NULL);

#if CONFIG_USE_SELECT == 1
		/* NOTE: Systems not supporting pselect() will have a possible race here */

		/* Wait for activity on the file descriptor, but resume execution when a signal is
		 * caught.
		 */
		if (select(rund.fd_remote + 1, &fd_rset, NULL, NULL, NULL) < 0) {
			log_warn("conn_daemon_process_accept_remote(): select(): %s\n", strerror(errno));
			pthread_sigmask(SIG_SETMASK, &si_prev, NULL);
			continue;
		}
#endif

		/* Validate if we've actually received data for processing, or if the interruption
		 * was caused by a signal.
		 */
		if (!FD_ISSET(rund.fd_remote, &fd_rset))
			continue;

		/* Accept client connection */
		if ((fd = accept(rund.fd_remote, NULL, NULL)) < 0) {
			log_warn("conn_daemon_process_accept_remote(): accept(): %s\n", strerror(errno));
			continue;
		}

		/* Increment the number of active connections */
		rund.conn_cur ++;

		/* Check if the current active connections exceeds the configuration limit */
		if (rund.conn_cur > rund.config.network.conn_limit) {
			log_warn("conn_daemon_process_accept_remote(): Maximum number of active connections exceedeed (current: %lu, maximum: %lu).\n", rund.conn_cur, rund.config.network.conn_limit);
			conn_daemon_client_close(fd);
			continue;
		}

		/* Process connection */
		if (_conn_daemon_process_fd(fd) < 0) {
			log_warn("conn_daemon_process_accept_remote(): _conn_daemon_process_fd(): %s\n", strerror(errno));
			continue;
		}
	}

	log_info("_conn_daemon_process_accept_remote(): Thread exiting...\n");

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
	if (conn_is_remote(fd))
		rund.conn_cur --;

	panet_safe_close(fd);
}

void conn_daemon_destroy(void) {
	panet_safe_close(rund.fd_unix);
	panet_safe_close(rund.fd_remote);
	rtsaio_destroy();
}

