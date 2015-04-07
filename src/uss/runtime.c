/**
 * @file runtime.c
 * @brief uSched
 *        Runtime handlers interface - Stat
 *
 * Date: 31-03-2015
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
#include <stdlib.h>
#include <time.h>

#include "config.h"
#include "debug.h"
#include "runtime.h"
#include "log.h"
#include "thread.h"
#include "schedule.h"
#include "sig.h"
#include "bitops.h"
#include "ipc.h"

static int _runtime_stat_drop_privs(void) {
	int errsv = 0;

	if (setregid(runs.config.stat.privdrop_gid, runs.config.stat.privdrop_gid) < 0) {
		errsv = errno;
		log_crit("_runtime_stat_drop_privs(): setregid(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	if (setreuid(runs.config.stat.privdrop_uid, runs.config.stat.privdrop_uid) < 0) {
		errsv = errno;
		log_crit("_runtime_stat_drop_privs(): setreuid(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	return 0;
}

int runtime_stat_init(int argc, char **argv) {
	int errsv = 0;

	memset(&runs, 0, sizeof(struct usched_runtime_stat));

	runs.argc = argc;
	runs.argv = argv;

	/* Initialize logging interface */
	if (log_stat_init() < 0) {
		errsv = errno;
		debug_printf(DEBUG_CRIT, "runtime_stat_init(): log_stat_init(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	log_info("Logging interface initialized.\n");

	/* Initialize configuration interface */
	log_info("Initializing configuration interface...\n");

	if (config_stat_init() < 0) {
		errsv = errno;
		log_crit("runtime_stat_init(): config_stat_init(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	log_info("Configuration interface initialized.\n");

	/* Initialize signals interface */
	log_info("Initializing signals interface...\n");

	if (sig_stat_init() < 0) {
		errsv = errno;
		log_crit("runtime_stat_init(): sig_stat_init(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	log_info("Signals interface initialized.\n");

	/* Initialize threads behaviour */
	log_info("Initializing thread behaviour interface...\n");

	if (thread_stat_behaviour_init() < 0) {
		errsv = errno;
		log_crit("runtmie_stat_init(): thread_stat_behaviour_init(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	log_info("Thread behaviour interface initialized.\n");

	/* Initialize IPC */
	log_info("Initializing IPC interface...\n");

	if (ipc_stat_init() < 0) {
		errsv = errno;
		log_crit("runtime_stat_init(): ipc_stat_init(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	log_info("IPC interface initialized.\n");

	/* Drop privileges to the configured nopriv user and group */
	log_info("Dropping process privileges...\n");

	if (_runtime_stat_drop_privs() < 0) {
		errsv = errno;
		log_crit("runtime_stat_init(): _runtime_stat_drop_privs(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	log_info("Privileges successfully dropped.\n");

	/* All good */
	log_info("All systems go. Ignition!\n");

	return 0;
}

int runtime_stat_interrupted(void) {
	if (bit_test(&runs.flags, USCHED_RUNTIME_FLAG_TERMINATE) || bit_test(&runs.flags, USCHED_RUNTIME_FLAG_RELOAD))
		return 1;

	return 0;
}

void runtime_stat_destroy(void) {
	/* Destroy IPC interface */
	log_info("Destroying IPC interface...\n");
	ipc_stat_destroy();
	log_info("IPC interface destroyed.\n");

	/* Destroy thread behaviour interface */
	log_info("Destroying thread behaviour interface...\n");
	thread_stat_behaviour_destroy();
	log_info("Thread behaviour interface destroyed.\n");

	/* Destroy signals interface */
	log_info("Destroying signals interface...\n");
	sig_stat_destroy();
	log_info("Signals interface destroyed.\n");

	log_info("All systems stopped.\n");

	/* Destroy configuration interface */
	log_info("Destroying configuration interface...\n");
	config_stat_destroy();
	log_info("Configuration interface destroyed.\n");

	/* Destroy logging interface */
	log_info("Destroying logging interface...\n");
	log_destroy();
}
