/**
 * @file daemon.c
 * @brief uSched
 *        Daemon Main Component
 *
 * Date: 30-01-2015
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
#include <stdlib.h>

#include "debug.h"
#include "bitops.h"
#include "runtime.h"
#include "marshal.h"
#include "log.h"
#include "conn.h"

static void _serialize(void) {
	if (marshal_daemon_serialize_pools() < 0) {
		log_warn("_serialize(): marshal_daemon_serialize_pools(): %s\n", strerror(errno));
		return;
	}

	log_info("_serialize(): Active pools were serialized.");

	marshal_daemon_destroy();

	if (marshal_daemon_init() < 0) {
		log_crit("_serialize(): marshal_daemon_init(): %s\n", strerror(errno));
		log_crit("_serialize(): === DO NOT TRY TO FLUSH DATA AGAIN! SERVICE RESTART IS REQUIRED ===");
	}
}

static void _init(int argc, char **argv) {
	if (runtime_daemon_init(argc, argv) < 0) {
		log_crit("_init(): runtime_daemon_init(): %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
}

static void _destroy(void) {
	runtime_daemon_destroy();
}

static int _loop(int argc, char **argv) {
	int ret = 0;

	_init(argc, argv);

	for (;;) {
		/* Process connections */
		if (conn_daemon_process_all() < 0) {
			log_crit("_loop(): conn_daemon_process_all(): %s", strerror(errno));
			break;
		}

		/* Check for runtime interruptions */
		if (bit_test(&rund.flags, USCHED_RUNTIME_FLAG_FLUSH)) {
			bit_clear(&rund.flags, USCHED_RUNTIME_FLAG_FLUSH);
			_serialize();
		}

		if (bit_test(&rund.flags, USCHED_RUNTIME_FLAG_FATAL)) {
			ret = 1;
			break;
		}

		if (bit_test(&rund.flags, USCHED_RUNTIME_FLAG_TERMINATE))
			break;

		if (bit_test(&rund.flags, USCHED_RUNTIME_FLAG_RELOAD)) {
			_destroy();
			_init(argc, argv);
		}
	}

	_destroy();

	return ret;
}

int main(int argc, char *argv[]) {
	return _loop(argc, argv);
}

