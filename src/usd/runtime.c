/**
 * @file runtime.c
 * @brief uSched
 *        Runtime handlers interface - Daemon
 *
 * Date: 19-03-2015
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
#include <unistd.h>

#include <pall/cll.h>

#include "config.h"
#include "debug.h"
#include "runtime.h"
#include "mm.h"
#include "conn.h"
#include "log.h"
#include "thread.h"
#include "pool.h"
#include "schedule.h"
#include "ipc.h"
#include "sig.h"
#include "bitops.h"
#include "marshal.h"
#include "gc.h"
#include "delta.h"

#if CONFIG_USCHED_JAIL == 1
static int _runtime_daemon_jail(void) {
	int errsv = 0;

	if (chdir(rund.config.core.jail_dir) < 0) {
		errsv = errno;
		log_crit("_runtime_daemon_jail(): chdir(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	if (chroot(rund.config.core.jail_dir) < 0) {
		errsv = errno;
		log_crit("_runtime_daemon_jail(): chroot(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	return 0;
}
#endif

#if CONFIG_USCHED_DROP_PRIVS == 1
static int _runtime_daemon_drop_privs(void) {
	int errsv = 0;

	if (setregid(rund.config.core.privdrop_gid, rund.config.core.privdrop_gid) < 0) {
		errsv = errno;
		log_crit("_runtime_daemon_drop_privs(): setregid(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	if (setreuid(rund.config.core.privdrop_uid, rund.config.core.privdrop_uid) < 0) {
		errsv = errno;
		log_crit("_runtime_daemon_drop_privs(): setreuid(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	return 0;
}
#endif

int runtime_daemon_init(int argc, char **argv) {
	int errsv = 0;

	memset(&rund, 0, sizeof(struct usched_runtime_daemon));

	rund.argc = argc;
	rund.argv = argv;

	rund.pid = getpid();
	rund.t_runtime = pthread_self();

	/* Initialize logging interface */
	if (log_daemon_init() < 0) {
		errsv = errno;
		debug_printf(DEBUG_CRIT, "runtime_daemon_init(): log_daemon_init(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	log_info("Logging interface initialized.\n");

	/* Initialize configuration interface */
	log_info("Initializing configuration interface...\n");

	if (config_daemon_init() < 0) {
		errsv = errno;
		log_crit("runtime_daemon_init(): config_daemon_init(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	log_info("Configuration interface initialized.\n");

	/* Initialize garbage collector interface */
	log_info("Initializing garbage collector interface...\n");

	if (gc_init() < 0) {
		errsv = errno;
		log_crit("runtime_daemon_init(): gc_init(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	log_info("Garbage collector interface initialized.\n");

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

	if (ipc_daemon_init() < 0) {
		errsv = errno;
		log_crit("runtime_daemon_init(): ipc_daemon_init(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	log_info("IPC interface initialized.\n");

	/* Initialize thread components (mutexes, conditions, ...)  */
	log_info("Initializing thread components...\n");

	if (thread_daemon_components_init() < 0) {
		errsv = errno;
		log_crit("runtime_daemon_init(): thread_components_init(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	log_info("Thread components initialized.\n");

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

#if CONFIG_USCHED_DROP_PRIVS == 1
	log_info("Backing up the current serialization file...\n");

	if (marshal_daemon_backup() < 0) {
		errsv = errno;
		log_crit("runtime_daemon_init(): marshal_daemon_backup(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	log_info("Serialization file backed up.\n");
#endif

	/* Unserialize data, if any */
	log_info("Unserializing active pools...\n");

	if (marshal_daemon_unserialize_pools() < 0) {
#if CONFIG_USCHED_DROP_PRIVS == 0
		log_info("Unable to unserialize active pools.\n");

		log_info("Backing up the current serialization file...\n");

		if (marshal_daemon_backup() < 0) {
			log_info("runtime_daemon_init(): marshal_daemon_backup(): %s\n", strerror(errno));
		} else {
			log_info("Serialization file backed up. Manual recovery is required to restore the previous uSched Daemon state.\n");
		}
#else
		errsv = errno;
		log_crit("runtime_daemon_init(): marshal_daemon_unserialize_pools(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
#endif
	} else {
		log_info("Active pools unserialized.\n");
	}

	/* Re-initialize marshal interface, wiping the current contents */
	log_info("Re-initializing marshal interface...\n");

	marshal_daemon_destroy();

	if (marshal_daemon_init() < 0) {
		errsv = errno;
		log_crit("runtime_daemon_init(): marshal_daemon_init(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	log_info("Marshal interface initialized.\n");

	/* Initialize marshal monitor */
	log_info("Initializing marshal monitor...\n");

	if (marshal_daemon_monitor_init() < 0) {
		errsv = errno;
		log_crit("runtime_daemon_init(): marshal_daemon_monitor_init(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	log_info("Marshal monitor initialized.\n");

	/* Initialize delta T monitor */
	log_info("Initializing delta time monitor...\n");

	if (delta_daemon_time_init() < 0) {
		errsv = errno;
		log_crit("runtime_daemon_init(): delta_daemon_time_init(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	log_info("Delta time monitor initialized.\n");

	/* Initialize connections interface */
	log_info("Initializing connections interface...\n");

	if (conn_daemon_init() < 0) {
		errsv = errno;
		log_crit("runtime_daemon_init(): conn_daemon_init(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	log_info("Connections interface initialized.\n");

#if CONFIG_USCHED_JAIL == 1
	/* Jail process */
	log_info("Jailing process to %s...\n", rund.config.core.jail_dir);

	if (_runtime_daemon_jail() < 0) {
		errsv = errno;
		log_crit("runtime_daemon_init(): _runtime_daemon_jail(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	log_info("Process successfully jailed.\n");
#endif

#if CONFIG_USCHED_DROP_PRIVS == 1
	/* Drop process privileges */
	log_info("Dropping process privileges...\n");

	if (_runtime_daemon_drop_privs() < 0) {
		errsv = errno;
		log_crit("runtime_daemon_init(): _runtime_daemon_drop_privs(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	log_info("Privileges successfully dropped.\n");
#endif

	/* All good */
	log_info("All systems go. Ignition!\n");

	return 0;
}

void runtime_daemon_fatal(void) {
	/* We need to check if the runtime was already interrupted BEFORE setting the FATAL flag.
	 * If already interrupted, just mark it with FATAL.
	 * If not interrupted, mark it with FATAL and force the interruption.
	 */
	if (runtime_daemon_interrupted()) {
		bit_set(&rund.flags, USCHED_RUNTIME_FLAG_FATAL);
	} else {
		bit_set(&rund.flags, USCHED_RUNTIME_FLAG_FATAL);
		runtime_daemon_interrupt();
	}
}

void runtime_daemon_interrupt(void) {
	/* Atomically set the SIGNALED flag (if unset) and deliver the interruption signal */
	pthread_mutex_lock(&rund.mutex_interrupt);

	if (!bit_test(&rund.flags, USCHED_RUNTIME_FLAG_INTERRUPT)) {
		bit_set(&rund.flags, USCHED_RUNTIME_FLAG_INTERRUPT);

		pthread_mutex_unlock(&rund.mutex_interrupt);

		pthread_cancel(rund.t_unix);
		pthread_cancel(rund.t_remote);

		return;
	}

	pthread_mutex_unlock(&rund.mutex_interrupt);
}

int runtime_daemon_terminated(void) {
	if (bit_test(&rund.flags, USCHED_RUNTIME_FLAG_TERMINATE) || bit_test(&rund.flags, USCHED_RUNTIME_FLAG_RELOAD) || bit_test(&rund.flags, USCHED_RUNTIME_FLAG_FATAL))
		return 1;

	return 0;
}

int runtime_daemon_interrupted(void) {
	if (bit_test(&rund.flags, USCHED_RUNTIME_FLAG_TERMINATE) || bit_test(&rund.flags, USCHED_RUNTIME_FLAG_RELOAD) || bit_test(&rund.flags, USCHED_RUNTIME_FLAG_FATAL) || bit_test(&rund.flags, USCHED_RUNTIME_FLAG_FLUSH) || bit_test(&rund.flags, USCHED_RUNTIME_FLAG_INTERRUPT))
		return 1;

	return 0;
}

void runtime_daemon_destroy(void) {
	/* Destroy connections interface */
	log_info("Destroying connections interface...\n");
	conn_daemon_destroy();
	log_info("Connections interface destroyed.\n");

	/* Destroy delta T monitor */
	log_info("Destroying delta time monitor...\n");
	delta_daemon_time_destroy();
	log_info("Delta time monitor destroyed.\n");

	/* Serialize data and destroy marshal monitor */
	log_info("Destroying marshal monitor...\n");
	marshal_daemon_monitor_destroy();
	log_info("Marshal monitor destroyed.\n");

	/* Destroy scheduling interface */
	log_info("Destroying scheduling interface...\n");
	schedule_daemon_destroy();
	log_info("Scheduling interface destroyed.\n");

#if CONFIG_USCHED_SERIALIZE_ON_REQ == 0
	log_info("Serializing active pools...\n");

	if (marshal_daemon_serialize_pools() < 0) {
		log_crit("marshal_daemon_serialize_pools(): %s\n", strerror(errno)):
	} else {
		log_info("Active pools serialized.\n");
	}
#endif

	/* Destroy marshal interface */
	log_info("Destroying marshal interface...\n");
	marshal_daemon_destroy();
	log_info("Marshal interface destroyed.\n");

	/* Destroy pools */
	log_info("Destroying pools...\n");
	pool_daemon_destroy();
	log_info("Pools destroyed.\n");

	/* Destroy thread components */
	log_info("Destroying thread components...\n");
	thread_daemon_components_destroy();
	log_info("Thread components destroyed.\n");

	/* Destroy IPC interface */
	log_info("Destroying IPC interface...\n");
	ipc_daemon_destroy();
	log_info("IPC interface destroyed.\n");

	/* Destroy signals interface */
	log_info("Destroying signals interface...\n");
	sig_daemon_destroy();
	log_info("Signals interface destroyed.\n");

	/* Destroy garbage collector interface */
	log_info("Destroying garbage collector interface...\n");
	gc_destroy();
	log_info("Garbage collector interface destroyed.\n");

	/* Destroy configuration interface */
	log_info("Destroying configuration interface...\n");
	config_daemon_destroy();
	log_info("Configuration interface destroyed.\n");

	log_info("All systems stopped.\n");

	/* Destroy logging interface */
	log_info("Destroying logging interface...\n");
	log_destroy();
}

