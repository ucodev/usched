/**
 * @file auth.c
 * @brief uSched
 *        Authentication and Authorization interface - Client
 *
 * Date: 12-08-2014
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

#include "log.h"
#include "auth.h"

int auth_client_remote_user_token_process(
	char *session_passwd,
	const char *plain_passwd,
	unsigned char *nonce,
	unsigned char *token)
{
	int errsv = 0, rounds = 5000;
	unsigned char salt[8];
	unsigned char pwhash[HASH_DIGEST_SIZE_SHA512];
	unsigned char key[CRYPT_KEY_SIZE_XSALSA20];
	size_t out_len = 0;

	/* Extract salt and nonce from session */
	memcpy(salt, session_passwd, sizeof(salt));
	memcpy(nonce, session_passwd + sizeof(salt), CRYPT_NONCE_SIZE_XSALSA20);

	/* Create a password hash with the same parameters as remote party */
	if (!kdf_pbkdf2_hash(pwhash, hash_buffer_sha512, HASH_DIGEST_SIZE_SHA512, HASH_BLOCK_SIZE_SHA512, (unsigned char *) plain_passwd, strlen(plain_passwd), salt, sizeof(salt), rounds, HASH_DIGEST_SIZE_SHA512) < 0) {
		errsv = errno;
		log_warn("auth_client_remote_user_token_process(): kdf_pbkdf2_hash(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Re-hash the result to match the decryption key size (32 bytes) */
	if (!hash_buffer_blake2s(key, pwhash, sizeof(pwhash))) {
		errsv = errno;
		log_warn("auth_client_remote_user_token_process(): hash_buffer_blake2s(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Decrypt the token value */
	if (!crypt_decrypt_xsalsa20(token, &out_len, (unsigned char *) (session_passwd + sizeof(salt) + CRYPT_NONCE_SIZE_XSALSA20), CRYPT_KEY_SIZE_XSALSA20 + CRYPT_EXTRA_SIZE_XSALSA20, nonce, key)) {
		errsv = errno;
		log_warn("auth_client_remote_user_token_process(): crypt_decrypt_xsalsa20(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Generate a new nonce value */
	if (!generate_bytes_random(nonce, CRYPT_NONCE_SIZE_XSALSA20)) {
		errsv = errno;
		log_warn("auth_client_remote_user_token_process(): generate_bytes_random(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Encrypt the user password hash with the session token as key */
	if (!crypt_encrypt_xsalsa20((unsigned char *) (session_passwd + CRYPT_NONCE_SIZE_XSALSA20), &out_len, pwhash, sizeof(pwhash), nonce, token)) {
		errsv = errno;
		log_warn("auth_client_remote_user_token_process(): crypt_encrypt_xsalsa20(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Set nonce value to the head of session */
	memcpy(session_passwd, nonce, CRYPT_NONCE_SIZE_XSALSA20);

	return 0;
}

