/**
 * @file schedule.c
 * @brief uSched
 *        Scheduling handlers interface
 *
 * Date: 05-03-2015
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


#include <time.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <pthread.h>

#include <sys/types.h>

#include <psched/psched.h>

#include "debug.h"
#include "log.h"
#include "mm.h"
#include "runtime.h"
#include "entry.h"
#include "index.h"
#include "schedule.h"

int schedule_daemon_init(void) {
	int errsv = 0;

	if (!(rund.psched = psched_thread_init())) {
		errsv = errno;
		log_crit("schedule_daemon_init(): psched_thread_init(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	return 0;
}

void schedule_daemon_destroy(void) {
	psched_t *psched_tmp = rund.psched;

	/* psched_destroy() must be called before we acquire the apool lock, or a deadlock will
	 * occur (since event_pmq_dispatch() will wait to acquire this lock, while this function will
	 * wait for pmq_dispatch() to complete).
	 *
	 * This will not cause any races nor corruptions, since psched library prevent any new
	 * entries from being created, while granting integrity of the current entries until they
	 * complete.
	 */
	psched_destroy(rund.psched);

	/* Lock the active pool access to avoid races */
	pthread_mutex_lock(&rund.mutex_apool);

	/* NOTE: We need to explicitly inform that scheduling interface was destroyed to avoid entry
	 * updates that may occur due to queued routines on psched library that were not yet executed
	 */
	rund.psched = NULL;

	/*
	 * We need to set rund.psched to NULL first and then use the temporary pointer as the
	 * argument for the psched_handler_destroy() to avoid NULL pointer reference access attempts
	 * by the SIGABRT signal handler.
	 * Note that we can't acquire the mutex_apool lock in the SIGABRT handler since the locking
	 * mechanism from libpthread isn't AS-safe.
	 */

	psched_handler_destroy(psched_tmp);

	/* FIXME: If a SIGABRT emerges from now on and before rund.psched is set to NULL, this will
	 * cause a NULL pointer reference access attempt. To fix this, the signal handler for
	 * SIGABRT must acquire the mutex_apool lock... but this is not possible since the locking
	 * mechanism isn't AS-safe. Although the probability of this event to happen is very low,
	 * this must be corrected with some new approach in the future.
	 *
	 */

	/* TODO or FIXME: We must grant somehow at this point that all the routines queued by
	 * libpsched are flushed (inactive) OR grant that they won't disrupt the runtime exiting
	 * procedures if they are triggered after this point in time.
	 */

	pthread_mutex_unlock(&rund.mutex_apool);
}

int schedule_daemon_active(void) {
	return (int) (uintptr_t) rund.psched;
}

int schedule_entry_create(struct usched_entry *entry) {
	int errsv = 0;

	pthread_mutex_lock(&rund.mutex_apool);

	/* Grant a unique entry->id that is different than 0 */
	do {
		/* Create the unique index key for this entry */
		if (index_entry_create(entry) < 0) {
			errsv = errno;
			log_warn("schedule_entry_create(): index_entry_create(): %s\n", strerror(errno));
			errno = errsv;

			pthread_mutex_unlock(&rund.mutex_apool);

			return -1;
		}
	} while (rund.apool->search(rund.apool, entry) || !entry->id);

	/* Install a new scheduling entry based on the current entry parameters */
	if ((entry->reserved.psched_id = psched_timestamp_arm(rund.psched, entry->trigger, entry->step, entry->expire, &entry_daemon_pmq_dispatch, entry)) == (pschedid_t) -1) {
		errsv = errno;

		log_warn("schedule_entry_create(): psched_timestamp_arm(): %s\n", strerror(errno));

		errno = errsv;

		return -1;
	}

	/* Insert the new entry into the entries list */
	if (rund.apool->insert(rund.apool, entry) < 0) {
		errsv = errno;
		pthread_mutex_unlock(&rund.mutex_apool);
		errno = errsv;

		log_warn("schedule_entry_create(): rund.apool->insert(): %s\n", strerror(errno));

		if (psched_disarm(rund.psched, entry->reserved.psched_id) < 0)
			log_warn("schedule_entry_create(): psched_disarm(): %s\n", strerror(errno));

		errno = errsv;

		return -1;
	}

	pthread_mutex_unlock(&rund.mutex_apool);

	return 0;
}

struct usched_entry *schedule_entry_get_copy(uint64_t entry_id) {
	int errsv = 0;
	struct usched_entry *entry = NULL, *entry_dest = NULL;

	pthread_mutex_lock(&rund.mutex_apool);
	entry = rund.apool->search(rund.apool, usched_entry_id(entry_id));

	if (!entry) {
		log_warn("schedule_entry_search(): entry == NULL\n");
		pthread_mutex_unlock(&rund.mutex_apool);
		errno = EINVAL;
		return NULL;
	}

