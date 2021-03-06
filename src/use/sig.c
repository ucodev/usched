/**
 * @file sig.c
 * @brief uSched
 *        Signals interface - Exec
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


static void _sig_term_exec_handler(int n) {
	bit_set(&rune.flags, USCHED_RUNTIME_FLAG_TERMINATE);
}

static void _sig_hup_exec_handler(int n) {
	bit_set(&rune.flags, USCHED_RUNTIME_FLAG_RELOAD);
}

int sig_exec_init(void) {
	int errsv = 0;
	struct sigaction sa;

	memset(&sa, 0, sizeof(struct sigaction));

	sa.sa_handler = _sig_term_exec_handler;
	sigemptyset(&sa.sa_mask);

	if (sigaction(SIGTERM, &sa, &rune.sa_save) < 0) {
		errsv = errno;
		log_warn("sig_exec_init(): sigaction(SIGTERM, ...): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	if (sigaction(SIGINT, &sa, NULL) < 0) {
		errsv = errno;
		log_warn("sig_exec_init(): sigaction(SIGINT, ...): %s\n", strerror(errno));
		goto _failure;
	}

	if (sigaction(SIGQUIT, &sa, NULL) < 0) {
		errsv = errno;
		log_warn("sig_exec_init(): sigaction(SIGQUIT, ...): %s\n", strerror(errno));
		goto _failure;
	}

	sa.sa_handler = _sig_hup_exec_handler;

	if (sigaction(SIGHUP, &sa, NULL) < 0) {
		errsv = errno;
		log_warn("sig_exec_init(): sigaction(SIGHUP, ...): %s\n", strerror(errno));
		goto _failure;
	}

	return 0;

_failure:
	sig_exec_destroy();

	errno = errsv;

	return -1;
}

void sig_exec_destroy(void) {
	sigaction(SIGTERM, &rune.sa_save, NULL);
	sigaction(SIGINT, &rune.sa_save, NULL);
	sigaction(SIGQUIT, &rune.sa_save, NULL);
	sigaction(SIGHUP, &rune.sa_save, NULL);
}

