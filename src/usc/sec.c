/**
 * @file sec.c
 * @brief uSched
 *        Security interface - Client
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

#include <psec/ke.h>

#include "runtime.h"
#include "log.h"
#include "sec.h"

int sec_client_init(void) {
	int errsv = 0;

	/* Create DH private/public key pair */
	if (sec_dh_keys_init(runc.sec.key_prv, sizeof(runc.sec.key_prv), runc.sec.key_pub, sizeof(runc.sec.key_pub)) < 0) {
		errsv = errno;
		log_warn("sec_daemon_init(): sec_dh_keys_init(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* All good */
	return 0;
}

int sec_client_compute_shared_key(unsigned char *shared_key, unsigned char *public_key) {
	int errsv = 0;

	/* Compute shared key based on remote public key*/
	if (!ke_dh_shared(shared_key, public_key, sizeof(runc.sec.key_pub), runc.sec.key_prv, sizeof(runc.sec.key_prv))) {
		errsv = errno;
		log_warn("sec_client_compute_shared_key(): ke_dh_shared(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* All good */
	return 0;
}

void sec_client_destroy(void) {
	memset(&runc.sec, 0, sizeof(rund.sec));
}

