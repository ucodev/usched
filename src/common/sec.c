/**
 * @file sec.c
 * @brief uSched
 *        Security interface - Common
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

#include "log.h"

int sec_dh_keys_init(
	unsigned char *key_prv,
	size_t prv_size,
	unsigned char *key_pub,
	size_t pub_size)
{
	int errsv = 0;

	/* Create DH private key */
	if (!ke_dh_private(key_prv, prv_size)) {
		errsv = errno;
		log_warn("sec_dh_keys_init(): ke_dh_private(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Create DH public key */
	if (!ke_dh_public(key_pub, pub_size, key_prv, prv_size)) {
		errsv = errno;
		log_warn("sec_dh_keys_init(): ke_dh_public(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* All good */
	return 0;
}


