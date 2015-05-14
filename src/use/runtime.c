/**
 * @file runtime.c
 * @brief uSched
 *        Runtime handlers interface - Exec
 *
 * Date: 14-05-2015
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
#if CONFIG_USE_IPC_PMQ == 1
 #include "pmq.h"
#endif

#if CONFIG_USCHED_MULTIUSER == 0
static int _runtime_exec_drop_privs(void) {
	int errsv = 0;

	if (setregid(rune.config.core.privdrop_gid, rune.config.core.privdrop_gid) < 0) {
		errsv = errno;
		log_crit("_runtime_exec_drop_privs(): setregid(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	if (setreuid(rune.config.core.privdrop_uid, rune.config.core.privdrop_uid) < 0) {
		errsv = errno;
		log_crit("_runtime_exec_drop_privs(): setreuid(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	return 0;
}
#endif

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

	/* Initialize configuration interface */
	log_info("Initializing configuration interface...\n");

	if (config_exec_init() < 0) {
		errsv = errno;
		log_crit("runtime_exec_init(): config_exec_init(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	log_info("Configuration interface initialized.\n");

	/* Initialize signals interface */
	log_info("Initializing signals interface...\n");

	if (sig_exec_init() < 0) {
		errsv = errno;
		log_crit("runtime_exec_init(): sig_exec_init(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	log_info("Signals interface initialized.\n");

	/* Initialize thread components  */
	log_info("Initializing thread components interface...\n");

	if (thread_exec_components_init() < 0) {
		errsv = errno;
		log_crit("runtime_exec_init(): thread_exec_components_init(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	log_info("Thread components interface initialized.\n");

	/* Initialize threads behaviour */
	log_info("Initializing thread behaviour interface...\n");

	if (thread_exec_behaviour_init() < 0) {
		errsv = errno;
		log_crit("runtime_exec_init(): thread_exec_behaviour_init(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	log_info("Thread behaviour interface initialized.\n");

	/* Initialize IPC */
	log_info("Initializing IPC interface...\n");

	if (ipc_exec_init() < 0) {
		errsv = errno;
		log_crit("runtime_exec_init(): ipc_exec_init(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	log_info("IPC interface initialized.\n");

#if CONFIG_USCHED_MULTIUSER == 0
	/* If no multiuser support, drop privileges to the configured nopriv user and group */
	log_info("Dropping process privileges (no multi-user support)...\n");

	if (_runtime_exec_drop_privs() < 0) {
		errsv = errno;
		log_crit("runtime_exec_init(): _runtime_exec_drop_privs(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	log_info("Privileges successfully dropped.\n");
#endif

	/* All good */
	log_info("All systems go. Ignition!\n");

	return 0;
}

void runtime_exec_fatal(void) {
	/* We need to check if the runtime was already interrupted BEFORE setting the FATAL flag.
	 * If already interrupted, just mark it with FATAL.
	 * If not interrupted, mark it with FATAL and force the interruption.
	 */
	if (runtime_exec_interrupted()) {
		bit_set(&rune.flags, USCHED_RUNTIME_FLAG_FATAL);
	} else {
		bit_set(&rune.flags, USCHED_RUNTIME_FLAG_FATAL);
		runtime_exec_interrupt();
	}
}

void runtime_exec_interrupt(void) {
	/* Atomically set the INTERRUPT flag (if unset) and deliver the interruption signal */
	pthread_mutex_lock(&rune.mutex_interrupt);

	if (!bit_test(&rune.flags, USCHED_RUNTIME_FLAG_INTERRUPT)) {
		bit_set(&rune.flags, USCHED_RUNTIME_FLAG_INTERRUPT);

		pthread_mutex_unlock(&rune.mutex_interrupt);

		return;
	}

	pthread_mutex_unlock(&rune.mutex_interrupt);
}

int runtime_exec_interrupted(void) {
	/* FIXME: Probably it's a good idea to acquire the mutex_interrupt before testing */
	if (bit_test(&rune.flags, USCHED_RUNTIME_FLAG_TERMINATE) || bit_test(&rune.flags, USCHED_RUNTIME_FLAG_RELOAD) || bit_test(&rune.flags, USCHED_RUNTIME_FLAG_FATAL) || bit_test(&rune.flags, USCHED_RUNTIME_FLAG_INTERRUPT))
		return 1;

	return 0;
}

void runtime_exec_destroy(void) {
	/* Destroy IPC interface */
	log_info("Destroying IPC interface...\n");
	ipc_exec_destroy();
	log_info("IPC interface destroyed.\n");

	/* Destroy thread behaviour interface */
	log_info("Destroying thread behaviour interface...\n");
	thread_exec_behaviour_destroy();
	log_info("Thread behaviour interface destroyed.\n");

	/* Destroy thread components interface */
	log_info("Destroying thread components interface...\n");
	thread_exec_components_destroy();
	log_info("Thread components interface destroyed.\n");

	/* Destroy signals interface */
	log_info("Destroying signals interface...\n");
	sig_exec_destroy();
	log_info("Signals interface destroyed.\n");

	log_info("All systems stopped.\n");

	/* Destroy configuration interface */
	log_info("Destroying configuration interface...\n");
	config_exec_destroy();
	log_info("Configuration interface destroyed.\n");

	/* Destroy logging interface */
	log_info("Destroying logging interface...\n");
	log_destroy();
}

void runtime_exec_quiet_destroy(void) {
	/* NOTE: this destructor is _ONLY_ used by child processes invoked by uSched Executer */
	ipc_exec_destroy();
	thread_exec_behaviour_destroy();
	thread_exec_components_destroy();
	sig_exec_destroy();
	config_exec_destroy();

	/* NOTE:
	 *
	 * closelog() isn't required here as the execpl() call will automatically close the logging
	 * interface.
	 *
	 * FIXME: Check if this automatic close isn't dangerous (eg, it doesn't internally call the
	 * closelog() as this will be UB due to the AS-Unsafe of that function.
	 *
	 */

	/* FIXME (AS-Unsafe): log_destroy(); */
}

