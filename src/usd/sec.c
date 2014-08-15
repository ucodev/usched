/**
 * @file sec.c
 * @brief uSched
 *        Security interface - Daemon
 *
 * Date: 15-08-2014
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

#include "runtime.h"
#include "log.h"
#include "sec.h"

int sec_daemon_init(void) {
	int errsv = 0;

	/* Create DH private/public key pair */
	if (sec_dh_keys_init(rund.sec.key_prv, sizeof(rund.sec.key_prv), rund.sec.key_pub, sizeof(rund.sec.key_pub)) < 0) {
		errsv = errno;
		log_warn("sec_daemon_init(): sec_dh_keys_init(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* All good */
	return 0;
}

void sec_daemon_destroy(void) {
	memset(&rund.sec, 0, sizeof(rund.sec));
}

