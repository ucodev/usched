/**
 * @file pmq.c
 * @brief uSched
 *        POSIX Message Queueing interface - Admin
 *
 * Date: 21-02-2015
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

int pmq_admin_create(void) {
	int errsv = 0;
	int oflags = O_RDWR | O_CREAT;
	mqd_t pmqd;

	/* (Re)create the message queue */
	if ((pmqd = pmq_init(runa.config.core.pmq_name, oflags, 0700, runa.config.core.pmq_msgmax, runa.config.core.pmq_msgsize)) == (mqd_t) -1) {
		errsv = errno;
		log_crit("pmq_admin_create(): pmq_init(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Close the descriptor */
	pmq_destroy(pmqd);

	/* All good */
	return 0;
}

int pmq_admin_delete(void) {
	return pmq_unlink(runa.config.core.pmq_name);
}

