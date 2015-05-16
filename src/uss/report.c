/**
 * @file report.c
 * @brief uSched
 *        Status and statistics reporting interface
 *
 * Date: 16-05-2015
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
#include <errno.h>
#include <unistd.h>
#include <pthread.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <fsop/path.h>

#include "config.h"
#include "runtime.h"
#include "report.h"
#include "log.h"

static void _report_stat_dump(FILE *fp) {
	log_info("_report_stat_dump(): Dumping statistical data...");

	/* DUmp statistical data */
	fprintf(fp, "TODO\n");
}

static void *_report_stat_monitor(void *arg) {
	FILE *fp = NULL;

	/* Dump statistical data whenever it is requested */
	for (;;) {
		/* TODO: Check for runtime interruptions */

		/* Open the named pipe */
		if (!(fp = fopen("/tmp/uss", "w"))) {
			log_warn("_report_stat_monitor(): fopen(\"%s\", \"w\"): %s\n", "/tmp/uss", strerror(errno));
			continue;
		}

		/* Dump data */
		_report_stat_dump(fp);

		/* Close the pipe */
		fclose(fp);

		/* Wait some time before generating another report */
		usleep(1000000);
	}

	/* All good */
	pthread_exit(NULL);

	/* Unreachable */
	return NULL;
}

int report_stat_init(void) {
	int errsv = 0;

	/* TODO: /tmp/uss and 0666 mode must be fetched from configuration, not hardcoded */

	/* If the named pipe for reporting doesn't exist or if the file exists but it isn't a
	 * named pipe, recreate it
	 */
	if (!fsop_path_exists("/tmp/uss") || !fsop_path_isfifo("/tmp/uss")) {
		unlink("/tmp/uss");

		if (mkfifo("/tmp/uss", 0666) < 0) {
			errsv = errno;
			log_warn("report_stat_init(): mkfifo(\"%s\"): %s\n", "/tmp/uss", strerror(errno));
			errno = errsv;
			return -1;
		}
	}

	/* Grant that the named pipe has the correct mode */
	if (chmod("/tmp/uss", 0666) < 0) {
		errsv = errno;
		log_warn("report_stat_init(): chmod(): %s\n", strerror(errno));
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

