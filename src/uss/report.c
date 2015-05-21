/**
 * @file report.c
 * @brief uSched
 *        Status and statistics reporting interface
 *
 * Date: 21-05-2015
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
#include <stdint.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <fsop/path.h>

#include <psched/timespec.h>

#include "config.h"
#include "runtime.h"
#include "report.h"
#include "log.h"
#include "stat.h"

static unsigned long long _report_stat_entry_count(void) {
	return runs.spool->count(runs.spool);
}

static size_t _report_stat_entry_ok(void) {
	struct usched_stat_entry *e = NULL;
	size_t count = 0;

	pthread_mutex_lock(&runs.mutex_spool);

	for (runs.spool->rewind(runs.spool, 0); (e = runs.spool->iterate(runs.spool)); ) {
		/* If exit status is 0, the entry is OK */
		if (!e->current.status)
			count ++;
	}

	pthread_mutex_unlock(&runs.mutex_spool);

	return count;
}

static size_t _report_stat_entry_fail(void) {
	struct usched_stat_entry *e = NULL;
	size_t count = 0;

	pthread_mutex_lock(&runs.mutex_spool);

	for (runs.spool->rewind(runs.spool, 0); (e = runs.spool->iterate(runs.spool)); ) {
		/* If exit status isn't 0, the entry failed */
		if (e->current.status)
			count ++;
	}

	pthread_mutex_unlock(&runs.mutex_spool);

	return count;
}

static size_t _report_stat_entry_total_exec(void) {
	struct usched_stat_entry *e = NULL;
	size_t count = 0;

	pthread_mutex_lock(&runs.mutex_spool);

	for (runs.spool->rewind(runs.spool, 0); (e = runs.spool->iterate(runs.spool)); ) {
		count += e->nr_exec;
	}

	pthread_mutex_unlock(&runs.mutex_spool);

	return count;
}

static size_t _report_stat_entry_total_ok(void) {
	struct usched_stat_entry *e = NULL;
	size_t count = 0;

	pthread_mutex_lock(&runs.mutex_spool);

	for (runs.spool->rewind(runs.spool, 0); (e = runs.spool->iterate(runs.spool)); ) {
		count += e->nr_ok;
	}

	pthread_mutex_unlock(&runs.mutex_spool);

	return count;
}

static size_t _report_stat_entry_total_fail(void) {
	struct usched_stat_entry *e = NULL;
	size_t count = 0;

	pthread_mutex_lock(&runs.mutex_spool);

	for (runs.spool->rewind(runs.spool, 0); (e = runs.spool->iterate(runs.spool)); ) {
		count += e->nr_fail;
	}

	pthread_mutex_unlock(&runs.mutex_spool);

	return count;
}

static unsigned long long _report_stat_latency_max(void) {
	struct usched_stat_entry *e = NULL;
	struct timespec tval = { 0, 0 }, tmax = { 0, 0 };
	unsigned long long ret = 0;

	pthread_mutex_lock(&runs.mutex_spool);

	for (runs.spool->rewind(runs.spool, 0); (e = runs.spool->iterate(runs.spool)); memset(&tval, 0, sizeof(struct timespec))) {
		memcpy(&tval, &e->current.start, sizeof(struct timespec));
		timespec_sub(&tval, &e->current.trigger);

		if (timespec_cmp(&tval, &tmax) > 0)
			memcpy(&tmax, &tval, sizeof(struct timespec));
	}

	pthread_mutex_unlock(&runs.mutex_spool);

	ret = (tmax.tv_sec * 1000000000) + tmax.tv_nsec;
	
	return ret;
}

