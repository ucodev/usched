/**
 * @file pmq.c
 * @brief uSched
 *        POSIX Message Queueing interface
 *
 * Date: 12-07-2014
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


#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <mqueue.h>

#include <sys/stat.h>

#include "config.h"
#include "runtime.h"
#include "log.h"
#include "pmq.h"

int pmq_daemon_init(void) {
	int errsv = 0;
	struct mq_attr mqattr = {
		0,				/* Flags */
		CONFIG_USCHED_PMQ_MSG_MAX,	/* Max number of messagees on queue */
		CONFIG_USCHED_PMQ_MSG_SIZE,	/* Max message size in bytes */
		0				/* Number of messages currently in queue */
	};

	if ((rund.pmqd = mq_open(CONFIG_USCHED_PMQ_DESC_NAME, O_WRONLY | O_CREAT, 0200, &mqattr)) == (mqd_t) - 1) {
		errsv = errno;
		log_crit("pmq_daemon_init(): mq_open(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	return 0;
}

int pmq_exec_init(void) {
	int errsv = 0;
	struct mq_attr mqattr = {
		0,				/* Flags */
		CONFIG_USCHED_PMQ_MSG_MAX,	/* Max number of messagees on queue */
		CONFIG_USCHED_PMQ_MSG_SIZE,	/* Max message size in bytes */
		0				/* Number of messages currently in queue */
	};

	if ((rune.pmqd = mq_open(CONFIG_USCHED_PMQ_DESC_NAME, O_RDONLY, 0400, &mqattr)) == (mqd_t) - 1) {
		errsv = errno;
		log_crit("pmq_exec_init(): mq_open(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	return 0;
}

void pmq_daemon_destroy(void) {
	mq_close(rund.pmqd);
}

void pmq_exec_destroy(void) {
	mq_close(rune.pmqd);
}

