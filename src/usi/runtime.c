/**
 * @file runtime.c
 * @brief uSched
 *        Runtime handlers interface - IPC
 *
 * Date: 13-05-2015
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
#include "pool.h"

#if CONFIG_USCHED_DROP_PRIVS == 1
static int _runtime_ipc_drop_group_privs(void) {
	int errsv = 0;

	debug_printf(DEBUG_INFO, "PROCESS GID: %u\n", runi.config.ipc.privdrop_gid);

	if (setregid(runi.config.ipc.privdrop_gid, runi.config.ipc.privdrop_gid) < 0) {
		errsv = errno;
		log_crit("_runtime_ipc_drop_group_privs(): setregid(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	return 0;
}

static int _runtime_ipc_drop_user_privs(void) {
	int errsv = 0;

	debug_printf(DEBUG_INFO, "PROCESS UID: %u\n", runi.config.ipc.privdrop_uid);

	if (setreuid(runi.config.ipc.privdrop_uid, runi.config.ipc.privdrop_uid) < 0) {
		errsv = errno;
		log_crit("_runtime_ipc_drop_user_privs(): setreuid(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	return 0;
}
#endif

int runtime_ipc_init(int argc, char **argv) {
	int errsv = 0;

	memset(&runi, 0, sizeof(struct usched_runtime_ipc));

	runi.argc = argc;
	runi.argv = argv;

	/* Initialize logging interface */
	if (log_ipc_init() < 0) {
		errsv = errno;
		debug_printf(DEBUG_CRIT, "runtime_ipc_init(): log_ipc_init(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	log_info("Logging interface initialized.\n");

	/* Initialize configuration interface */
	log_info("Initializing configuration interface...\n");

	if (config_ipc_init() < 0) {
		errsv = errno;
		log_crit("runtime_ipc_init(): config_ipc_init(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	log_info("Configuration interface initialized.\n");

	/* Initialize signals interface */
	log_info("Initializing signals interface...\n");

	if (sig_ipc_init() < 0) {
		errsv = errno;
		log_crit("runtime_ipc_init(): sig_ipc_init(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	log_info("Signals interface initialized.\n");

	/* Initialize threads behaviour */
	log_info("Initializing thread components interface...\n");

	if (thread_ipc_components_init() < 0) {
		errsv = errno;
		log_crit("runtime_ipc_init(): thread_ipc_components_init(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	log_info("Thread components interface initialized.\n");

#if CONFIG_USCHED_DROP_PRIVS == 1
	/* NOTE: Drop group privileges BEFORE initializing IPC interface, so the IPC system will
	 * belong to the same group.
	 */

	/* Drop privileges to the configured nopriv group */
	log_info("Dropping group process privileges...\n");

	if (_runtime_ipc_drop_group_privs() < 0) {
		errsv = errno;
		log_crit("runtime_ipc_init(): _runtime_ipc_drop_group_privs(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	log_info("Group privileges successfully dropped.\n");
#endif

	/* Initialize IPC */
	log_info("Initializing IPC interface...\n");

	if (ipc_ipc_init() < 0) {
		errsv = errno;
		log_crit("runtime_ipc_init(): ipc_init(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	log_info("IPC interface initialized.\n");

#if CONFIG_USCHED_DROP_PRIVS == 1
	/* Drop privileges to the configured nopriv user */
	log_info("Dropping user process privileges...\n");

	if (_runtime_ipc_drop_user_privs() < 0) {
		errsv = errno;
		log_crit("runtime_ipc_init(): _runtime_ipc_drop_user_privs(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	log_info("User privileges successfully dropped.\n");
#endif

	/* All good */
	log_info("All systems go. Ignition!\n");

	return 0;
}

int runtime_ipc_interrupted(void) {
	if (bit_test(&runi.flags, USCHED_RUNTIME_FLAG_TERMINATE) || bit_test(&runi.flags, USCHED_RUNTIME_FLAG_RELOAD))
		return 1;

	return 0;
}

void runtime_ipc_destroy(void) {
	/* Destroy IPC interface */
	log_info("Destroying IPC interface...\n");
	ipc_ipc_destroy();
	log_info("IPC interface destroyed.\n");

	/* Destroy thread behaviour interface */
	log_info("Destroying thread components interface...\n");
	thread_ipc_components_destroy();
	log_info("Thread components interface destroyed.\n");

	/* Destroy signals interface */
	log_info("Destroying signals interface...\n");
	sig_ipc_destroy();
	log_info("Signals interface destroyed.\n");

	log_info("All systems stopped.\n");

	/* Destroy configuration interface */
	log_info("Destroying configuration interface...\n");
	config_ipc_destroy();
	log_info("Configuration interface destroyed.\n");

	/* Destroy logging interface */
	log_info("Destroying logging interface...\n");
	log_destroy();
}

