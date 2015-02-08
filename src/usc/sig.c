/**
 * @file sig.c
 * @brief uSched
 *        Signals interface - Client
 *
 * Date: 08-02-2015
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
#include <pthread.h>

#include "config.h"
#include "runtime.h"
#include "bitops.h"
#include "log.h"
#include "sig.h"

static void _sig_pipe_client_handler(int n) {
	/* Ignore SIGPIPE */
	return;
}

int sig_client_init(void) {
#if !defined(COMPILE_WIN32) || COMPILE_WIN32 == 0
	int errsv = 0;
	struct sigaction sa;

	memset(&sa, 0, sizeof(struct sigaction));

	sa.sa_handler = _sig_pipe_client_handler;
	sigemptyset(&sa.sa_mask);

	if (sigaction(SIGPIPE, &sa, &runc.sa_save) < 0) {
		errsv = errno;
		log_warn("sig_client_init(): sigaction(SIGPIPE, ...): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}
#endif
	return 0;
}

void sig_client_destroy(void) {
#if !defined(COMPILE_WIN32) || COMPILE_WIN32 == 0
	sigaction(SIGPIPE, &runc.sa_save, NULL);
#endif
}

