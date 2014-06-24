/**
 * @file thread.c
 * @brief uSched
 *        Thread handlers interface
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


#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>

#include "log.h"
#include "runtime.h"
#include "thread.h"

int thread_daemon_mutexes_init(void) {
	if (pthread_mutex_init(&rund.mutex_rpool, NULL)) {
		log_crit("thread_daemon_mutexes_init(): pthread_mutex_init(): %s\n", strerror(errno));

		return -1;
	}

	if (pthread_mutex_init(&rund.mutex_apool, NULL)) {
		log_crit("thread_daemon_mutexes_init(): pthread_mutex_init(): %s\n", strerror(errno));

		return -1;
	}

	return 0;
}

void thread_daemon_mutexes_destroy(void) {
	pthread_mutex_destroy(&rund.mutex_rpool);
	pthread_mutex_destroy(&rund.mutex_apool);
}

static void _thread_atfork_noop(void) {
	return;
}

static void _thread_atfork_prepare(void) {
	_thread_atfork_noop();
}

static void _thread_atfork_parent(void) {
	_thread_atfork_noop();
}

static void _thread_atfork_child(void) {
	_thread_atfork_noop();
}

int thread_exec_behaviour_init(void) {
	if (pthread_atfork(&_thread_atfork_prepare, &_thread_atfork_parent, &_thread_atfork_child)) {
		log_crit("thread_exec_behaviour_init(): pthread_atfork(): %s\n", strerror(errno));
		return -1;
	}

	return 0;
}

void thread_exec_behaviour_destroy(void) {
	if (pthread_atfork(&_thread_atfork_noop, &_thread_atfork_noop, &_thread_atfork_noop))
		log_crit("thread_exec_behaviour_destroy(): pthread_atfork(): %s\n", strerror(errno));
}

