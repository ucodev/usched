/**
 * @file schedule.c
 * @brief uSched
 *        Scheduling handlers interface
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
#include <stdint.h>
#include <pthread.h>

#include <psched/sched.h>

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
	psched_destroy(rund.psched);
}

int schedule_entry_create(struct usched_entry *entry) {
	int errsv = 0;

	/* Create the unique index key for this entry */
	if (index_entry_create(entry) < 0) {
		errsv = errno;
		log_warn("schedule_entry_create(): index_entry_create(): %s\n", strerror(errno));

		mm_free(entry->payload);
		mm_free(entry);

		errno = errsv;

		return -1;
	}

	/* Install a new scheduling entry based on the current entry parameters */
	if ((entry->psched_id = psched_timestamp_arm(rund.psched, entry->trigger, entry->step, entry->expire, &entry_pmq_dispatch, entry)) == (pschedid_t) -1) {
		errsv = errno;
		log_warn("schedule_entry_create(): psched_timestamp_arm(): %s\n", strerror(errno));

		mm_free(entry->payload);
		mm_free(entry);

		errno = errsv;

		return -1;
	}

	/* Insert the new entry into the entries list */
	pthread_mutex_lock(&rund.mutex_apool);

	if (rund.apool->insert(rund.apool, entry) < 0) {
		errsv = errno;
		pthread_mutex_unlock(&rund.mutex_apool);
		errno = errsv;

		log_warn("schedule_entry_create(): rund.apool->insert(): %s\n", strerror(errno));

		if (psched_disarm(rund.psched, entry->psched_id) < 0)
			log_warn("schedule_entry_create(): psched_disarm(): %s\n", strerror(errno));

		mm_free(entry->payload);
		mm_free(entry);

		errno = errsv;

		return -1;
	}

	pthread_mutex_unlock(&rund.mutex_apool);

	return 0;
}

struct usched_entry *schedule_entry_disable(struct usched_entry *entry) {
	int errsv = 0;

	pthread_mutex_lock(&rund.mutex_apool);
	entry = rund.apool->search(rund.apool, entry);
	pthread_mutex_unlock(&rund.mutex_apool);

	if (!entry) {
		errsv = errno;
		log_warn("schedule_entry_disable(): entry == NULL\n");
		errno = errsv;
		return NULL;
	}

	if (!entry->psched_id) {
		errsv = errno;
		log_warn("schedule_entry_disable(): entry->psched_id == NULL\n");
		errno = errsv;
		return NULL;
	}

	if (psched_disarm(rund.psched, entry->psched_id) < 0) {
		errsv = errno;
		log_warn("schedule_entry_disable(): psched_disarm(): %s\n", strerror(errno));
		errno = errsv;
		/* This isn't critical, so do not return NULL here. */
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

int schedule_entry_update(struct usched_entry *entry) {
	struct timespec trigger, step, expire;

	if (psched_search(rund.psched, entry->psched_id, &trigger, &step, &expire) < 0) {
		/* Entry not found */
		return 0;
	}

	entry->trigger = trigger.tv_sec;
	entry->step = step.tv_sec;
	entry->expire = expire.tv_sec;

	return 1;
}

