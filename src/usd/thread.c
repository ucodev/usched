/**
 * @file thread.c
 * @brief uSched
 *        Thread handlers interface - Daemon
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


#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>

#include "config.h"
#include "log.h"
#include "runtime.h"
#include "thread.h"

int thread_daemon_components_init(void) {
	int errsv = 0;

	if ((errno = pthread_mutex_init(&rund.mutex_interrupt, NULL))) {
		errsv = errno;
		log_crit("thread_daemon_components_init(): pthread_mutex_init(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	if ((errno = pthread_mutex_init(&rund.mutex_rpool, NULL))) {
		errsv = errno;
		log_crit("thread_daemon_components_init(): pthread_mutex_init(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	if ((errno = pthread_mutex_init(&rund.mutex_apool, NULL))) {
		errsv = errno;
		log_crit("thread_daemon_components_init(): pthread_mutex_init(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

#if CONFIG_USCHED_SERIALIZE_ON_REQ == 1
	if ((errno = pthread_mutex_init(&rund.mutex_marshal, NULL))) {
		errsv = errno;
		log_crit("thread_daemon_components_init(): pthread_mutex_init(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	if ((errno = pthread_cond_init(&rund.cond_marshal, NULL))) {
		errsv = errno;
		log_crit("thread_daemon_components_init(): pthread_cond_init(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}
#endif
	return 0;
}

void thread_daemon_components_destroy(void) {
#if CONFIG_USCHED_SERIALIZE_ON_REQ == 1
	pthread_mutex_destroy(&rund.mutex_marshal);
	pthread_cond_destroy(&rund.cond_marshal);
#endif
	pthread_mutex_destroy(&rund.mutex_rpool);
	pthread_mutex_destroy(&rund.mutex_apool);
	pthread_mutex_destroy(&rund.mutex_interrupt);
}

