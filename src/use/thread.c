/**
 * @file thread.c
 * @brief uSched
 *        Thread handlers interface - Exec
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

int thread_exec_behaviour_init(void) {
	int errsv = 0;

	if ((errno = pthread_atfork(&thread_atfork_prepare, &thread_atfork_parent, &thread_atfork_child))) {
		errsv = errno;
		log_crit("thread_exec_behaviour_init(): pthread_atfork(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	return 0;
}

void thread_exec_behaviour_destroy(void) {
	if ((errno = pthread_atfork(&thread_atfork_noop, &thread_atfork_noop, &thread_atfork_noop)))
		log_crit("thread_exec_behaviour_destroy(): pthread_atfork(): %s\n", strerror(errno));
}

