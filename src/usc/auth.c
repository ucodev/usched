/**
 * @file auth.c
 * @brief uSched
 *        Authentication and Authorization interface - Client
 *
 * Date: 19-08-2014
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

int auth_client_remote_session_token_create(
	char *session,
	const char *username,
	const char *plain_passwd,
	unsigned char *token)
{
	int errsv = 0, rounds = CONFIG_USCHED_SEC_KDF_ROUNDS;
	char *session_pos = session + sizeof(runc.sec.key_pub);
	unsigned char salt[HASH_DIGEST_SIZE_BLAKE2S];
	unsigned char salt_raw[CONFIG_USCHED_AUTH_USERNAME_MAX];
	unsigned char pwhash[HASH_DIGEST_SIZE_SHA512];
	unsigned char key[HASH_DIGEST_SIZE_BLAKE2S];
	size_t out_len = 0;

	/* Check if username doesn't exceed the expected size */
	if (strlen(username) > sizeof(salt)) {
		log_warn("auth_client_remote_session_token_create(): strlen(username) > sizeof(salt)\n");
		errno = EINVAL;
		return -1;
	}

	/* Craft raw salt */
	memset(salt_raw, 'x', sizeof(salt_raw));
	memcpy(salt_raw, username, strlen(username));

	/* Hash raw salt */
	if (!hash_buffer_blake2s(salt, salt_raw, sizeof(salt_raw))) {
		errsv = errno;
		log_warn("auth_client_remote_session_token_create(): hash_buffer_blake2s(): %s\n", strerror(errno));
		errno = errno;
		return -1;
	}

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

	/* Cleanup data */
	memset(key, 0, sizeof(key));
	memset(pwhash, 0, sizeof(pwhash));

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
	unsigned char salt[HASH_DIGEST_SIZE_BLAKE2S];
	unsigned char salt_raw[CONFIG_USCHED_AUTH_USERNAME_MAX];
	unsigned char pwhash_c[HASH_DIGEST_SIZE_SHA512];
	unsigned char pwhash_s[HASH_DIGEST_SIZE_BLAKE2S]; /* Server pwhash is a re-hashed version */
	unsigned char key[CRYPT_KEY_SIZE_XSALSA20];
	unsigned char key_agreed[HASH_DIGEST_SIZE_BLAKE2S];
	unsigned char client_hash[HASH_DIGEST_SIZE_BLAKE2S];
	unsigned char client_hash_tmp[HASH_DIGEST_SIZE_SHA512 * 2];
	unsigned char server_token[HASH_DIGEST_SIZE_BLAKE2S];
	unsigned char server_recvd_session[HASH_DIGEST_SIZE_BLAKE2S + CRYPT_EXTRA_SIZE_XSALSA20POLY1305];
	unsigned char pw_payload[CONFIG_USCHED_AUTH_PASSWORD_MAX + 1];
	size_t out_len = 0, pw_len = 0;

	/* Session contents:
	 *
	 * | pubkey (512 bytes) | nonce (24 bytes) | encrypted server token (16 + 32 bytes) |
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
	memset(salt_raw, 'x', sizeof(salt_raw));
	memcpy(salt_raw, username, strlen(username));

	/* Hash raw salt */
	if (!hash_buffer_blake2s(salt, salt_raw, sizeof(salt_raw))) {
		errsv = errno;
		log_warn("auth_client_remote_session_token_process(): hash_buffer_blake2s(): %s\n", strerror(errno));
		errno = errno;
		return -1;
	}

	/* Extract nonce from session */
	memcpy(nonce, session_pos, CRYPT_NONCE_SIZE_XSALSA20);
	memcpy(server_recvd_session, session_pos + CRYPT_NONCE_SIZE_XSALSA20, HASH_DIGEST_SIZE_BLAKE2S + CRYPT_EXTRA_SIZE_XSALSA20POLY1305);

	/* Shrink the shared key with a blake2s digest in order to match the encryption key size */
	if (!hash_buffer_blake2s(key, dh_shared, dh_shared_size)) {
		errsv = errno;
		log_warn("auth_daemon_remote_session_token_process(): hash_buffer_blake2s(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Decrypt server session */
	if (!crypt_decrypt_xsalsa20poly1305(server_token, &out_len, server_recvd_session, sizeof(server_token) + CRYPT_EXTRA_SIZE_XSALSA20POLY1305, nonce, key)) {
		errsv = errno;
		log_warn("auth_daemon_remote_session_token_process(): crypt_decrypt_xsalsa20poly1305(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Create a password hash with the same parameters as remote party */
	if (!kdf_pbkdf2_hash(pwhash_c, hash_buffer_sha512, HASH_DIGEST_SIZE_SHA512, HASH_BLOCK_SIZE_SHA512, (unsigned char *) plain_passwd, strlen(plain_passwd), salt, sizeof(salt), rounds, HASH_DIGEST_SIZE_SHA512) < 0) {
		errsv = errno;
		log_warn("auth_client_remote_session_token_process(): kdf_pbkdf2_hash(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Compute a client hash based on pwhash and client token as salt */
	if (!kdf_pbkdf2_hash(client_hash_tmp, hash_buffer_sha512, HASH_DIGEST_SIZE_SHA512, HASH_BLOCK_SIZE_SHA512, pwhash_c, sizeof(pwhash_c), token, HASH_DIGEST_SIZE_BLAKE2S, rounds, HASH_DIGEST_SIZE_SHA512 * 2) < 0) {
		errsv = errno;
		log_warn("auth_client_remote_session_token_process(): kdf_pbkdf2_hash(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Shrink the temporary client hash */
	if (!hash_buffer_blake2s(client_hash, client_hash_tmp, HASH_DIGEST_SIZE_SHA512 * 2)) {
		errsv = errno;
		log_warn("auth_client_remote_session_token_process(): hash_buffer_blake2s(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Decrypt server token3 with client hash as key to achieve server token2 */
	if (!crypt_decrypt_otp(pwhash_s, &out_len, server_token, HASH_DIGEST_SIZE_BLAKE2S, NULL, client_hash)) {
		errsv = errno;
		log_warn("auth_client_remote_session_token_process(): crypt_decrypt_otp(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Re-hash the pwhash to achieve the decryption key for client token */
	if (!hash_buffer_blake2s(key, pwhash_c, sizeof(pwhash_c))) {
		errsv = errno;
		log_warn("auth_client_remote_session_token_process(): hash_buffer_blake2s(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Compare Server and Client hashes */
	if (memcmp(pwhash_s, key, HASH_DIGEST_SIZE_BLAKE2S)) {
		log_warn("auth_client_remote_session_token_process(): Server and client password hash doesn't match.\n");
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

	/* Encrypt the re-hashed DH shared key with the re-hashed pwhash to obain the agreed key */
	if (!crypt_encrypt_otp(key_agreed, &out_len, pwhash_s, sizeof(pwhash_s), NULL, key)) {
		errsv = errno;
		log_warn("auth_client_remote_session_token_process(): crypt_encrypt_otp(): %s\n", strerror(errno));
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
	if (!crypt_encrypt_xsalsa20poly1305((unsigned char *) (session + CRYPT_NONCE_SIZE_XSALSA20), &out_len, pw_payload, sizeof(pw_payload), nonce, key_agreed)) {
		errsv = errno;
		log_warn("auth_client_remote_session_token_process(): crypt_encrypt_xsalsa20poly1305(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Set nonce value to the head of session */
	memcpy(session, nonce, CRYPT_NONCE_SIZE_XSALSA20);

	/* Set encryption/decryption token */
	memcpy(token, key_agreed, HASH_DIGEST_SIZE_BLAKE2S);

	/* Session data contents:
	 *
	 * | nonce (24 bytes) | encrypted pw payload (16 bytes + 1 byte + 256 bytes) |
	 *
	 * Total session size: 297 bytes
	 *
	 */

	/* Cleanup data */
	memset(key, 0, sizeof(key));
	memset(key_agreed, 0, sizeof(key_agreed));
	memset(pwhash_c, 0, sizeof(pwhash_c));
	memset(pwhash_s, 0, sizeof(pwhash_c));
	memset(client_hash, 0, sizeof(client_hash));
	memset(client_hash_tmp, 0, sizeof(client_hash));
	memset(server_token, 0, sizeof(client_hash));
	memset(server_recvd_session, 0, sizeof(server_recvd_session));
	memset(pw_payload, 0, sizeof(pw_payload));

	/* All good */
	return 0;
}

