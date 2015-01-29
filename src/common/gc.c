/**
 * @file gc.c
 * @brief uSched
 *        Garbage Collector interface
 *
 * Date: 29-01-2015
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

#include <pall/lifo.h>

#include "mm.h"
#include "log.h"

/* Statics */
static struct lifo_handler *_gc = NULL;
static pthread_mutex_t _gc_mutex;

static void _gc_data_destroy(void *data) {
	mm_free(data);
}

/* Globals */
int gc_insert(void *data) {
	int ret = 0;

	pthread_mutex_lock(&_gc_mutex);

	ret = _gc->push(_gc, data);

	pthread_mutex_unlock(&_gc_mutex);

	return ret;
}

void gc_cleanup(void) {
	void *data = NULL;

	pthread_mutex_lock(&_gc_mutex);

	while ((data = _gc->pop(_gc)))
		_gc_data_destroy(data);

	pthread_mutex_unlock(&_gc_mutex);
}

int gc_init(void) {
	int errsv = 0;

	if (pthread_mutex_init(&_gc_mutex, NULL)) {
		errsv = errno;
		log_warn("gc_init(): pthread_mutex_init(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	pthread_mutex_lock(&_gc_mutex);

	if (!(_gc = pall_lifo_init(&_gc_data_destroy, NULL, NULL))) {
		errsv = errno;
		log_warn("gc_init(): pall_lifo_init(): %s\n", strerror(errno));
		pthread_mutex_unlock(&_gc_mutex);
		errno = errsv;
		return -1;
	}

	pthread_mutex_unlock(&_gc_mutex);

	return 0;
}

void gc_destroy(void) {
	pthread_mutex_lock(&_gc_mutex);

	/* This call will also safely destroy all the entries still present in the LIFO */
	pall_lifo_destroy(_gc);

	pthread_mutex_unlock(&_gc_mutex);

	pthread_mutex_destroy(&_gc_mutex);
}

