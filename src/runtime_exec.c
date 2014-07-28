/**
 * @file runtime_exec.c
 * @brief uSched
 *        Runtime handlers interface - Exec
 *
 * Date: 28-07-2014
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


#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "debug.h"
#include "runtime.h"
#include "log.h"
#include "thread.h"
#include "schedule.h"
#include "pmq.h"
#include "sig.h"
#include "bitops.h"


int runtime_exec_init(int argc, char **argv) {
	int errsv = 0;

	memset(&rune, 0, sizeof(struct usched_runtime_exec));

	rune.argc = argc;
	rune.argv = argv;

	/* Initialize logging interface */
	if (log_exec_init() < 0) {
		errsv = errno;
		debug_printf(DEBUG_CRIT, "runtime_exec_init(): log_exec_init(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	log_info("Logging interface initialized.\n");

	/* Initialize signals interface */
	log_info("Initializing signals interface...\n");

	if (sig_exec_init() < 0) {
		errsv = errno;
		log_crit("runtime_exec_init(): sig_exec_init(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	log_info("Signals interface initialized.\n");

	/* Initialize threads behaviour */
	log_info("Initializing thread behaviour interface...\n");

	if (thread_exec_behaviour_init() < 0) {
		errsv = errno;
		log_crit("runtmie_exec_init(): thread_exec_behaviour_init(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	log_info("Thread behaviour interface initialized.\n");

	/* Initialize IPC */
	log_info("Initializing IPC interface...\n");

	if (pmq_exec_init() < 0) {
		errsv = errno;
		log_crit("runtime_exec_init(): pmq_exec_init(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	log_info("IPC interface initialized.\n");

	/* All good */
	log_info("All systems go. Ignition!\n");

	return 0;
}

int runtime_exec_interrupted(void) {
	if (bit_test(&rune.flags, USCHED_RUNTIME_FLAG_TERMINATE) || bit_test(&rune.flags, USCHED_RUNTIME_FLAG_RELOAD))
		return 1;

	return 0;
}

void runtime_exec_destroy(void) {
	/* Destroy IPC interface */
	log_info("Destroying IPC interface...\n");
	pmq_exec_destroy();
	log_info("IPC interface destroyed.\n");

	/* Destroy thread behaviour interface */
	log_info("Destroying thread behaviour interface...\n");
	thread_exec_behaviour_destroy();
	log_info("Thread behaviour interface destroyed.\n");

	/* Destroy signals interface */
	log_info("Destroying signals interface...\n");
	sig_exec_destroy();
	log_info("Signals interface destroyed.\n");

	log_info("All systems stopped.\n");

	/* Destroy logging interface */
	log_info("Destroying logging interface...\n");
	log_destroy();
}

