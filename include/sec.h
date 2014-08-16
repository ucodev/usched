/**
 * @file sec.h
 * @brief uSched
 *        Security interface header
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


#ifndef USCHED_SEC_H
#define USCHED_SEC_H

#include <stdio.h>

#include "config.h"

/* Structures */
struct usched_sec {
	unsigned char key_prv[CONFIG_USCHED_SEC_PRVKEY_SIZE];
	unsigned char key_pub[CONFIG_USCHED_SEC_PUBKEY_SIZE];
};

/* Prototypes */
int sec_dh_keys_init(unsigned char *key_prv, size_t prv_size, unsigned char *key_pub, size_t pub_size);
int sec_client_init(void);
int sec_client_compute_shared_key(unsigned char *shared_key, unsigned char *public_key);
void sec_client_destroy(void);
int sec_daemon_init(void);
int sec_daemon_compute_shared_key(unsigned char *shared_key, unsigned char *public_key);
void sec_daemon_destroy(void);

#endif

