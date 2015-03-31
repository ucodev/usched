/**
 * @file pmq.c
 * @brief uSched
 *        POSIX Message Queueing interface - Stat
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


#include <string.h>
#include <errno.h>
#include <fcntl.h>

#include <sys/stat.h>

#if CONFIG_USE_IPC_PMQ == 1
 #include <mqueue.h>
#endif

#include "config.h"
#include "runtime.h"
#include "log.h"
#include "pmq.h"

int pmq_stat_init(void) {
#if CONFIG_USE_IPC_PMQ == 1
	int errsv = 0;

	if ((runs.ipcd = pmq_init(runs.config.stat.ipc_name, O_RDONLY, 0, 0, 0)) == (mqd_t) -1) {
		errsv = errno;
		log_crit("pmq_stat_init(): pmq_init(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	return 0;
#else
	errno = ENOSYS;
	return -1;
#endif
}

void pmq_stat_destroy(void) {
#if CONFIG_USE_IPC_PMQ == 1
	pmq_destroy(runs.ipcd);
#else
	return;
#endif
}

