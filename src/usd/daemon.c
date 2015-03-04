/**
 * @file daemon.c
 * @brief uSched
 *        Daemon Main Component
 *
 * Date: 04-03-2015
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
#include <pthread.h>

#include "config.h"
#include "debug.h"
#include "bitops.h"
#include "runtime.h"
#include "marshal.h"
#include "log.h"
#include "conn.h"

static void _flush(void) {
	pthread_mutex_lock(&rund.mutex_marshal);

	bit_set(&rund.flags, USCHED_RUNTIME_FLAG_SERIALIZE);

	pthread_cond_signal(&rund.cond_marshal);

	pthread_mutex_unlock(&rund.mutex_marshal);
}

static void _init(int argc, char **argv) {
	if (runtime_daemon_init(argc, argv) < 0) {
		log_crit("_init(): runtime_daemon_init(): %s\n", strerror(errno));
		exit(PROCESS_EXIT_STATUS_CUSTOM_BAD_RUNTIME_OR_CONFIG);
	}
}

static void _destroy(void) {
	runtime_daemon_destroy();
}

static int _loop(int argc, char **argv) {
	int ret = 0;

	_init(argc, argv);

	for (;;) {
		/* Runtime can be canceled from this point on... */
		pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, (int [1]) { 0 });

		/* Process connections */
		if (conn_daemon_process_all() < 0) {
			log_crit("_loop(): conn_daemon_process_all(): %s", strerror(errno));
			break;
		}

		/* Runtime can't be canceled from this point on... */
		pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, (int [1]) { 0 });

		/* Check for runtime interruptions */
		if (bit_test(&rund.flags, USCHED_RUNTIME_FLAG_FLUSH)) {
			bit_clear(&rund.flags, USCHED_RUNTIME_FLAG_FLUSH);
			_flush();
		}

		if (bit_test(&rund.flags, USCHED_RUNTIME_FLAG_FATAL)) {
			ret = 1;
			break;
		}

		if (bit_test(&rund.flags, USCHED_RUNTIME_FLAG_TERMINATE))
			break;

		if (bit_test(&rund.flags, USCHED_RUNTIME_FLAG_RELOAD)) {
#if CONFIG_USCHED_DROP_PRIVS == 0
			_destroy();
			_init(argc, argv);
#else
			/* NOTE: If privilege drop is enabled, conventional reload will fail to load
			 * files owned and readable only by root, so we need to terminate the
			 * execution of the daemon and wait for uSched Monitor (usm) to restart the
			 * process.
			 */
			ret = PROCESS_EXIT_STATUS_CUSTOM_RELOAD_NOPRIV;
			break;
#endif
		}

		/* Clear interrupt flag if we reach this point */
		if (bit_test(&rund.flags, USCHED_RUNTIME_FLAG_INTERRUPT)) {
			pthread_mutex_lock(&rund.mutex_interrupt);
			bit_clear(&rund.flags, USCHED_RUNTIME_FLAG_INTERRUPT);
			pthread_mutex_unlock(&rund.mutex_interrupt);
		}
	}

	_destroy();

	return ret;
}

int main(int argc, char *argv[]) {
	return _loop(argc, argv);
}

