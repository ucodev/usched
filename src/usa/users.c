/**
 * @file users.c
 * @brief uSched
 *        Users configuration interface
 *
 * Date: 06-08-2014
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
#include <time.h>

#include <sys/types.h>
#include <unistd.h>

#include <psec/encode.h>
#include <psec/hash.h>
#include <psec/kdf.h>

#include "debug.h"
#include "config.h"
#include "runtime.h"
#include "log.h"

int users_admin_config_add(const char *username, uid_t uid, gid_t gid, const char *password) {
	int errsv = 0, len = 0, rounds = 5000;
	char digest[HASH_DIGEST_SIZE_SHA512];
	char *encoded_digest = NULL, *result = NULL, *encoded_salt = NULL, *path = NULL;
	uint64_t salt = 0;
	size_t edigest_out_len = 0, esalt_out_len = 0;
	FILE *fp = NULL;

	/* Grant that username isn't empty */
	if (!username || !username[0]) {
		log_warn("users_admin_config_add(): Username is an empty string.\n");
		errno = EINVAL;
		return -1;
	}

	/* Grant that password isn't empty */
	if (!password || !password[0]) {
		log_warn("users_admin_config_add(): Password is an empty string.\n");
		errno = EINVAL;
		return -1;
	}

	/* Initialize (weak) random generator */
	srandom((time(NULL) + 3) * (clock() + 5) * (getpid() + 7) * (getppid() + 11) + uid + gid + username[0] + username[1] + password[0] + password[1] + (strlen(username) * strlen(password)));

	/* Get some (weak) random value, different than 0 */
	while (!salt)
		salt = (random() + 3) * (random() + 5) * (random() + 7) * (random() + 11);


	/* Generate the password digest */
	if (!kdf_pbkdf2_hash(digest, hash_buffer_sha512, HASH_DIGEST_SIZE_SHA512, HASH_BLOCK_SIZE_SHA512, password, strlen(password), (const char *) &salt, sizeof(salt), rounds, HASH_DIGEST_SIZE_SHA512)) {
		errsv = errno;
		log_warn("users_admin_config_add(): kdf_pbkdf2_hash(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Encode the salt */
	if (!(encoded_salt = encode_buffer_base64(NULL, &esalt_out_len, (const char *) &salt, sizeof(salt)))) {
		errsv = errno;
		log_warn("users_admin_config_add(): encode_buffer_base64(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Remove the leading '=' from the encoded_salt */
	while (encoded_salt[esalt_out_len - 1] == '=')
		encoded_salt[-- esalt_out_len] = 0;

	/* Encode the digest */
	if (!(encoded_digest = encode_buffer_base64(NULL, &edigest_out_len, digest, HASH_DIGEST_SIZE_SHA512))) {
		errsv = errno;
		log_warn("users_admin_config_add(): encode_buffer_base64(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Remove the leading '=' from the encoded_digest */
	while (encoded_digest[edigest_out_len - 1] == '=')
		encoded_digest[-- edigest_out_len] = 0;

	/* Allocate enough memory for the final result */
	len = 20 + 1 + 20 + 1 + esalt_out_len + 1 + edigest_out_len + 1  + 1;
	/*    uid  :   gid  :   encoded_salt    $   encoded_digest    \0   \0 */

	if (!(result = mm_alloc(len))) {
		errsv = errno;
		log_warn("users_admin_config_add(): mm_alloc(): %s\n", strerror(errno));
		encode_destroy(encoded_salt);
		encode_destroy(encoded_digest);
		errno = errsv;
		return -1;
	}

	/* Reset result memory */
	memset(result, 0, len);

	/* Craft the result string */
	snprintf(result, len - 1, "%u:%u:%s$%s", uid, gid, encoded_salt, encoded_digest);

	/* Free unused memory */
	encode_destroy(encoded_salt);
	encode_destroy(encoded_digest);

	/* Allocate filename path memory */
	len = sizeof(CONFIG_USCHED_DIR_BASE) + sizeof(CONFIG_USCHED_DIR_USERS) + strlen(username) + 1 + 1;
	/*    CONFIG_USCHED_DIR_BASE         / CONFIG_USCHED_DIRS_USERS        / username           \0  \0 */

	if (!(path = mm_alloc(len))) {
		errsv = errno;
		log_warn("users_admin_config_add(): mm_alloc(): %s\n", strerror(errno));
		mm_free(result);
		errno = errsv;
		return -1;
	}

	/* Reset path memory */
	memset(path, 0, len);

	/* Craft the filename path */
	snprintf(path, len - 1, "%s/%s/%s", CONFIG_USCHED_DIR_BASE, CONFIG_USCHED_DIR_USERS, username);

	/* Open the file for writting */
	if (!(fp = fopen(path, "w"))) {
		errsv = errno;
		log_warn("users_admin_config_add(): fopen(\"%s\", \"w\"): %s", path, strerror(errno));
		mm_free(result);
		mm_free(path);
		errno = errsv;
		return -1;
	}

	/* Write the result into the user filename */
	fprintf(fp, "%s", result);

	/* Flush the file */
	fflush(fp);

	/* Close the file */
	fclose(fp);

	/* Debug info */
	debug_printf(DEBUG_INFO, "%s\n", path);
	debug_printf(DEBUG_INFO, "%s\n", result);

	mm_free(path);
	mm_free(result);

	return 0;
}

int users_admin_config_delete(const char *username) {
	errno = ENOSYS;
	return -1;
}

int users_admin_config_change(const char *username, uid_t uid, gid_t gid, const char *password) {
	errno = ENOSYS;
	return -1;
}

int users_admin_config_show(void) {
	errno = ENOSYS;
	return -1;
}

