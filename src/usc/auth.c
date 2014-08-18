/**
 * @file auth.c
 * @brief uSched
 *        Authentication and Authorization interface - Client
 *
 * Date: 18-08-2014
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

#include <psec/crypt.h>
#include <psec/generate.h>
#include <psec/hash.h>
#include <psec/kdf.h>

#include "runtime.h"
#include "log.h"
#include "auth.h"

static int _auth_client_remote_session_server_compute(
	unsigned char *out,
	unsigned char *server_token1,
	unsigned char *server_token2,
	unsigned char *server_token3,
	unsigned char *client_token,
	unsigned char *pwhash,
	unsigned char *dh_shared)
{
	return 0;
}

int auth_client_remote_session_token_create(
	char *session,
	const char *username,
	const char *plain_passwd,
	unsigned char *token)
{
	int errsv = 0, rounds = CONFIG_USCHED_SEC_KDF_ROUNDS;
	char *session_pos = session + sizeof(runc.sec.key_pub);
	unsigned char salt[CONFIG_USCHED_AUTH_USERNAME_MAX];
	unsigned char pwhash[HASH_DIGEST_SIZE_SHA512];
	unsigned char key[HASH_DIGEST_SIZE_BLAKE2S];
	size_t out_len = 0;

	/* Check if username doesn't exceed the expected size */
	if (strlen(username) > sizeof(salt)) {
		log_warn("auth_client_remote_session_token_create(): strlen(username) > sizeof(salt)\n");
		errno = EINVAL;
		return -1;
	}

	/* Craft salt */
	memset(salt, 'x', sizeof(salt));
	memcpy(salt, username, strlen(username));

	/* Create a password hash with the same parameters as remote party */
	if (!kdf_pbkdf2_hash(pwhash, hash_buffer_sha512, HASH_DIGEST_SIZE_SHA512, HASH_BLOCK_SIZE_SHA512, (unsigned char *) plain_passwd, strlen(plain_passwd), salt, sizeof(salt), rounds, HASH_DIGEST_SIZE_SHA512) < 0) {
		errsv = errno;
		log_warn("auth_client_remote_session_token_create(): kdf_pbkdf2_hash(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Generate a new client token */
	if (!generate_bytes_random(token, HASH_DIGEST_SIZE_BLAKE2S)) {
		errsv = errno;
		log_warn("auth_client_remote_session_token_create(): generate_bytes_random(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Re-hash the result to match the encryption key size (32 bytes) */
	if (!hash_buffer_blake2s(key, pwhash, sizeof(pwhash))) {
		errsv = errno;
		log_warn("auth_client_remote_session_token_create(): hash_buffer_blake2s(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Encrypt the token with the rehashed pwhash as key */
	if (!crypt_encrypt_otp((unsigned char *) session_pos, &out_len, token, HASH_DIGEST_SIZE_BLAKE2S, NULL, key)) {
		errsv = errno;
		log_warn("auth_client_remote_session_token_create(): crypt_encrypt_otp(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Session contents:
	 *
	 * | pubkey (512 bytes) | encrypted token (32 bytes) |
	 *
	 * Total session size: 544 bytes
	 *
	 */

	/* TODO: Cleanup data by memset()ing all arrays to 0 */

	/* All good */
	return 0;
}

int auth_client_remote_session_token_process(
	char *session,
	const char *username,
	const char *plain_passwd,
	unsigned char *dh_shared,
	size_t dh_shared_size,
	unsigned char *nonce,
	unsigned char *token)
{
	int errsv = 0, rounds = CONFIG_USCHED_SEC_KDF_ROUNDS;
	char *session_pos = session + sizeof(runc.sec.key_pub);
	unsigned char salt[CONFIG_USCHED_AUTH_USERNAME_MAX];
	unsigned char pwhash[HASH_DIGEST_SIZE_SHA512];
	unsigned char key[CRYPT_KEY_SIZE_XSALSA20];
	unsigned char client_hash[HASH_DIGEST_SIZE_BLAKE2S];
	unsigned char server_token1[HASH_DIGEST_SIZE_BLAKE2S];
	unsigned char server_token2[HASH_DIGEST_SIZE_BLAKE2S];
	unsigned char server_token3[HASH_DIGEST_SIZE_BLAKE2S];
	unsigned char server_recvd_session[HASH_DIGEST_SIZE_BLAKE2S];
	unsigned char server_computed_session[HASH_DIGEST_SIZE_BLAKE2S];
	unsigned char pw_payload[CONFIG_USCHED_AUTH_PASSWORD_MAX + 1];
	size_t out_len = 0, pw_len = 0;

	/* Session contents:
	 *
	 * | pubkey (512 bytes) | nonce (24 bytes) | encrypted server token3 (16 + 32 bytes) |
	 *
	 * Total session size: 584 bytes
	 *
	 */

	/* Check if username doesn't exceed the expected size */
	if (strlen(username) > sizeof(salt)) {
		log_warn("auth_client_remote_session_token_process(): strlen(username) > sizeof(salt)\n");
		errno = EINVAL;
		return -1;
	}

	/* Craft salt */
	memset(salt, 'x', sizeof(salt));
	memcpy(salt, username, strlen(username));

	/* Extract nonce from session */
	memcpy(nonce, session_pos, CRYPT_NONCE_SIZE_XSALSA20);
	memcpy(server_recvd_session, session_pos + CRYPT_NONCE_SIZE_XSALSA20, HASH_DIGEST_SIZE_BLAKE2S);

	/* Shrink the shared key with a blake2s digest in order to match the encryption key size */
	if (!hash_buffer_blake2s(key, dh_shared, dh_shared_size)) {
		errsv = errno;
		log_warn("auth_daemon_remote_session_token_process(): hash_buffer_blake2s(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Decrypt server session */
	if (!crypt_decrypt_xsalsa20(server_token3, &out_len, server_recvd_session, CRYPT_KEY_SIZE_XSALSA20, nonce, key)) {
		errsv = errno;
		log_warn("auth_daemon_remote_session_token_process(): crypt_decrypt_xsalsa20(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Create a password hash with the same parameters as remote party */
	if (!kdf_pbkdf2_hash(pwhash, hash_buffer_sha512, HASH_DIGEST_SIZE_SHA512, HASH_BLOCK_SIZE_SHA512, (unsigned char *) plain_passwd, strlen(plain_passwd), salt, sizeof(salt), rounds, HASH_DIGEST_SIZE_SHA512) < 0) {
		errsv = errno;
		log_warn("auth_client_remote_session_token_process(): kdf_pbkdf2_hash(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Compute a client hash based on client token and pwhash as salt */
	if (!kdf_pbkdf2_hash(client_hash, hash_buffer_blake2s, HASH_DIGEST_SIZE_BLAKE2S, HASH_BLOCK_SIZE_BLAKE2S, token, HASH_DIGEST_SIZE_BLAKE2S, pwhash, sizeof(pwhash), rounds, HASH_DIGEST_SIZE_BLAKE2S) < 0) {
		errsv = errno;
		log_warn("auth_client_remote_session_token_process(): kdf_pbkdf2_hash(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Decrypt server token3 with client hash as key to achieve server token2 */
	if (!crypt_decrypt_otp(server_token2, &out_len, server_token3, HASH_DIGEST_SIZE_BLAKE2S, NULL, client_hash)) {
		errsv = errno;
		log_warn("auth_client_remote_session_token_process(): crypt_decrypt_otp(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Re-hash the pwhash to achieve the decryption key for server token2 */
	if (!hash_buffer_blake2s(key, pwhash, sizeof(pwhash))) {
		errsv = errno;
		log_warn("auth_client_remote_session_token_process(): hash_buffer_blake2s(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Decrypt server token2 with re-hashed version of pwhash to achieve server token1 */
	if (!crypt_decrypt_otp(server_token1, &out_len, server_token2, HASH_DIGEST_SIZE_BLAKE2S, NULL, key)) {
		errsv = errno;
		log_warn("auth_client_remote_session_token_process(): crypt_decrypt_otp(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Reconstruct server session */
	if (_auth_client_remote_session_server_compute(server_computed_session, server_token1, server_token2, server_token3, token, pwhash, dh_shared) < 0) {
		errsv = errno;
		log_warn("auth_client_remote_session_token_process(): _auth_client_remote_session_server_compute(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Compare session values */
	if (memcmp(server_recvd_session, server_computed_session, HASH_DIGEST_SIZE_BLAKE2S)) {
		log_warn("auth_client_remote_session_token_process(): Session data received from server doesn't match the local computed session.\n");
		errno = EINVAL;
		return -1;
	}

	/* Shrink the shared key with a blake2s digest in order to match the encryption key size */
	if (!hash_buffer_blake2s(key, dh_shared, dh_shared_size)) {
		errsv = errno;
		log_warn("auth_daemon_remote_session_token_process(): hash_buffer_blake2s(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Generate a new nonce value */
	if (!generate_bytes_random(nonce, CRYPT_NONCE_SIZE_XSALSA20)) {
		errsv = errno;
		log_warn("auth_client_remote_session_token_process(): generate_bytes_random(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Reset session data */
	if (!generate_bytes_random((unsigned char *) session, CONFIG_USCHED_AUTH_SESSION_MAX)) {
		errsv = errno;
		log_warn("auth_client_remote_session_token_process(): generate_bytes_random(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Check password length */
	if ((pw_len = strlen(plain_passwd)) > (sizeof(pw_payload) - 1)) {
		log_warn("auth_client_remote_session_token_process(): Password too long.\n");
		errno = EINVAL;
		return -1;
	}

	/* Reset pw_payload data */
	if (!generate_bytes_random(pw_payload, sizeof(pw_payload))) {
		errsv = errno;
		log_warn("auth_client_remote_session_token_process(): generate_bytes_random(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Craft plain password payload */
	memcpy(pw_payload, (uint8_t [1]) { pw_len }, 1);
	memcpy(pw_payload + 1, plain_passwd, pw_len);

	/* Encrypt the user password hash with the session token as key */
	if (!crypt_encrypt_xsalsa20((unsigned char *) (session + CRYPT_NONCE_SIZE_XSALSA20), &out_len, pw_payload, sizeof(pw_payload), nonce, key)) {
		errsv = errno;
		log_warn("auth_client_remote_session_token_process(): crypt_encrypt_xsalsa20(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Set nonce value to the head of session */
	memcpy(session, nonce, CRYPT_NONCE_SIZE_XSALSA20);

	/* Session data contents:
	 *
	 * | nonce (24 bytes) | encrypted pw payload (16 bytes + 1 byte + 256 bytes) |
	 *
	 * Total session size: 297 bytes
	 *
	 */

	/* TODO: Cleanup data by memset()ing all arrays to 0 */

	return 0;
}