	if (!entry->reserved.psched_id) {
		log_warn("schedule_entry_get_copy(): entry->reserved.psched_id == 0\n");
		pthread_mutex_unlock(&rund.mutex_apool);
		errno = EINVAL;
		return NULL;
	}

	if (!(entry_dest = mm_alloc(sizeof(struct usched_entry)))) {
		errsv = errno;
		log_warn("schedule_entry_get_copy(): mm_alloc(): %s\n", strerror(errno));
		pthread_mutex_unlock(&rund.mutex_apool);
		errno = errsv;
		return NULL;
	}

	if (entry_copy(entry_dest, entry) < 0) {
		errsv = errno;
		log_warn("schedule_entry_get_copy(): entry_copy(): %s\n", strerror(errno));
		pthread_mutex_unlock(&rund.mutex_apool);
		mm_free(entry_dest);
		errno = errsv;
		return NULL;
	}

	pthread_mutex_unlock(&rund.mutex_apool);

	return entry_dest;
}

int schedule_entry_get_by_uid(uid_t uid, uint64_t **entry_list, uint32_t *count) {
	int errsv = 0;
	struct usched_entry *entry = NULL;

	pthread_mutex_lock(&rund.mutex_apool);

	for (*count = 0, *entry_list = NULL, rund.apool->rewind(rund.apool, 0); (entry = rund.apool->iterate(rund.apool)); ++ *count) {
		if (entry->uid != uid)
			continue;

		if (!(*entry_list = mm_realloc(*entry_list, (1 + *count) * sizeof(uint64_t)))) {
			errsv = errno;
			log_warn("schedule_entry_get_by_uid(): mm_realloc(): %s\n", strerror(errno));
			*count = 0;
			pthread_mutex_unlock(&rund.mutex_apool);
			errno = errsv;
			return -1;
		}

		(*entry_list)[*count] = entry->id;
	}

	pthread_mutex_unlock(&rund.mutex_apool);

	if (!*entry_list)
		*count = 0;

	return 0;
}

struct usched_entry *schedule_entry_disable(struct usched_entry *entry) {
	int errsv = 0;

	pthread_mutex_lock(&rund.mutex_apool);
	entry = rund.apool->search(rund.apool, entry);
	pthread_mutex_unlock(&rund.mutex_apool);

	if (!entry) {
		log_warn("schedule_entry_disable(): entry == NULL\n");
		errno = EINVAL;
		return NULL;
	}

	if (!entry->reserved.psched_id) {
		log_warn("schedule_entry_disable(): entry->reserved.psched_id == 0\n");
		errno = EINVAL;
		return NULL;
	}

	if (psched_disarm(rund.psched, entry->reserved.psched_id) < 0) {
		errsv = errno;

		/* This isn't that critical, so do not return NULL here. However, we should set the
		 * FATAL flag to runtime in order to force the daemon to be restarted by uSched
		 * monitor (usm) and load the consistent data from the serialization file, cleaning
		 * up the existing unhandled armed entries.
		 */
		runtime_daemon_fatal();

		log_warn("schedule_entry_disable(): psched_disarm(): %s\n", strerror(errno));

		errno = errsv;
	}

	pthread_mutex_lock(&rund.mutex_apool);
	entry = rund.apool->pope(rund.apool, entry);
	pthread_mutex_unlock(&rund.mutex_apool);

	return entry;
}

