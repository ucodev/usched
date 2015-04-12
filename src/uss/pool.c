/**
 * @file pool.c
 * @brief uSched
 *        Pool handlers interface
 *
 * Date: 12-04-2015
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
#include "stat.h"
#include "log.h"

int pool_stat_init(void) {
	int errsv = 0;

	/* Initialize dispatch pool */
	if (!(runs.dpool = pall_cll_init(&stat_compare, &stat_destroy, NULL, NULL))) {
		errsv = errno;
		log_crit("pool_stat_init(): runs.dpool = pall_cll_init(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Setup CLL: No auto search, head insert, search forward */
	(void) runs.dpool->set_config(runs.dpool, (ui32_t) (CONFIG_SEARCH_FORWARD | CONFIG_INSERT_HEAD));

	/* Initialize stat entries pool */
	if (!(runs.spool = pall_cll_init(&stat_compare, &stat_destroy, NULL, NULL))) {
		errsv = errno;
		log_crit("pool_stat_init(): runs.spool = pall_cll_init(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Setup CLL: No auto search, head insert, search forward */
	(void) runs.spool->set_config(runs.spool, (ui32_t) (CONFIG_SEARCH_FORWARD | CONFIG_INSERT_HEAD));

	/* All good */
	return 0;
}

void pool_stat_destroy(void) {
	/* Destroy dispatch pool */
	pthread_mutex_lock(&runs.mutex_dpool);

	if (runs.dpool) {
		pall_cll_destroy(runs.dpool);
		runs.dpool = NULL;
	}
	pthread_mutex_unlock(&runs.mutex_dpool);

	/* Destory stat entries pool */
	pthread_mutex_lock(&runs.mutex_spool);

	if (runs.spool) {
		pall_cll_destroy(runs.spool);
		runs.spool = NULL;
	}
	pthread_mutex_unlock(&runs.mutex_spool);
}

