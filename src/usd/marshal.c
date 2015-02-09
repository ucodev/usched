/**
 * @file marshal.c
 * @brief uSched
 *        Serialization / Unserialization interface
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
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <pthread.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <psched/sched.h>

#include <fsop/file.h>

#include "config.h"
#include "debug.h"
#include "runtime.h"
#include "marshal.h"
#include "log.h"
#include "entry.h"
#include "schedule.h"

int marshal_daemon_init(void) {
	int errsv = 0;

	if ((rund.ser_fd = open(rund.config.core.serialize_file, O_CREAT | O_SYNC | O_RDWR, S_IRUSR | S_IWUSR)) < 0) {
		errsv = errno;
		log_warn("marshal_daemon_init(): open(\"%s\", ...): %s\n", rund.config.core.serialize_file, strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Test lock on serialization file */
	if (lockf(rund.ser_fd, F_TLOCK, 0) < 0) {
		errsv = errno;
		log_warn("marshal_daemon_init(): lockf(): Serialization file is locked by another process.\n");
		errno = errsv;
		return -1;
	}

	/* Acquire lock on serialization file */
	if (lockf(rund.ser_fd, F_LOCK, 0) < 0) {
		errsv = errno;
		log_warn("marshal_daemon_init(): lockf(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	return 0;
}

int marshal_daemon_serialize_pools(void) {
	int errsv = 0;
	int ret = -1;
	struct usched_entry *entry = NULL;

	pthread_mutex_lock(&rund.mutex_apool);

	log_info("Serialize pools (Checkpoint #1)...\n");

	/* Grant entry status correctness before serialization */
	for (rund.apool->rewind(rund.apool, 0); (entry = rund.apool->iterate(rund.apool)); ) {
		/* Check if we need to compensate the entry time values */
		if (abs(rund.delta_last) >= rund.config.core.delta_reload) {
			/* If this entry was triggered at least once OR if has a relative trigger,
			 * we must compensate the trigger value with the last known time variation
			 * value.
			 *
			 * NOTE that for already TRIGGERED entries, we must only compensate if the
			 * time variation is negative, because if the time was changed to the future
			 * the compensation was already performed by the psched library.
			 *
			 */
			if ((entry_has_flag(entry, USCHED_ENTRY_FLAG_TRIGGERED) && (rund.delta_last < 0)) || entry_has_flag(entry, USCHED_ENTRY_FLAG_RELATIVE_TRIGGER)) {
				log_info("Entry ID 0x%016llX trigger (timestamp: %u) will be compensated by %lld seconds due to machine time changes...", entry->id, entry->trigger, rund.delta_last);

				entry->trigger += rund.delta_last;
			}
		}

		/* NOTE: Further integrity checks should be implemented below */
	}

	log_info("Serialize pools (Checkpoint #2)...\n");

	/* Always set the file descriptor position to the beggining of the serialization file */
	if (lseek(rund.ser_fd, 0, SEEK_SET) == (off_t) -1) {
		errsv = errno;
		log_warn("marshal_daemon_serialize_pools(): lseek(%d, 0, SEEK_SET): %s\n", rund.ser_fd, strerror(errsv));

#if CONFIG_USCHED_SERIALIZE_ON_REQ == 1
		pthread_mutex_unlock(&rund.mutex_apool);

		errno = errsv;

		/* We can give up here... all the entries are serialized */
		return -1;
#else
		/* NOTE: We can't just give up here, or all the entries will be lost.
		 * We'll desperately try to reopen the serialization file and hope for the best...
		 */

 #if CONFIG_USCHED_DROP_PRIVS == 0
  #error "CONFIG_USCHED_SERIALIZE_ON_REQ is disabled and CONFIG_USCHED_DROP_PRIVS isn't enabled... unable to compile a uSched safe state."
 #endif
		close(rund.ser_fd);

		marshal_daemon_destroy();

		if (marshal_daemon_init() < 0) {
			/* TODO:
			 * We've tried almost everything... but we can still create another file
			 * on some temporary directory to dump the data...*/

			pthread_mutex_unlock(&rund.mutex_apool);

			errno = errsv;

			return -1;
		}
#endif
	}

	log_info("Serialize pools (Checkpoint #3)...\n");

	/* Serialize the active pool */
	if ((ret = rund.apool->serialize(rund.apool, rund.ser_fd)) < 0) {
		errsv = errno;
		log_warn("marshal_daemon_serialize_pools(): rund.apool->serialize(): %s\n", strerror(errno));
	}

	log_info("Serialize pools (Checkpoint #4)...\n");

	pthread_mutex_unlock(&rund.mutex_apool);

	errno = errsv;

	return ret;
}

int marshal_daemon_unserialize_pools(void) {
	int ret = -1, errsv = errno;
	struct usched_entry *entry = NULL;

	pthread_mutex_lock(&rund.mutex_apool);

	/* Always set the file descriptor position to the beggining of the serialization file */
	if (lseek(rund.ser_fd, 0, SEEK_SET) == (off_t) -1) {
		errsv = errno;
		pthread_mutex_unlock(&rund.mutex_apool);
		log_warn("marshal_daemon_serialize_pools(): lseek(%d, 0, SEEK_SET): %s\n", rund.ser_fd, strerror(errsv));
		errno = errsv;

		return -1;
	}

	/* Unserialize active pool */
	if ((ret = rund.apool->unserialize(rund.apool, rund.ser_fd)) < 0) {
		errsv = errno;
		log_warn("marshal_daemon_unserialize_pools(): rund.apool->unserialize(): %s\n", strerror(errno));
		goto _unserialize_finish;
	}

	/* Activate all the unserialized entries through libpsched */
	for (rund.apool->rewind(rund.apool, 0); (entry = rund.apool->iterate(rund.apool)); ) {
		/* If the entry was already triggered before and the next execution exceeds the step
		 * value relative to the current time, then the machine time was changed while the
		 * daemon wasn't running and we need to compensate this entry.
		 */
		if (entry_has_flag(entry, USCHED_ENTRY_FLAG_TRIGGERED) && ((entry->trigger - entry->step) >= time(NULL))) {
			do entry->trigger -= entry->step;
			while (entry->trigger >= time(NULL));

			/* Further adjustments (positive) will be performed in the next loop */
		}

		/* TODO or FIXME: Currently we can't handle relative triggered entries that were not
		 * triggered before the daemon serialized the data. There's also no guarantee that
		 * this will ever be supported as it will require some changes in the daemon and
		 * data tracking that will not be implemented in the near future. Avoid the use of the
		 * IN preposition if you expect the daemon to be stopped while the machine time is
		 * changed to the past.
		 */

		/* Update the trigger value based on step and current time */
		while (entry->step && (entry->trigger <= time(NULL))) {
			/* Check if we've to align (month or year?) and update trigger accordingly */
			if (entry_has_flag(entry, USCHED_ENTRY_FLAG_MONTHDAY_ALIGN)) {
				entry->trigger += schedule_step_ts_add_month(entry->trigger, entry->step / 2592000);
			} else if (entry_has_flag(entry, USCHED_ENTRY_FLAG_YEARDAY_ALIGN)) {
				entry->trigger += schedule_step_ts_add_year(entry->trigger, entry->step / 31536000);
			} else {
				/* No alignment required */
				entry->trigger += entry->step;
			}
		}

		/* Check if the trigger remains valid, i.e., does not exceed the expiration time */
		if (entry->expire && (entry->trigger >= entry->expire)) {
			log_info("marshal_daemon_unserialize_pools(): An entry is expired (ID: 0x%llX).\n", entry->id);

			/* libpall grants that it's safe to remove a node while iterating the list */
			rund.apool->del(rund.apool, entry);
			continue;
		}

		/* If the trigger time is lesser than current time and no step is defined, invalidate this entry. */
		if ((entry->trigger <= time(NULL)) && !entry->step) {
			log_info("marshal_daemon_unserialize_pools(): Found an invalid entry (ID: 0x%llX).\n", entry->id);

			/* libpall grants that it's safe to remove a node while iterating the list */
			rund.apool->del(rund.apool, entry);
			continue;
		}

		debug_printf(DEBUG_INFO, "[TIME: %lu]: entry->id: 0x%016llX, entry->trigger: %lu, entry->step: %lu, entry->expire: %lu\n", time(NULL), entry->id, entry->trigger, entry->step, entry->expire);

		/* Install a new scheduling entry based on the current entry parameters */
		if ((entry->reserved.psched_id = psched_timestamp_arm(rund.psched, entry->trigger, entry->step, entry->expire, &entry_daemon_pmq_dispatch, entry)) == (pschedid_t) -1) {
			log_warn("marshal_daemon_unserialize_pools(): psched_timestamp_arm(): %s\n", strerror(errno));

			/* libpall grants that it's safe to remove a node while iterating the list */
			rund.apool->del(rund.apool, entry);

			/* TODO or FIXME: This is critical, the entry will be lost and we can't force
			 * a graceful daemon restart or the serialization data will be overwritten
			 * with a missing entry... Something must be done here to prevent such damage.
			 *
			 * For now, an abort() will be performed in order to force the restart of the
			 * the daemon through the uSched Monitor (usm)... but despite the fact that
			 * this is pretty ugly, it may cause an infinite restart loop if we'll be
			 * still unable to perform a psched_timestamp_arm() successfully on the
			 * subsequent daemon restarts!
			 */
			abort();

			continue; /* Unreachable for now (abort() preceeds this) */
		}
	}

_unserialize_finish:
	pthread_mutex_unlock(&rund.mutex_apool);

	errno = errsv;

	return ret;
}

int marshal_daemon_backup(void) {
	int errsv = 0;
	size_t len = strlen(rund.config.core.serialize_file) + 50;
	char *file_bak = NULL;

	if (!(file_bak = mm_alloc(len))) {
		errsv = errno;
		log_warn("marshal_daemon_backup(): mm_alloc(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	memset(file_bak, 0, len);

	snprintf(file_bak, len - 1, "%s-%lu-%u", rund.config.core.serialize_file, time(NULL), getpid());

	if (fsop_cp(rund.config.core.serialize_file, file_bak, 8192) < 0) {
		errsv = errno;
		log_warn("marshal_daemon_backup(): fsop_cp(): %s\n", strerror(errno));
		mm_free(file_bak);
		errno = errsv;
		return -1;
	}

	mm_free(file_bak);

	return 0;
}

void marshal_daemon_wipe(void) {
	if (unlink(rund.config.core.serialize_file) < 0)
		log_warn("marshal_daemon_wipe(): unlink(\"%s\"): %s\n", rund.config.core.serialize_file, strerror(errno));
}

void marshal_daemon_destroy(void) {
#if CONFIG_USE_SYNCFS == 1
	if (syncfs(rund.ser_fd) < 0)
		log_warn("marshal_daemon_destroy(): syncfs(): %s\n", strerror(errno));
#else
	sync();
#endif
	/* Remove lock from serialization file */
	if (lockf(rund.ser_fd, F_ULOCK, 0) < 0)
		log_warn("marshal_daemon_init(): lockf(): %s\n", strerror(errno));

	if (close(rund.ser_fd) < 0)
		log_warn("marshal_daemon_destroy(): close(): %s\n", strerror(errno));
}