static unsigned long long _report_stat_latency_min(void) {
	struct usched_stat_entry *e = NULL;
	struct timespec tval = { 0, 0 }, tmin = { 0, 0 };
	unsigned long long ret = 0;

	pthread_mutex_lock(&runs.mutex_spool);

	for (runs.spool->rewind(runs.spool, 0); (e = runs.spool->iterate(runs.spool)); memset(&tval, 0, sizeof(struct timespec))) {
		memcpy(&tval, &e->current.start, sizeof(struct timespec));
		timespec_sub(&tval, &e->current.trigger);

		if (!timespec_cmp(&tmin, (struct timespec [1]) { { 0, 0 } })) {
			memcpy(&tmin, &tval, sizeof(struct timespec));
		} else if (timespec_cmp(&tmin, &tval) > 0) {
			memcpy(&tmin, &tval, sizeof(struct timespec));
		}
	}

	pthread_mutex_unlock(&runs.mutex_spool);

	ret = (tmin.tv_sec * 1000000000) + tmin.tv_nsec;
	
	return ret;
}

static unsigned long long _report_stat_latency_avg(void) {
	struct usched_stat_entry *e = NULL;
	struct timespec tval = { 0, 0 }, tavg = { 0, 0 };
	unsigned long long ret = 0;
	size_t count = 0;

	pthread_mutex_lock(&runs.mutex_spool);

	for (runs.spool->rewind(runs.spool, 0); (e = runs.spool->iterate(runs.spool)); ) {
		memcpy(&tval, &e->current.start, sizeof(struct timespec));
		timespec_sub(&tval, &e->current.trigger);
		timespec_add(&tavg, &tval);

		count ++;
	}

	pthread_mutex_unlock(&runs.mutex_spool);

	ret = ((tavg.tv_sec * 1000000000) + tavg.tv_nsec) / count;
	
	return ret;
}

static unsigned long long _report_stat_exectime_max(void) {
	struct usched_stat_entry *e = NULL;
	struct timespec tval = { 0, 0 }, tmax = { 0, 0 };
	unsigned long long ret = 0;

	pthread_mutex_lock(&runs.mutex_spool);

	for (runs.spool->rewind(runs.spool, 0); (e = runs.spool->iterate(runs.spool)); memset(&tval, 0, sizeof(struct timespec))) {
		memcpy(&tval, &e->current.end, sizeof(struct timespec));
		timespec_sub(&tval, &e->current.start);

		if (timespec_cmp(&tval, &tmax) > 0)
			memcpy(&tmax, &tval, sizeof(struct timespec));
	}

	pthread_mutex_unlock(&runs.mutex_spool);

	ret = (tmax.tv_sec * 1000000000) + tmax.tv_nsec;
	
	return ret;
}

static unsigned long _report_stat_exectime_min(void) {
	struct usched_stat_entry *e = NULL;
	struct timespec tval = { 0, 0 }, tmin = { 0, 0 };
	unsigned long long ret = 0;

	pthread_mutex_lock(&runs.mutex_spool);

	for (runs.spool->rewind(runs.spool, 0); (e = runs.spool->iterate(runs.spool)); memset(&tval, 0, sizeof(struct timespec))) {
		memcpy(&tval, &e->current.end, sizeof(struct timespec));
		timespec_sub(&tval, &e->current.start);

		if (!timespec_cmp(&tmin, (struct timespec [1]) { { 0, 0 } })) {
			memcpy(&tmin, &tval, sizeof(struct timespec));
		} else if (timespec_cmp(&tmin, &tval) > 0) {
			memcpy(&tmin, &tval, sizeof(struct timespec));
		}
	}

	pthread_mutex_unlock(&runs.mutex_spool);

	ret = (tmin.tv_sec * 1000000000) + tmin.tv_nsec;
	
	return ret;
}

static unsigned long _report_stat_exectime_avg(void) {
	struct usched_stat_entry *e = NULL;
	struct timespec tval = { 0, 0 }, tavg = { 0, 0 };
	unsigned long long ret = 0;
	size_t count = 0;

	pthread_mutex_lock(&runs.mutex_spool);

	for (runs.spool->rewind(runs.spool, 0); (e = runs.spool->iterate(runs.spool)); ) {
		memcpy(&tval, &e->current.end, sizeof(struct timespec));
		timespec_sub(&tval, &e->current.start);
		timespec_add(&tavg, &tval);

		count ++;
	}

	pthread_mutex_unlock(&runs.mutex_spool);

	ret = ((tavg.tv_sec * 1000000000) + tavg.tv_nsec) / count;
	
	return ret;
}

