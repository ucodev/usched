/**
 * @file sig.c
 * @brief uSched
 *        Signals interface - Daemon
 *
 * Date: 05-03-2015
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
#include <signal.h>
#include <stdlib.h>
#include <pthread.h>

#include <psched/psched.h>

#include "config.h"
#include "runtime.h"
#include "bitops.h"
#include "log.h"
#include "sig.h"


static void _sig_term_daemon_handler(int n) {
	bit_set(&rund.flags, USCHED_RUNTIME_FLAG_TERMINATE);

	/* Cancel active threads */
	pthread_cancel(rund.t_unix);
	pthread_cancel(rund.t_remote);
}

static void _sig_hup_daemon_handler(int n) {
	bit_set(&rund.flags, USCHED_RUNTIME_FLAG_RELOAD);

	/* Cancel active threads */
	pthread_cancel(rund.t_unix);
	pthread_cancel(rund.t_remote);
}

static void _sig_usr1_daemon_handler(int n) {
	bit_set(&rund.flags, USCHED_RUNTIME_FLAG_FLUSH);

	/* Cancel active threads */
	pthread_cancel(rund.t_unix);
	pthread_cancel(rund.t_remote);
}

static void _sig_pipe_daemon_handler(int n) {
	/* Ignore SIGPIPE */
	return;
}

static void _sig_abrt_daemon_handler(int n) {
	/* Check if the abort signal came from the psched library */
	if (rund.psched && psched_fatal(rund.psched)) {
		/* If so, just reload the daemon to force a clean state */
		bit_set(&rund.flags, USCHED_RUNTIME_FLAG_RELOAD);

		/* Cancel active threads */
		pthread_cancel(rund.t_unix);
		pthread_cancel(rund.t_remote);
	}

	/* Otherwise we don't know where this signal came from... better abort execution */
	exit(PROCESS_EXIT_STATUS_CUSTOM_UNKNOWN_ABORT);
}

int sig_daemon_init(void) {
	int errsv = 0;
	struct sigaction sa;

	memset(&sa, 0, sizeof(struct sigaction));

	sa.sa_handler = _sig_term_daemon_handler;
	sigemptyset(&sa.sa_mask);

	if (sigaction(SIGTERM, &sa, &rund.sa_save) < 0) {
		errsv = errno;
		log_warn("sig_daemon_init(): sigaction(SIGTERM, ...): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	if (sigaction(SIGINT, &sa, NULL) < 0) {
		errsv = errno;
		log_warn("sig_daemon_init(): sigaction(SIGINT, ...): %s\n", strerror(errno));
		goto _failure;
	}

	if (sigaction(SIGQUIT, &sa, NULL) < 0) {
		errsv = errno;
		log_warn("sig_daemon_init(): sigaction(SIGQUIT, ...): %s\n", strerror(errno));
		goto _failure;
	}

	sa.sa_handler = _sig_hup_daemon_handler;

	if (sigaction(SIGHUP, &sa, NULL) < 0) {
		errsv = errno;
		log_warn("sig_daemon_init(): sigaction(SIGHUP, ...): %s\n", strerror(errno));
		goto _failure;
	}

	sa.sa_handler = _sig_pipe_daemon_handler;

	if (sigaction(SIGPIPE, &sa, NULL) < 0) {
		errsv = errno;
		log_warn("sig_daemon_init(): sigaction(SIGPIPE, ...): %s\n", strerror(errno));
		goto _failure;
	}

	sa.sa_handler = _sig_usr1_daemon_handler;

	if (sigaction(SIGUSR1, &sa, NULL) < 0) {
		errsv = errno;
		log_warn("sig_daemon_init(): sigaction(SIGUSR1, ...): %s\n", strerror(errno));
		goto _failure;
	}

	sa.sa_handler = _sig_abrt_daemon_handler;

	if (sigaction(SIGABRT, &sa, NULL) < 0) {
		errsv = errno;
		log_warn("sig_daemon_init(): sigaction(SIGABRT, ...): %s\n", strerror(errno));
		goto _failure;
	}

	return 0;

_failure:
	sig_daemon_destroy();

	errno = errsv;

	return -1;
}

void sig_daemon_destroy(void) {
	sigaction(SIGTERM, &rund.sa_save, NULL);
	sigaction(SIGINT, &rund.sa_save, NULL);
	sigaction(SIGQUIT, &rund.sa_save, NULL);
	sigaction(SIGHUP, &rund.sa_save, NULL);
	sigaction(SIGPIPE, &rund.sa_save, NULL);
	sigaction(SIGUSR2, &rund.sa_save, NULL);
}

