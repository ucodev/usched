/**
 * @file users.c
 * @brief uSched
 *        Users configuration and administration interface
 *
 * Date: 20-02-2015
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
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <fsop/path.h>
#include <fsop/file.h>
#include <fsop/dir.h>

#include <psec/encode.h>
#include <psec/hash.h>
#include <psec/kdf.h>
#include <psec/mac.h>

#include "debug.h"
#include "config.h"
#include "runtime.h"
#include "log.h"
#include "print.h"

static int _cleanup_action(
	int order,
	const char *fpath,
	const char *rpath,
	void *arg)
{
	struct stat st;

	if (order == FSOP_WALK_INORDER) {
		if (stat(fpath, &st) < 0)
			return -1;

		/* Delete file if it's empty */
		if (S_ISREG(st.st_mode) && !st.st_size) {
			if (unlink(fpath) < 0)
				return -1;
		}
	}

	/* All good */
	return 0;
}


static int _commit_action(
	int order,
	const char *fpath,
	const char *rpath,
	void *arg)
{
	char *new_path = NULL;

	if ((order == FSOP_WALK_INORDER) && (rpath[0] == '.')) {
		if (!(new_path = mm_alloc(strlen(fpath) + 1)))
			return -1;

		(strrchr(strcpy(new_path, fpath), '/') + 1)[0] = 0;
		strcat(new_path, &rpath[1]);

		if (fsop_cp(fpath, new_path, 128) < 0)
			return -1;
	}

	/* All good */
	return 0;
}

static int _rollback_action(
	int order,
	const char *fpath,
	const char *rpath,
	void *arg)
{
	char *new_path = NULL;

	if ((order == FSOP_WALK_INORDER) && (rpath[0] != '.')) {
		if (!(new_path = mm_alloc(strlen(fpath) + 2)))
			return -1;

		(strrchr(strcpy(new_path, fpath), '/') + 1)[0] = 0;
		strcat(new_path, ".");
		strcat(new_path, rpath);

		if (fsop_cp(fpath, new_path, 128) < 0)
			return -1;
	}

	/* All good */
	return 0;
}