static void _report_stat_dump(FILE *fp) {
	log_info("_report_stat_dump(): Dumping statistical data...");

	/* DUmp statistical data */
	fprintf(fp, "Entries seen:              %llu\n", _report_stat_entry_count());
	fprintf(fp, "Entries in OK state:       %zu\n", _report_stat_entry_ok());
	fprintf(fp, "Entries in failed state:   %zu\n", _report_stat_entry_fail());
	fprintf(fp, "\n");
	fprintf(fp, "Total executions:          %zu\n", _report_stat_entry_total_exec());
	fprintf(fp, "Total executions OK:       %zu\n", _report_stat_entry_total_ok());
	fprintf(fp, "Total executions failed:   %zu\n", _report_stat_entry_total_fail());
	fprintf(fp, "\n");
	fprintf(fp, "Maximum scheduler latency: %.3fus\n", _report_stat_latency_max() / (float) 1000.0);
	fprintf(fp, "Minimum scheduler latency: %.3fus\n", _report_stat_latency_min() / (float) 1000.0);
	fprintf(fp, "Average scheduler latency: %.3fus\n", _report_stat_latency_avg() / (float) 1000.0);
	fprintf(fp, "\n");
	fprintf(fp, "Maximum entry exectime:    %.3fus\n", _report_stat_exectime_max() / (float) 1000.0);
	fprintf(fp, "Minimum entry exectime:    %.3fus\n", _report_stat_exectime_min() / (float) 1000.0);
	fprintf(fp, "Average entry exectime:    %.3fus\n", _report_stat_exectime_avg() / (float) 1000.0);

}

static void *_report_stat_monitor(void *arg) {
	FILE *fp = NULL;

	/* Dump statistical data whenever it is requested */
	for (;;) {
		/* TODO: Check for runtime interruptions */

		/* Open the named pipe */
		if (!(fp = fopen(runs.config.stat.report_file, "w"))) {
			log_warn("_report_stat_monitor(): fopen(\"%s\", \"w\"): %s\n", runs.config.stat.report_file, strerror(errno));
			continue;
		}

		/* Dump data */
		_report_stat_dump(fp);

		/* Close the pipe */
		fclose(fp);

		/* Wait some time before generating another report */
		usleep(1000000 / (float) runs.config.stat.report_freq);
	}

	/* All good */
	pthread_exit(NULL);

	/* Unreachable */
	return NULL;
}

int report_stat_init(void) {
	int errsv = 0;

	/* If the named pipe for reporting doesn't exist or if the file exists but it isn't a
	 * named pipe, recreate it
	 */
	if (!fsop_path_exists(runs.config.stat.report_file) || !fsop_path_isfifo(runs.config.stat.report_file)) {
		unlink(runs.config.stat.report_file);

		if (mkfifo(runs.config.stat.report_file, runs.config.stat.report_mode) < 0) {
			errsv = errno;
			log_warn("report_stat_init(): mkfifo(\"%s\"): %s\n", runs.config.stat.report_file, strerror(errno));
			errno = errsv;
			return -1;
		}
	}

	/* Grant that the named pipe has the correct mode */
	if (chmod(runs.config.stat.report_file, runs.config.stat.report_mode) < 0) {
		errsv = errno;
		log_warn("report_stat_init(): chmod(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	if (chown(runs.config.stat.report_file, runs.config.stat.privdrop_uid, runs.config.stat.privdrop_gid) < 0) {
		errsv = errno;
		log_warn("report_stat_init(): chown(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Create the report monitor worker */
	if ((errno = pthread_create(&runs.tid_report, NULL, &_report_stat_monitor, NULL))) {
		errsv = errno;
		log_warn("report_stat_init(): pthread_create(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* All good */
	return 0;
}

void report_stat_destroy(void) {
	/* Cancel the report monitor worker */
	pthread_cancel(runs.tid_report);
}

