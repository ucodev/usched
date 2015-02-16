/**
 * @file delta.c
 * @brief uSched
 *        Delta T interface - Daemon
 *
 * Date: 16-02-2015
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
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>

#include "config.h"
#include "bitops.h"
#include "runtime.h"
#include "log.h"
#include "delta.h"

static void *_delta_time_monitor(void *arg) {
	for (;;) {
		/* Check if the daemon was interrupted */
		if (runtime_daemon_interrupted())
			break;

		/* Compute delta time */
		rund.delta_last = time(NULL) - rund.time_last - CONFIG_USCHED_DELTA_CHECK_INTERVAL;

		/* Check if the absolute time variation value exceeds the acceptable limits */
		if (abs(rund.delta_last) >= rund.config.core.delta_reload) {
			log_warn("delta_time_monitor(): System time change detected. Reloading daemon...\n");

			/* Time was changed, we need to reload the daemon */
			bit_set(&rund.flags, USCHED_RUNTIME_FLAG_RELOAD);

			/* Interrupt daemon execution */
			runtime_daemon_interrupt();

			/* This worker has nothing else to do */
			break;
		}

		/* Update last time reference */
		rund.time_last = time(NULL);

		/* Wait until next check */
		usleep(CONFIG_USCHED_DELTA_CHECK_INTERVAL * 1000000);
	}

	log_info("delta_time_monitor(): Thread exiting...\n");

	/* All good */
	pthread_exit(NULL);

	return NULL;
}

int delta_time_init(void) {
	int errsv = 0;

	/* Set the last known time reference */
	rund.time_last = time(NULL);

	/* Set the last known time variation */
	rund.delta_last = 0;

	/* Create a delta time monitor worker */
	if (pthread_create(&rund.t_delta, NULL, &_delta_time_monitor, NULL)) {
		errsv = errno;
		log_warn("delta_time_init(): pthread_create(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* All good */
	return 0;
}

void delta_time_destroy(void) {
	pthread_cancel(rund.t_delta);

	pthread_join(rund.t_delta, NULL);
}

