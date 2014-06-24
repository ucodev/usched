/**
 * @file daemonize.c
 * @brief uSched
 *        Daemonizer interface
 *
 * Date: 24-06-2014
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
#include <stdlib.h>
#include <unistd.h>

#include "config.h"
#include "log.h"

int daemonize(void) {
#if !defined(CONFIG_USCHED_DEBUG) || (CONFIG_USCHED_DEBUG == 0)
	pid_t pid = 0;

	if ((pid = fork()) > 0) {
		exit(EXIT_SUCCESS);
	} else if (pid == (pid_t) -1) {
		log_warn("daemonize(): fork(): %s\n", strerror(errno));
		return -1;
	}

	if (!(freopen(CONFIG_SYS_DEV_ZERO, "r", stdin))) {
		log_warn("daemonize(): freopen(\"%s\", \"r\", stdin): %s\n", CONFIG_SYS_DEV_ZERO, strerror(errno));
		return -1;
	}

	if (!(freopen(CONFIG_SYS_DEV_NULL, "a", stdout))) {
		log_warn("daemonize(): freopen(\"%s\", \"a\", stdout): %s\n", CONFIG_SYS_DEV_NULL, strerror(errno));
		return -1;
	}

	if (!(freopen(CONFIG_SYS_DEV_NULL, "a", stderr))) {
		log_warn("daemonize(): freopen(\"%s\", \"a\", stderr): %s\n", CONFIG_SYS_DEV_NULL, strerror(errno));
		return -1;
	}

	if (setsid() == (pid_t) -1) {
		log_warn("daemonize(): setsid(): %s\n", strerror(errno));
		return -1;
	}
#endif

	return 0;
}

