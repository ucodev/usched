/**
 * @file runtime.c
 * @brief uSched
 *        Runtime handlers interface
 *
 * Date: 24-06-2014
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


#include <stdio.h> /* Debug */
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include <pall/cll.h>

#include "debug.h"
#include "runtime.h"
#include "mm.h"
#include "conn.h"
#include "usage.h"
#include "parse.h"
#include "op.h"
#include "log.h"
#include "thread.h"
#include "pool.h"
#include "schedule.h"
#include "pmq.h"

struct usched_runtime_client runc;
struct usched_runtime_daemon rund;
struct usched_runtime_exec rune;

int runtime_client_init(int argc, char **argv) {
	memset(&runc, 0, sizeof(struct usched_runtime_client));

	runc.argc = argc;
	runc.argv = argv;
	runc.t = time(NULL);

	/* Initialize logging interface */
	if (log_client_init() < 0) {
		debug_printf(DEBUG_CRIT, "runtime_client_init(): log_client_init(): %s\n", strerror(errno));
		return -1;
	}

	/* Parse requested command line instruction */
	if (!(runc.req = parse_instruction_array(argc - 1, &argv[1]))) {
		usage_client_show();
		return -1;
	}

	/* Initialize pools */
	if (pool_client_init() < 0) {
		log_crit("runtime_client_init(): pool_client_init(): %s\n", strerror(errno));
		return -1;
	}

	/* Process requested operations */
	if (op_client_process() < 0) {
		log_crit("runtime_client_init(): op_client_process(): %s\n", strerror(errno));
		return -1;
	}

	/* Initialize client connection handlers */
	if (conn_client_init() < 0) {
		log_crit("runtime_client_init(): conn_client_init(): %s\n", strerror(errno));
		return -1;
	}

	/* All good */
	return 0;
}

int runtime_daemon_init(int argc, char **argv) {
	memset(&rund, 0, sizeof(struct usched_runtime_daemon));

	rund.argc = argc;
	rund.argv = argv;

	/* Initialize logging interface */
	if (log_daemon_init() < 0) {
		debug_printf(DEBUG_CRIT, "runtime_daemon_init(): log_daemon_init(): %s\n", strerror(errno));
		return -1;
	}

	/* Initialize IPC */
	if (pmq_daemon_init() < 0) {
		log_crit("runtime_daemon_init(): pmq_daemon_init(): %s\n", strerror(errno));
		return -1;
	}

	/* Initialize mutexes */
	if (thread_daemon_mutexes_init() < 0) {
		log_crit("runtime_daemon_init(): thread_mutexes_init(): %s\n", strerror(errno));
		return -1;
	}

	/* Initialize pools */
	if (pool_daemon_init() < 0) {
		log_crit("runtime_daemon_init(): pool_daemon_init(): %s\n", strerror(errno));
		return -1;
	}

	/* Initialize scheduling interface */
	if (schedule_daemon_init() < 0) {
		log_crit("runtime_daemon_init(): schedule_daemon_init(): %s\n", strerror(errno));
		return -1;
	}

	/* Initialize connections interface */
	if (conn_daemon_init() < 0) {
		log_crit("runtime_daemon_init(): conn_daemon_init(): %s\n", strerror(errno));
		return -1;
	}

	/* All good */
	return 0;
}

int runtime_exec_init(int argc, char **argv) {
	memset(&rune, 0, sizeof(struct usched_runtime_daemon));

	rune.argc = argc;
	rune.argv = argv;

	/* Initialize logging interface */
	if (log_exec_init() < 0) {
		debug_printf(DEBUG_CRIT, "runtime_exec_init(): log_exec_init(): %s\n", strerror(errno));
		return -1;
	}

	/* Initialize IPC */
	if (pmq_exec_init() < 0) {
		log_crit("runtime_daemon_init(): pmq_daemon_init(): %s\n", strerror(errno));
		return -1;
	}

	/* Initialize threads behaviour */
	if (thread_exec_behaviour_init() < 0) {
		log_crit("thread_exec_behaviour_init(): %s\n", strerror(errno));
		return -1;
	}

	return 0;
}

void runtime_client_destroy(void) {
	parse_req_destroy(runc.req);

	if (runc.usage_err_offending)
		mm_free(runc.usage_err_offending);

	conn_client_destroy();

	pool_client_destroy();

	log_destroy();
}

void runtime_daemon_destroy(void) {
	/* Destroy connections interface */
	conn_daemon_destroy();

	/* Destroy pools */
	pool_daemon_destroy();

	/* Destroy mutexes */
	thread_daemon_mutexes_destroy();

	/* Destroy logging interface */
	log_destroy();
}

void runtime_exec_destroy(void) {
	/* Destroy logging interface */
	log_destroy();
}

