/**
 * @file pmq.c
 * @brief uSched
 *        POSIX Message Queueing interface - Daemon
 *
 * Date: 10-02-2015
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
#include <mqueue.h>

#include "config.h"
#include "runtime.h"
#include "log.h"
#include "pmq.h"

int pmq_daemon_init(void) {
	int errsv = 0;

	if ((rund.pmqd = pmq_init(rund.config.core.pmq_name, O_WRONLY | O_CREAT, 0600, rund.config.core.pmq_msgmax, rund.config.core.pmq_msgsize)) == (mqd_t) -1) {
		errsv = errno;
		log_crit("pmq_daemon_init(): pmq_init(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	return 0;
}

void pmq_daemon_destroy(void) {
	pmq_destroy(rund.pmqd);
}

