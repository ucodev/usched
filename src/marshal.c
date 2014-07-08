/**
 * @file marshal.c
 * @brief uSched
 *        Serialization / Unserialization interface
 *
 * Date: 07-07-2014
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
#include <time.h>
#include <pthread.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <psched/sched.h>

#include "config.h"
#include "runtime.h"
#include "marshal.h"
#include "log.h"
#include "entry.h"

int marshal_daemon_init(void) {
	int errsv = 0;

	if ((rund.ser_fd = open(CONFIG_USCHED_FILE_DAEMON_SERIALIZE, O_CREAT | O_SYNC | O_RDWR, S_IRUSR | S_IWUSR)) < 0) {
		errsv = errno;
		log_warn("marshal_daemon_init(): open(\"%s\", ...): %s\n", CONFIG_USCHED_FILE_DAEMON_SERIALIZE, strerror(errno));
		errno = errsv;
		return -1;
	}

	return 0;
}

int marshal_daemon_serialize_pools(void) {
	int ret = -1;

	pthread_mutex_lock(&rund.mutex_apool);

	/* NOTE: All entries present in the active pool shall be already disarmed (granted by schedule_daemon_destroy()) */

	/* Serialize the active pool */
	if ((ret = rund.apool->serialize(rund.apool, rund.ser_fd)) < 0)
		log_warn("marshal_daemon_serialize_pools(): rund.apool->serialize(): %s\n", strerror(errno));

	pthread_mutex_unlock(&rund.mutex_apool);

	return ret;
}

int marshal_daemon_unserialize_pools(void) {
	int ret = -1;
	struct usched_entry *entry = NULL;
	time_t t = time(NULL);

	pthread_mutex_lock(&rund.mutex_apool);

	/* Unserialize active pool */
	if ((ret = rund.apool->unserialize(rund.apool, rund.ser_fd)) < 0) {
		log_warn("marshal_daemon_unserialize_pools(): rund.apool->unserialize(): %s\n", strerror(errno));
		goto _unserialize_finish;
	}

	/* Activate all the unserialized entries through libpsched */
	for (rund.apool->rewind(rund.apool, 0); (entry = rund.apool->iterate(rund.apool)); ) {
		/* Update the trigger value based on step and current time */
		while (entry->step && (entry->trigger < t))
			entry->trigger += entry->step;

		/* Check if the trigger remains valid, i.e., does not exceed the expiration time */
		if (entry->expire && (entry->trigger > entry->expire)) {
			log_info("marshal_daemon_unserialize_pools(): An entry is expired (ID: 0x%llX).\n", entry->id);
			rund.apool->del(rund.apool, entry);
			continue;
		}

		/* If the trigger time is lesser than current time and no step is defined, invalidate this entry. */
		if ((entry->trigger < t) && !entry->step) {
			log_info("marshal_daemon_unserialize_pools(): Found an invalid entry (ID: 0x%llX).\n", entry->id);
			rund.apool->del(rund.apool, entry);
			continue;
		}
			
		/* Install a new scheduling entry based on the current entry parameters */
		if ((entry->psched_id = psched_timestamp_arm(rund.psched, entry->trigger, entry->step, entry->expire, &entry_pmq_dispatch, entry)) == (pschedid_t) -1) {
			log_warn("marshal_daemon_unserialize_pools(): psched_timestamp_arm(): %s\n", strerror(errno));

			rund.apool->del(rund.apool, entry);

			continue;
		}
	}

_unserialize_finish:
	pthread_mutex_unlock(&rund.mutex_apool);

	return ret;
}

void marshal_daemon_wipe(void) {
	if (unlink(CONFIG_USCHED_FILE_DAEMON_SERIALIZE) < 0)
		log_warn("marshal_daemon_wipe(): unlink(\"%s\"): %s\n", CONFIG_USCHED_FILE_DAEMON_SERIALIZE, strerror(errno));
}

void marshal_daemon_destroy(void) {
	if (syncfs(rund.ser_fd) < 0)
		log_warn("marshal_daemon_destroy(): syncfs(): %s\n", strerror(errno));

	if (close(rund.ser_fd) < 0)
		log_warn("marshal_daemon_destroy(): close(): %s\n", strerror(errno));
}

