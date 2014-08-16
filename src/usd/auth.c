/**
 * @file auth.c
 * @brief uSched
 *        Authentication and Authorization interface - Daemon
 *
 * Date: 16-08-2014
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
#include <unistd.h>

#include <psec/crypt.h>
#include <psec/decode.h>
#include <psec/generate.h>
#include <psec/hash.h>

#include "debug.h"
#include "config.h"
#include "runtime.h"
#include "log.h"
#include "local.h"

int auth_daemon_local(int fd, uid_t *uid, gid_t *gid) {
	int errsv = 0;

	if (local_fd_peer_cred(fd, uid, gid) < 0) {
		errsv = errno;
		log_warn("auth_daemon_local(): local_fd_peer_cred(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	return 0;
}

int auth_daemon_remote_user_token_verify(
	const char *username,
	const char *session,
	unsigned char *nonce,
	const unsigned char *token,
	uid_t *uid,
	gid_t *gid)
{
	int errsv = 0;
	struct usched_config_userinfo *userinfo = NULL;
	unsigned char pwhash_c[HASH_DIGEST_SIZE_SHA512], pwhash_s[HASH_DIGEST_SIZE_SHA512 + 3];
	size_t out_len = 0;

	/* Session data contents:
	 *
	 * | nonce (24 bytes) | encrypted pwhash (16 + 64 bytes) |
	 *
	 * Total session size: 104 bytes
	 *
	 */

	/* Get userinfo data from current configuration */
	if (!(userinfo = rund.config.users.list->search(rund.config.users.list, (struct usched_config_userinfo [1]) { { (char *) username, NULL, NULL, 0, 0} }))) {
		errsv = errno;
		log_warn("auth_daemon_remote_user_token_verify(): No such username: %s\n", username);
		errno = errsv;
		return -1;
	}

	/* Extract nonce from the received session field */
	memcpy(nonce, session, CRYPT_NONCE_SIZE_XSALSA20);

	/* Decrypt client password hash with token as key */
	if (!crypt_decrypt_xsalsa20(pwhash_c, &out_len, (unsigned char *) (session + CRYPT_NONCE_SIZE_XSALSA20), HASH_DIGEST_SIZE_SHA512 + CRYPT_EXTRA_SIZE_XSALSA20, nonce, token)) {
		errsv = errno;
		log_warn("auth_daemon_remote_user_token_verify(): crypt_decrypt_xsalsa20(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Grant that userinfo->password doesn't exceed the expected length */
	if (decode_size_base64(strlen(userinfo->password)) > sizeof(pwhash_s)) {
		log_warn("auth_daemon_remote_user_token_verify(): pwhash_s buffer is too small to receive the decoded user password.\n");
		errno = EINVAL;
		return -1;
	}

	/* Decode the base64 encoded password hash from current configuration */
	if (!decode_buffer_base64(pwhash_s, &out_len, (unsigned char *) userinfo->password, strlen(userinfo->password))) {
		errsv = errno;
		log_warn("auth_daemon_remote_user_token_verify(): decode_buffer_base64(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Check if password hashes match */
	if (memcmp(pwhash_s, pwhash_c, HASH_DIGEST_SIZE_SHA512)) {
		log_warn("auth_daemon_remote_user_token_verify(): Server and Client password hashes do not match.\n");
		errno = EINVAL;
		return -1;
	}

	/* Set uid and gid */
	*uid = userinfo->uid;
	*gid = userinfo->gid;

	/* All good */
	return 0;
}

int auth_daemon_remote_user_token_create(
	const char *username,
	char *session,
	unsigned char *nonce,
	unsigned char *token)
{
	int errsv = 0;
	struct usched_config_userinfo *userinfo = NULL;
	unsigned char salt[10], pwhash[HASH_DIGEST_SIZE_SHA512 + 3];
	unsigned char key[CRYPT_KEY_SIZE_XSALSA20];
	size_t out_len = 0;

	/* Get userinfo data from current configuration */
	if (!(userinfo = rund.config.users.list->search(rund.config.users.list, (struct usched_config_userinfo [1]) { { (char *) username, NULL, NULL, 0, 0 } }))) {
		log_warn("auth_daemon_remote_user_token_create(): No such username: %s\n", username);
		errno = EINVAL;
		return -1;
	}

	/* Grant that userinfo->salt doesn't exceed the expected length */
	if (decode_size_base64(strlen(userinfo->salt)) > sizeof(salt)) {
		log_warn("auth_daemon_remote_user_token_verify(): salt buffer is too small to receive the decoded user salt.\n");
		errno = EINVAL;
		return -1;
	}

	/* Decode user password salt from base64 */
	if (!decode_buffer_base64(salt, &out_len, (unsigned char *) userinfo->salt, strlen(userinfo->salt))) {
		errsv = errno;
		log_warn("auth_daemon_remote_user_token_create(): decode_buffer_base64(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Grant that userinfo->password doesn't exceed the expected length */
	if (decode_size_base64(strlen(userinfo->password)) > sizeof(pwhash)) {
		log_warn("auth_daemon_remote_user_token_verify(): pwhash buffer is too small to receive the decoded user password.\n");
		errno = EINVAL;
		return -1;
	}

	/* Decode user password hash from base64 */
	if (!decode_buffer_base64(pwhash, &out_len, (unsigned char *) userinfo->password, strlen(userinfo->password))) {
		errsv = errno;
		log_warn("auth_daemon_remote_user_token_create(): decode_buffer_base64(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Generate a random session token */
	if (!generate_bytes_random(token, CRYPT_KEY_SIZE_XSALSA20)) {
		errsv = errno;
		log_warn("auth_daemon_remote_user_token_create(): generate_bytes_random(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Generate a random nonce for encryption */
	if (!generate_bytes_random(nonce, CRYPT_NONCE_SIZE_XSALSA20)) {
		errsv = errno;
		log_warn("auth_daemon_remote_user_token_create(): generate_bytes_random(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Shrink the password hash with a blake2s digest in order to match the encryption key size */
	if (!hash_buffer_blake2s(key, pwhash, sizeof(pwhash) - 3)) {
		errsv = errno;
		log_warn("auth_daemon_remote_user_token_create(): hash_buffer_blake2s(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Encrypt the session token with the resulting blake2s digest as key */
	if (!crypt_encrypt_xsalsa20((unsigned char *) (session + (sizeof(salt) - 2) + CRYPT_NONCE_SIZE_XSALSA20), &out_len, token, CRYPT_KEY_SIZE_XSALSA20, nonce, key)) {
		errsv = errno;
		log_warn("auth_daemon_remote_user_token_create(): crypt_encrypt_xsalsa20(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Craft session field */
	memcpy(session, salt, sizeof(salt) - 2);
	memcpy(session + sizeof(salt) - 2, nonce, CRYPT_NONCE_SIZE_XSALSA20);

	/* Session contents:
	 *
	 * | salt (8 bytes) | nonce (24 bytes) | encrypted token (16 + 32 bytes) |
	 *
	 * Total size of session field: 80 bytes
	 */

	return 0;
}

