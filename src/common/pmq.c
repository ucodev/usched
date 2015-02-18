/**
 * @file pmq.c
 * @brief uSched
 *        POSIX Message Queueing interface
 *
 * Date: 18-02-2015
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
#include <mqueue.h>

#include <sys/stat.h>

#include "config.h"
#include "runtime.h"
#include "log.h"
#include "pmq.h"

mqd_t pmq_init(const char *name, int oflags, mode_t mode, unsigned int maxmsg, unsigned int msgsize) {
	int errsv = 0;
	mqd_t ret;
	struct mq_attr mqattr;

	memset(&ret, 0, sizeof(mqd_t));

	if (oflags & O_CREAT) {
		memset(&mqattr, 0, sizeof(struct mq_attr));

		mqattr.mq_flags = 0;		/* Flags */
		mqattr.mq_maxmsg = maxmsg;	/* Max number of messagees on queue */
		mqattr.mq_msgsize = msgsize;	/* Max message size in bytes */
		mqattr.mq_curmsgs = 0;		/* Number of messages currently in queue */

		/* Try to unlink any previous message queue */
		if (mq_unlink(name) < 0) {
			if (errno != ENOENT) {
				errsv = errno;
				log_crit("pmq_init(): mq_unlink(): %s\n", strerror(errno));
				errno = errsv;
				return -1;
			}
		}
	}

	if (oflags & O_CREAT) {
#if !defined(CONFIG_SYS_BSD) || CONFIG_SYS_BSD == 0
		if ((ret = mq_open(name, oflags, mode, &mqattr)) == (mqd_t) -1) {
#else
		if ((ret = mq_open(name, oflags, mode, NULL)) == (mqd_t) -1) {
#endif
			errsv = errno;
			log_crit("pmq_init(): mq_open(): %s\n", strerror(errno));
			errno = errsv;
			return (mqd_t) -1;
		}
	} else if ((ret = mq_open(name, oflags)) == (mqd_t) -1) {
		errsv = errno;
		log_crit("pmq_init(): mq_open(): %s\n", strerror(errno));
		errno = errsv;
		return (mqd_t) -1;
	}

#if CONFIG_SYS_BSD == 1
	if ((oflags & O_CREAT) && mq_setattr(ret, &mqattr, NULL) < 0) {
		errsv = errno;
		log_crit("pmq_init(): mq_setattr(): %s\n", strerror(errno));
		errno = errsv;
		return (mqd_t) - 1;
	}
#endif

	return ret;
}

void pmq_destroy(mqd_t pmqd) {
	mq_close(pmqd);
}

int pmq_unlink(const char *pmqname) {
	return mq_unlink(pmqname);
}