int users_admin_commit(void) {
	int errsv = 0;

	if (fsop_walkdir(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_USERS, NULL, &_commit_action, NULL) < 0) {
		errsv = errno;
		log_crit("users_admin_commit(): fsop_walkdir(... &_commit_action): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	if (fsop_walkdir(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_USERS, NULL, &_cleanup_action, NULL) < 0) {
		errsv = errno;
		log_crit("users_admin_commit(): fsop_walkdir(... &_cleanup_action): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* All good */
	return 0;
}

int users_admin_rollback(void) {
	int errsv = 0;

	if (fsop_walkdir(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_USERS, NULL, &_rollback_action, NULL) < 0) {
		errsv = errno;
		log_crit("users_admin_rollback(): fsop_walkdir(... &_cleanup_action): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	if (fsop_walkdir(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_USERS, NULL, &_cleanup_action, NULL) < 0) {
		errsv = errno;
		log_crit("users_admin_rollback(): fsop_walkdir(... &_cleanup_action): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* All good */
	return 0;
}

int users_admin_add(const char *username, uid_t uid, gid_t gid, const char *password) {
	int errsv = 0, len = 0, rounds = CONFIG_USCHED_SEC_KDF_ROUNDS;
	unsigned char digest[HASH_DIGEST_SIZE_SHA512];
	unsigned char *encoded_digest = NULL, *encoded_salt = NULL;
	char *result = NULL, *tmp_path = NULL, *path = NULL;
	unsigned char salt[HASH_DIGEST_SIZE_BLAKE2S];
	unsigned char salt_raw[CONFIG_USCHED_AUTH_USERNAME_MAX];
	size_t edigest_out_len = 0, esalt_out_len = 0;
	FILE *fp = NULL;

	/* Grant that username isn't empty */
	if (!username || !username[0]) {
		log_warn("users_admin_add(): Username is an empty string.\n");
		errno = EINVAL;
		return -1;
	}

	/* Grant that password isn't empty */
	if (!password || !password[0]) {
		log_warn("users_admin_add(): Password is an empty string.\n");
		errno = EINVAL;
		return -1;
	}

	/* Check password length */
	if (strlen(password) < CONFIG_USCHED_AUTH_PASSWORD_MIN) {
		log_warn("users_admin_add(): Password is too short (it must be at least 8 characters long).\n");
		errno = EINVAL;
		return -1;
	}

	/* Check if username doesn't exceed the expected size */
	if (strlen(username) > sizeof(salt)) {
		log_warn("users_admin_add(): Username is too long.\n");
		errno = EINVAL;
		return -1;
	}

	/* Craft raw salt */
	memset(salt_raw, 'x', sizeof(salt_raw));
	memcpy(salt_raw, username, strlen(username));

	/* Hash raw salt */
	if (!hash_buffer_blake2s(salt, salt_raw, sizeof(salt_raw))) {
		errsv = errno;
		log_warn("users_admin_add(): hash_buffer_blake2s(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Generate the password digest */
	if (!kdf_pbkdf2_sha512(digest, (const unsigned char *) password, strlen(password), salt, sizeof(salt), rounds, HASH_DIGEST_SIZE_SHA512)) {
		errsv = errno;
		log_warn("users_admin_add(): kdf_pbkdf2_hash(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Encode the salt */
	if (!(encoded_salt = encode_buffer_base64(NULL, &esalt_out_len, salt, sizeof(salt)))) {
		errsv = errno;
		log_warn("users_admin_add(): encode_buffer_base64(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Remove the leading '=' from the encoded_salt */
	for (-- esalt_out_len; encoded_salt[esalt_out_len - 1] == (unsigned char) '='; )
		encoded_salt[-- esalt_out_len] = 0;

	/* Encode the digest */
	if (!(encoded_digest = encode_buffer_base64(NULL, &edigest_out_len, digest, HASH_DIGEST_SIZE_SHA512))) {
		errsv = errno;
		log_warn("users_admin_add(): encode_buffer_base64(): %s\n", strerror(errno));
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
		log_warn("users_admin_add(): mm_alloc(): %s\n", strerror(errno));
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
	len = sizeof(CONFIG_USCHED_DIR_BASE) + sizeof(CONFIG_USCHED_DIR_USERS) + 1 + strlen(username) + 1 + 1;
	/*    CONFIG_USCHED_DIR_BASE         / CONFIG_USCHED_DIRS_USERS        / .   username           \0  \0 */

	if (!(tmp_path = mm_alloc(len))) {
		errsv = errno;
		log_warn("users_admin_add(): mm_alloc(): %s\n", strerror(errno));
		mm_free(result);
		errno = errsv;
		return -1;
	}

	if (!(path = mm_alloc(len))) {
		errsv = errno;
		log_warn("users_admin_add(): mm_alloc(): %s\n", strerror(errno));
		mm_free(result);
		mm_free(tmp_path);
		errno = errsv;
		return -1;
	}

	/* Reset path memory */
	memset(tmp_path, 0, len);
	memset(path, 0, len);

	/* Craft the filename path */
	snprintf(tmp_path, len - 1, "%s/%s/.%s", CONFIG_USCHED_DIR_BASE, CONFIG_USCHED_DIR_USERS, username);
	snprintf(path, len - 1, "%s/%s/%s", CONFIG_USCHED_DIR_BASE, CONFIG_USCHED_DIR_USERS, username);

	/* Check if file exists */
	if (fsop_path_exists(tmp_path)) {
		log_warn("users_admin_add(): User \'%s\' already exists.\n", username);
		mm_free(result);
		mm_free(tmp_path);
		mm_free(path);
		errno = EEXIST;
		return -1;
	}

	/* Open the file for writting */
	if (!(fp = fopen(tmp_path, "w"))) {
		errsv = errno;
		log_warn("users_admin_add(): fopen(\"%s\", \"w\"): %s", tmp_path, strerror(errno));
		mm_free(result);
		mm_free(tmp_path);
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

	/* Create an empty file for effective user configuration */
	if (!(fp = fopen(path, "w"))) {
		errsv = errno;
		log_warn("users_admin_add(): fopen(\"%s\", \"w\"): %s", path, strerror(errno));
		mm_free(result);
		mm_free(tmp_path);
		mm_free(path);
		errno = errsv;
		return -1;
	}

	/* Close the file */
	fclose(fp);

	/* Debug info */
	debug_printf(DEBUG_INFO, "%s\n", tmp_path);
	debug_printf(DEBUG_INFO, "%s\n", path);
	debug_printf(DEBUG_INFO, "%s\n", result);

	/* Free unused memory */
	mm_free(result);
	mm_free(tmp_path);
	mm_free(path);

	/* All good */
	return 0;
}

static int _users_admin_delete_generic(const char *username, int create_empty_file) {
	int errsv = 0, len = 0;
	char *path = NULL;
	FILE *fp = NULL;

	/* Allocate filename path memory */
	len = sizeof(CONFIG_USCHED_DIR_BASE) + sizeof(CONFIG_USCHED_DIR_USERS) + 1 + strlen(username) + 1 + 1;
	/*    CONFIG_USCHED_DIR_BASE         / CONFIG_USCHED_DIRS_USERS        / .   username           \0  \0 */

	if (!(path = mm_alloc(len))) {
		errsv = errno;
		log_warn("users_admin_delete(): mm_alloc(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Reset path memory */
	memset(path, 0, len);

	/* Craft the filename path */
	snprintf(path, len - 1, "%s/%s/.%s", CONFIG_USCHED_DIR_BASE, CONFIG_USCHED_DIR_USERS, username);

	/* Delete the file */
	if (unlink(path) < 0) {
		errsv = errno;
		log_warn("users_admin_delete(): mm_alloc(): %s\n", strerror(errno));
		mm_free(path);
		errno = errsv;
		return -1;
	}

	if (create_empty_file) {
		/* Create an empty file for the current (temporary) user configuration */
		if (!(fp = fopen(path, "w"))) {
			errsv = errno;
			log_warn("users_admin_delete(): fopen(\"%s\", \"w\"): %s", path, strerror(errno));
			mm_free(path);
			errno = errsv;
			return -1;
		}

		/* Close the file */
		fclose(fp);
	}

	/* Debug info */
	debug_printf(DEBUG_INFO, "%s\n", path);

	/* Free unused memory */
	mm_free(path);

	/* All good */
	return 0;
}

int users_admin_delete(const char *username) {
	return _users_admin_delete_generic(username, 1);
}

int users_admin_change(const char *username, uid_t uid, gid_t gid, const char *password) {
	int errsv = 0;

	/* Delete the user */
	if (_users_admin_delete_generic(username, 0) < 0) {
		errsv = errno;
		log_warn("users_admin_change(): users_admin_delete(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Add the user */
	if (users_admin_add(username, uid, gid, password) < 0) {
		errsv = errno;
		log_warn("users_admin_change(): users_admin_add(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* All good */
	return 0;
}

int users_admin_show(void) {
	print_admin_config_users(&runa.config.users);

	return 0;
}

