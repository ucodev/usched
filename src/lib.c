/**
 * @file lib.c
 * @brief uSched
 *        uSched Client Library interface
 *
 * Date: 25-06-2014
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

#include "runtime.h"
#include "conn.h"

static int _init(const char *req) {
	if (runtime_client_lib_init(req) < 0)
		return -1;

	return 0;
}

static int _do(void) {
	if (conn_client_process() < 0)
		return -1;

	return 0;
}

static void _destroy(void) {
	runtime_client_lib_destroy();
}

int usched_request(const char *req) {
	if (_init(req) < 0)
		return -1;

	if (_do() < 0)
		return -1;

	return 0;
}

usched_usage_client_err_t usched_error(void) {
	/* TODO: Not yet implemented */
	errno = ENOSYS;

	return -1;
}

void usched_destroy(void) {
	_destroy();
}