int schedule_entry_delete(struct usched_entry *entry) {
	int errsv = 0;

	if (!(entry = schedule_entry_disable(entry))) {
		errsv = errno;
		log_warn("schedule_entry_delete(): schedule_entry_disable(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	entry_destroy(entry);

	return 0;
}

int schedule_entry_ownership_delete_by_id(uint64_t id, uid_t uid) {
	struct usched_entry *entry = NULL;

	pthread_mutex_lock(&rund.mutex_apool);

	entry = rund.apool->search(rund.apool, usched_entry_id(id));

	/* Check if the entry exists */
	if (!entry) {
		log_warn("schedule_entry_ownership_delete_by_id(): Entry ID 0x%llX not found.\n", id);
		pthread_mutex_unlock(&rund.mutex_apool);
		errno = EACCES;
		return -1;
	}

	/* Check if the entry is active and processing is finished */
	if (!entry_has_flag(entry, USCHED_ENTRY_FLAG_FINISH)) {
		log_warn("schedule_entry_ownership_delete_by_id(): Entry ID 0x%llX is still being processed.\n", id);
		pthread_mutex_unlock(&rund.mutex_apool);
		errno = EAGAIN;
		return -1;
	}

	/* Check if the requester owns the entry */
	if (entry->uid != uid) {
		log_warn("schedule_entry_ownership_delete_by_id(): Unauthorized delete (entry->uid[%u] != uid[%u])", entry->uid, uid);
		pthread_mutex_unlock(&rund.mutex_apool);
		errno = EACCES;
		return -1;
	}

	/* Check if psched_id is still valid */
	if (!entry->reserved.psched_id) {
		log_warn("schedule_entry_ownership_delete_by_id(): entry->reserved.psched_id == 0\n");
		pthread_mutex_unlock(&rund.mutex_apool);
		errno = EINVAL;
		return -1;
	}

	if (psched_disarm(rund.psched, entry->reserved.psched_id) < 0) {
		log_warn("schedule_entry_ownership_delete_by_id(): psched_disarm(): %s\n", strerror(errno));
		/* This isn't critical, so do not return NULL here. */
	}

	/* Delete the entry */
	rund.apool->del(rund.apool, entry);

	pthread_mutex_unlock(&rund.mutex_apool);

	return 0;
}

int schedule_entry_update(struct usched_entry *entry) {
	int errsv = 0;
	struct timespec trigger, step, expire;

	/* Check if it's safe to update this entry */
	if (!schedule_daemon_active()) {
		log_info("schedule_entry_update(): Scheduling interface was destroyed. Entry ID 0x%016llX will not be updated.\n", entry->id);
		errno = ECANCELED;
		return -1;
	}

	debug_printf(DEBUG_INFO, "[SCHEDULE UPDATE BEGIN]: entry->id: 0x%016llX, entry->trigger: %lu, entry->step: %lu, entry->expire: %lu\n", entry->id, entry->trigger, entry->step, entry->expire);

	/* Search for the entry in order to grant that it is still valid (not expired) */
	if (psched_search(rund.psched, entry->reserved.psched_id, &trigger, &step, &expire) < 0) {
		debug_printf(DEBUG_INFO, "[SCHEDULE UPDATE END]: Entry not found");

		/* Entry not found */
		return 0;
	}

	/* Check if entry is flagged for any alignment */
	if (entry_has_flag(entry, USCHED_ENTRY_FLAG_MONTHDAY_ALIGN) || entry_has_flag(entry, USCHED_ENTRY_FLAG_YEARDAY_ALIGN)) {
		/* Disarm the entry before performing any step alignments */
		if (psched_disarm(rund.psched, entry->reserved.psched_id) < 0) {
			errsv = errno;
			log_warn("schedule_entry_update(): psched_disarm(): %s\n", strerror(errno));
			errno = errsv;

			/* Update with the last known values of the psched entry before failing */
			entry->trigger = trigger.tv_sec;
			entry->step = step.tv_sec;
			entry->expire = expire.tv_sec;

			return -1;
		}

		/* Check what we've to align (month or year?) and update trigger accordingly */
		if (entry_has_flag(entry, USCHED_ENTRY_FLAG_MONTHDAY_ALIGN)) {
			entry->trigger += schedule_step_ts_add_month(entry->trigger, (unsigned int) entry->step / 2592000);
		} else { /* USCHED_ENTRY_FLAG_YEARDAY_ALIGN */
			entry->trigger += schedule_step_ts_add_year(entry->trigger, (unsigned int) entry->step / 31536000);
		}

		/* Re-arm the entry with the correct alignments */
		if ((entry->reserved.psched_id = psched_timestamp_arm(rund.psched, entry->trigger, entry->step, entry->expire, &entry_daemon_pmq_dispatch, entry)) == (pschedid_t) -1) {
			errsv = errno;

			runtime_daemon_fatal();

			log_crit("schedule_entry_update(): psched_timestamp_arm(): %s\n", strerror(errno));
			errno = errsv;

			/* We've set the FATAL flag to runtime, which means that the daemon will
			 * exit and restarted by uSched monitor (usm). TODO or FIXME: This may
			 * cause an infinite loop if further restarts can't successfuly perform the
			 * psched_timestamp_arm() routine (but this is unlikely to ever happen).
			 */

			return -1;
		}
	} else {
		entry->trigger = trigger.tv_sec;
		entry->step = step.tv_sec;
		entry->expire = expire.tv_sec;
	}

	debug_printf(DEBUG_INFO, "[SCHEDULE UPDATE END]: entry->id: 0x%016llX, entry->trigger: %lu, entry->step: %lu, entry->expire: %lu\n", entry->id, entry->trigger, entry->step, entry->expire);

	return 1;
}

uint32_t schedule_step_ts_add_month(time_t t, unsigned int months) {
	struct tm tm;

	localtime_r(&t, &tm);

	tm.tm_mon += months;

	return mktime(&tm) - t;
}

uint32_t schedule_step_ts_add_year(time_t t, unsigned int years) {
	struct tm tm;

	localtime_r(&t, &tm);

	tm.tm_year += years;

	return mktime(&tm) - t;
}

