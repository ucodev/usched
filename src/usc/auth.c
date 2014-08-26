/**
 * @file auth.c
 * @brief uSched
 *        Authentication and Authorization interface - Client
 *
 * Date: 24-08-2014
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

#include <psec/hash.h>
#include <psec/ke.h>

#include "runtime.h"
#include "log.h"
#include "auth.h"

int auth_client_remote_session_create(
	unsigned char *session,
	const char *username,
	const char *plain_passwd,
	unsigned char *context)
{
	int errsv = 0;
	unsigned char salt[HASH_DIGEST_SIZE_BLAKE2S];
	unsigned char salt_raw[CONFIG_USCHED_AUTH_USERNAME_MAX];

	/* Check if username doesn't exceed the expected size */
	if (strlen(username) > sizeof(salt)) {
		log_warn("auth_client_remote_session_create(): strlen(username) > sizeof(salt)\n");
		errno = EINVAL;
		return -1;
	}

	/* Craft raw salt */
	memset(salt_raw, 'x', sizeof(salt_raw));
	memcpy(salt_raw, username, strlen(username));

	/* Hash raw salt */
	if (!hash_buffer_blake2s(salt, salt_raw, sizeof(salt_raw))) {
		errsv = errno;
		log_warn("auth_client_remote_session_create(): hash_buffer_blake2s(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	if (!ke_pankake_client_init(session, context, plain_passwd, salt, sizeof(salt))) {
		errsv = errno;
		log_warn("auth_client_remote_session_create(): ke_pankake_client_init(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Session contents:
	 *
	 * | pubkey (32 bytes) | encrypted client token (32 bytes) |
	 *
	 * Total session size: 64 bytes
	 *
	 */

	/* All good */
	return 0;
}

int auth_client_remote_session_process(
	unsigned char *session,
	const char *username,
	const char *plain_passwd,
	unsigned char *context,
	unsigned char *agreed_key)
{
	int errsv = 0;
	unsigned char salt[HASH_DIGEST_SIZE_BLAKE2S];
	unsigned char salt_raw[CONFIG_USCHED_AUTH_USERNAME_MAX];
	unsigned char auth[KE_CLIENT_AUTH_SIZE_PANKAKE];

	/* Session contents:
	 *
	 * | pubkey (32 bytes) | encrypted server token (32 bytes) |
	 *
	 * Total session size: 64 bytes
	 *
	 */

	/* Check if username doesn't exceed the expected size */
	if (strlen(username) > sizeof(salt)) {
		log_warn("auth_client_remote_session_process(): strlen(username) > sizeof(salt)\n");
		errno = EINVAL;
		return -1;
	}

	/* Craft salt */
	memset(salt_raw, 'x', sizeof(salt_raw));
	memcpy(salt_raw, username, strlen(username));

	/* Hash raw salt */
	if (!hash_buffer_blake2s(salt, salt_raw, sizeof(salt_raw))) {
		errsv = errno;
		log_warn("auth_client_remote_session_process(): hash_buffer_blake2s(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	if (!ke_pankake_client_authorize(auth, context, agreed_key, session)) {
		errsv = errno;
		log_warn("auth_client_remote_session_process(): ke_pankake_client_authorize(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Set session to client auth */
	memcpy(session, auth, sizeof(auth));

	/* Session data contents:
	 *
	 * | encrypted password payload (1 byte + 256 bytes) |
	 *
	 * Total session size: 257 bytes
	 *
	 */

	/* All good */
	return 0;
}

