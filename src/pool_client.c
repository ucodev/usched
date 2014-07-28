/**
 * @file pool_client.c
 * @brief uSched
 *        Pool handlers interface - Client
 *
 * Date: 28-07-2014
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

#include <pall/cll.h>

#include "runtime.h"
#include "pool.h"
#include "entry.h"
#include "log.h"

int pool_client_init(void) {
	int errsv = 0;

	if (!(runc.epool = pall_fifo_init(&entry_destroy, NULL, NULL))) {
		errsv = errno;
		log_crit("pool_client_init(): runc.epool = pall_fifo_init(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	return 0;
}

void pool_client_destroy(void) {
	if (runc.epool) {
		pall_fifo_destroy(runc.epool);
		runc.epool = NULL;
	}
}

