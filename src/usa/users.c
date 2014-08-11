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

#include <sys/types.h>

#include <fsop/path.h>

#include <psec/encode.h>
#include <psec/generate.h>
#include <psec/hash.h>
#include <psec/kdf.h>

#include "debug.h"
#include "config.h"
#include "runtime.h"
#include "log.h"
#include "print.h"

int users_admin_config_add(const char *username, uid_t uid, gid_t gid, const char *password) {
	int errsv = 0, len = 0, rounds = 5000;
	unsigned char digest[HASH_DIGEST_SIZE_SHA512];
	unsigned char *encoded_digest = NULL, *encoded_salt = NULL;
	char *result = NULL, *path = NULL;
	unsigned char salt[8];
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

	/* Generate random salt */
	if (!generate_bytes_random(salt, sizeof(salt))) {
		errsv = errno;
		log_warn("users_admin_config_add(): generate_bytes_random(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Generate the password digest */
	if (!kdf_pbkdf2_hash(digest, hash_buffer_sha512, HASH_DIGEST_SIZE_SHA512, HASH_BLOCK_SIZE_SHA512, (const unsigned char *) password, strlen(password), salt, sizeof(salt), rounds, HASH_DIGEST_SIZE_SHA512)) {
		errsv = errno;
		log_warn("users_admin_config_add(): kdf_pbkdf2_hash(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Encode the salt */
	if (!(encoded_salt = encode_buffer_base64(NULL, &esalt_out_len, salt, sizeof(salt)))) {
		errsv = errno;
		log_warn("users_admin_config_add(): encode_buffer_base64(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Remove the leading '=' from the encoded_salt */
	for (-- esalt_out_len; encoded_salt[esalt_out_len - 1] == (unsigned char) '='; )
		encoded_salt[-- esalt_out_len] = 0;

	/* Encode the digest */
	if (!(encoded_digest = encode_buffer_base64(NULL, &edigest_out_len, digest, HASH_DIGEST_SIZE_SHA512))) {
		errsv = errno;
		log_warn("users_admin_config_add(): encode_buffer_base64(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Remove the leading '=' from the encoded_digest */
	for (-- edigest_out_len; encoded_digest[edigest_out_len - 1] == (unsigned char) '='; )
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
	snprintf(result, len - 1, "%u:%u:%s$%s", uid, gid, (char *) encoded_salt, (char *) encoded_digest);

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

	/* Check if file exists */
	if (fsop_path_exists(path)) {
		log_warn("users_admin_config_add(): User \'%s\' already exists.\n", username);
		mm_free(result);
		mm_free(path);
		errno = EEXIST;
		return -1;
	}

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

	/* Free unused memory */
	mm_free(result);
	mm_free(path);

	/* All good */
	return 0;
}

int users_admin_config_delete(const char *username) {
	int errsv = 0, len = 0;
	char *path = NULL;

	/* Allocate filename path memory */
	len = sizeof(CONFIG_USCHED_DIR_BASE) + sizeof(CONFIG_USCHED_DIR_USERS) + strlen(username) + 1 + 1;
	/*    CONFIG_USCHED_DIR_BASE         / CONFIG_USCHED_DIRS_USERS        / username           \0  \0 */

	if (!(path = mm_alloc(len))) {
		errsv = errno;
		log_warn("users_admin_config_delete(): mm_alloc(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Reset path memory */
	memset(path, 0, len);

	/* Craft the filename path */
	snprintf(path, len - 1, "%s/%s/%s", CONFIG_USCHED_DIR_BASE, CONFIG_USCHED_DIR_USERS, username);

	/* Delete the file */
	if (unlink(path) < 0) {
		errsv = errno;
		log_warn("users_admin_config_delete(): mm_alloc(): %s\n", strerror(errno));
		mm_free(path);
		errno = errsv;
		return -1;
	}

	/* Debug info */
	debug_printf(DEBUG_INFO, "%s\n", path);

	/* Free unused memory */
	mm_free(path);

	/* All good */
	return 0;
}

int users_admin_config_change(const char *username, uid_t uid, gid_t gid, const char *password) {
	int errsv = 0;

	/* Delete the user */
	if (users_admin_config_delete(username) < 0) {
		errsv = errno;
		log_warn("users_admin_config_change(): users_admin_config_delete(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Add the user */
	if (users_admin_config_add(username, uid, gid, password) < 0) {
		errsv = errno;
		log_warn("users_admin_config_change(): users_admin_config_add(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* All good */
	return 0;
}

int users_admin_config_show(void) {
	print_admin_config_users(&runa.config.users);

	return 0;
}

