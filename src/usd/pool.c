/**
 * @file pool.c
 * @brief uSched
 *        Pool handlers interface
 *
 * Date: 09-02-2015
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

#include <pall/cll.h>

#include "runtime.h"
#include "pool.h"
#include "entry.h"
#include "log.h"

int pool_daemon_init(void) {
	int errsv = 0;

	/* Initialize active scheduling entries pool */
	if (!(rund.apool = pall_cll_init(&entry_compare, &entry_destroy, &entry_daemon_serialize, &entry_daemon_unserialize))) {
		errsv = errno;
		log_crit("pool_daemon_init(): rund.apool = pall_cll_init(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Setup CLL: No auto search, head insert, search forward */
	rund.apool->set_config(rund.apool, CONFIG_SEARCH_FORWARD | CONFIG_INSERT_HEAD);

	/* Initialize connection pool */
	if (!(rund.rpool = pall_cll_init(&entry_compare, &entry_destroy, &entry_daemon_serialize, &entry_daemon_unserialize))) {
		errsv = errno;
		log_crit("pool_daemon_init(): rund.rpool = pall_cll_init(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Setup CLL: No auto search, head insert, search forward */
	rund.rpool->set_config(rund.rpool, CONFIG_SEARCH_FORWARD | CONFIG_INSERT_HEAD);

	return 0;
}

void pool_daemon_destroy(void) {
	/* FIXME: Check if there are active notification threads. If so, we need to wait until
	 * all the notification threads finish execution before destroyed the active pool...
	 */

	pthread_mutex_lock(&rund.mutex_apool);
	if (rund.apool) {
		pall_cll_destroy(rund.apool);
		rund.apool = NULL;
	}
	pthread_mutex_unlock(&rund.mutex_apool);

	/* TODO: Evaluate if there isn't a risk of after active pool is destroyed, some entries that
	 * may be shared with the remote connections pool have become invalid.
	 */
	pthread_mutex_lock(&rund.mutex_rpool);
	if (rund.rpool) {
		pall_cll_destroy(rund.rpool);
		rund.rpool = NULL;
	}
	pthread_mutex_unlock(&rund.mutex_rpool);
}

