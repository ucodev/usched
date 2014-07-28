/**
 * @file runtime_daemon.c
 * @brief uSched
 *        Runtime handlers interface - Daemon
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
#include "sig.h"
#include "bitops.h"
#include "marshal.h"


int runtime_daemon_init(int argc, char **argv) {
	int errsv = 0;

	memset(&rund, 0, sizeof(struct usched_runtime_daemon));

	rund.argc = argc;
	rund.argv = argv;

	/* Initialize logging interface */
	if (log_daemon_init() < 0) {
		errsv = errno;
		debug_printf(DEBUG_CRIT, "runtime_daemon_init(): log_daemon_init(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	log_info("Logging interface initialized.\n");

	/* Initialize signals interface */
	log_info("Initializing signals interface...\n");

	if (sig_daemon_init() < 0) {
		errsv = errno;
		log_crit("runtime_daemon_init(): sig_daemon_init(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	log_info("Signals interface initialized.\n");

	/* Initialize IPC */
	log_info("Initializing IPC interface...\n");

	if (pmq_daemon_init() < 0) {
		errsv = errno;
		log_crit("runtime_daemon_init(): pmq_daemon_init(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	log_info("IPC interface initialized.\n");

	/* Initialize mutexes */
	log_info("Initializing thread mutexes...\n");

	if (thread_daemon_mutexes_init() < 0) {
		errsv = errno;
		log_crit("runtime_daemon_init(): thread_mutexes_init(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	log_info("Thread mutexes initialized.\n");

	/* Initialize pools */
	log_info("Initializing pools...\n");

	if (pool_daemon_init() < 0) {
		errsv = errno;
		log_crit("runtime_daemon_init(): pool_daemon_init(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	log_info("Pools initialized.\n");

	/* Initialize scheduling interface */
	log_info("Initializing scheduling interface...\n");

	if (schedule_daemon_init() < 0) {
		errsv = errno;
		log_crit("runtime_daemon_init(): schedule_daemon_init(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	log_info("Scheduling interface initialized.\n");

	/* Initialize marshal interface */
	log_info("Initializing marshal interface...\n");

	if (marshal_daemon_init() < 0) {
		errsv = errno;
		log_crit("runtime_daemon_init(): marshal_daemon_init(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	log_info("Marshal interface initialized.\n");

	/* Unserialize data, if any */
	log_info("Unserializing active pools...\n");

	if (marshal_daemon_unserialize_pools() < 0) {
		log_info("Unable to unserialize active pools.\n");
		log_info("Backing up the current serialization file...\n");

		if (marshal_daemon_backup() < 0) {
			log_info("runtime_daemon_init(): marshal_daemon_backup(): %s\n", strerror(errno));
		} else {
			log_info("Serialization file backed up.\n");
		}
	} else {
		log_info("Active pools unserialized.\n");
	}

	/* Re-initialize marshal interface, wiping the current contents */
	log_info("Re-initializing marshal interface...\n");

	marshal_daemon_destroy();

	marshal_daemon_wipe();

	if (marshal_daemon_init() < 0) {
		errsv = errno;
		log_crit("runtime_daemon_init(): marshal_daemon_init(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	log_info("Marshal interface initialized.\n");

	/* Initialize connections interface */
	log_info("Initializing connections interface...\n");

	if (conn_daemon_init() < 0) {
		errsv = errno;
		log_crit("runtime_daemon_init(): conn_daemon_init(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	log_info("Connections interface initialized.\n");

	/* All good */
	log_info("All systems go. Ignition!\n");

	return 0;
}

int runtime_daemon_interrupted(void) {
	if (bit_test(&rund.flags, USCHED_RUNTIME_FLAG_TERMINATE) || bit_test(&rund.flags, USCHED_RUNTIME_FLAG_RELOAD))
		return 1;

	return 0;
}

void runtime_daemon_destroy(void) {
	/* Destroy connections interface */
	log_info("Destroying connections interface...\n");
	conn_daemon_destroy();
	log_info("Connections interface destroyed.\n");

	/* Destroy scheduling interface */
	log_info("Destroying scheduling interface...\n");
	schedule_daemon_destroy();
	log_info("Scheduling interface destroyed.\n");

	/* Serialize active pools before destroying them */
	log_info("Serializing active pools...\n");
	if (marshal_daemon_serialize_pools() < 0) {
		log_crit("runtime_daemon_destroy(): marshal_daemon_serialize_pools(): %s\n", strerror(errno));
	} else {
		log_info("Active pools serialized.\n");
	}

	marshal_daemon_destroy();

	/* Destroy pools */
	log_info("Destroying pools...\n");
	pool_daemon_destroy();
	log_info("Pools destroyed.\n");

	/* Destroy mutexes */
	log_info("Destroying thread mutexes...\n");
	thread_daemon_mutexes_destroy();
	log_info("Thread mutexes destroyed.\n");

	/* Destroy IPC interface */
	log_info("Destroying IPC interface...\n");
	pmq_daemon_destroy();
	log_info("IPC interface destroyed.\n");

	/* Destroy signals interface */
	log_info("Destroying signals interface...\n");
	sig_daemon_destroy();
	log_info("Signals interface destroyed.\n");

	log_info("All systems stopped.\n");

	/* Destroy logging interface */
	log_info("Destroying logging interface...\n");
	log_destroy();
}

