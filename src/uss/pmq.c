/**
 * @file pmq.c
 * @brief uSched
 *        POSIX Message Queueing interface - Stat
 *
 * Date: 03-04-2015
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

int pmq_stat_usd_init(void) {
#if CONFIG_USE_IPC_PMQ == 1
	int errsv = 0;
	int oflags = O_WRONLY;

	if ((runs.ipcd_usd_wo = pmq_init(runs.config.stat.ipc_name, oflags, 0, 0, 0)) == (mqd_t) -1) {
		errsv = errno;
		log_crit("pmq_stat_usd_init(): pmq_init(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	return 0;
#else
	errno = ENOSYS;
	return -1;
#endif
}

int pmq_stat_use_init(void) {
#if CONFIG_USE_IPC_PMQ == 1
	int errsv = 0;
	int oflags = O_RDONLY | O_CREAT;

	if ((runs.ipcd_use_ro = pmq_init(runs.config.exec.ipc_name, oflags, 0600, runs.config.exec.ipc_msgmax, runs.config.exec.ipc_msgsize)) == (mqd_t) -1) {
		errsv = errno;
		log_crit("pmq_stat_use_init(): pmq_init(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	return 0;
#else
	errno = ENOSYS;
	return -1;
#endif
}

void pmq_stat_usd_destroy(void) {
#if CONFIG_USE_IPC_PMQ == 1
	pmq_destroy(runs.ipcd_usd_wo);
#else
	return;
#endif
}

void pmq_stat_use_destroy(void) {
#if CONFIG_USE_IPC_PMQ == 1
	pmq_destroy(runs.ipcd_use_ro);
#else
	return;
#endif
}

