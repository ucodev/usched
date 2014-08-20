/**
 * @file auth.c
 * @brief uSched
 *        Authentication and Authorization interface - Daemon
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
#include <unistd.h>

#include <psec/crypt.h>
#include <psec/decode.h>
#include <psec/generate.h>
#include <psec/hash.h>
#include <psec/kdf.h>

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
	unsigned char *dh_shared,
	size_t dh_shared_size,
	unsigned char *nonce,
	unsigned char *token,
	uid_t *uid,
	gid_t *gid)
{
	int errsv = 0, rounds = CONFIG_USCHED_SEC_KDF_ROUNDS;
	struct usched_config_userinfo *userinfo = NULL;
	unsigned char salt[HASH_DIGEST_SIZE_BLAKE2S];
	unsigned char salt_raw[CONFIG_USCHED_AUTH_USERNAME_MAX];
	unsigned char pwhash_c[HASH_DIGEST_SIZE_SHA512];
	unsigned char pwhash_s[HASH_DIGEST_SIZE_SHA512 + 3];
	unsigned char pwrehash[HASH_DIGEST_SIZE_BLAKE2S];
	unsigned char pw_payload[CONFIG_USCHED_AUTH_PASSWORD_MAX + 1];
	unsigned char key[HASH_DIGEST_SIZE_BLAKE2S];
	unsigned char key_agreed[HASH_DIGEST_SIZE_BLAKE2S];
	char plain_passwd[CONFIG_USCHED_AUTH_PASSWORD_MAX + 1];
	size_t out_len = 0;

	/* Session data contents:
	 *
	 * | nonce (24 bytes) | encrypted plain password (16 bytes + 1 byte + 256 bytes) |
	 *
	 * Total session size: 297 bytes
	 *
	 */

	/* Check if username doesn't exceed the expected size */
	if (strlen(username) > sizeof(salt)) {
		log_warn("auth_daemon_remote_user_token_verify(): strlen(username) > sizeof(salt)\n");
		errno = EINVAL;
		return -1;
	}

	/* Craft raw salt */
	memset(salt_raw, 'x', sizeof(salt_raw));
	memcpy(salt_raw, username, strlen(username));

	/* Hash raw salt */
	if (!hash_buffer_blake2s(salt, salt_raw, sizeof(salt_raw))) {
		errsv = errno;
		log_warn("auth_daemon_remote_user_token_verify(): hash_buffer_blake2s(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Get userinfo data from current configuration */
	if (!(userinfo = rund.config.users.list->search(rund.config.users.list, (struct usched_config_userinfo [1]) { { (char *) username, NULL, NULL, 0, 0} }))) {
		errsv = errno;
		log_warn("auth_daemon_remote_user_token_verify(): No such username: %s\n", username);
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

	/* Extract nonce from the received session field */
	memcpy(nonce, session, CRYPT_NONCE_SIZE_XSALSA20);

	/* Shrink the shared key with a blake2s digest in order to match the encryption key size */
	if (!hash_buffer_blake2s(key, dh_shared, dh_shared_size)) {
		errsv = errno;
		log_warn("auth_daemon_remote_user_token_verify(): hash_buffer_blake2s(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Shrink the pwhash with a blake2s digest in order to match the encryption key size */
	if (!hash_buffer_blake2s(pwrehash, pwhash_s, sizeof(pwhash_s) - 3)) {
		errsv = errno;
		log_warn("auth_daemon_remote_user_token_verify(): hash_buffer_blake2s(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Encrypt the re-hashed DH shared key with the re-hashed pwhash to obain the agreed key */
	if (!crypt_encrypt_otp(key_agreed, &out_len, pwrehash, sizeof(pwrehash), NULL, key)) {
		errsv = errno;
		log_warn("auth_daemon_remote_user_token_verify(): crypt_encrypt_otp(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Decrypt client password with DH shared as key */
	if (!crypt_decrypt_xsalsa20poly1305(pw_payload, &out_len, (unsigned char *) (session + CRYPT_NONCE_SIZE_XSALSA20), sizeof(pw_payload) + CRYPT_EXTRA_SIZE_XSALSA20POLY1305, nonce, key_agreed)) {
		errsv = errno;
		log_warn("auth_daemon_remote_user_token_verify(): crypt_decrypt_xsalsa20poly1305(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Extract the plain password */
	memcpy(plain_passwd, pw_payload + 1, pw_payload[0]);
	plain_passwd[pw_payload[0]] = 0;

	/* Generate the client password hash */
	if (!kdf_pbkdf2_hash(pwhash_c, hash_buffer_sha512, HASH_DIGEST_SIZE_SHA512, HASH_BLOCK_SIZE_SHA512, (unsigned char *) plain_passwd, strlen(plain_passwd), salt, sizeof(salt), rounds, HASH_DIGEST_SIZE_SHA512) < 0) {
		errsv = errno;
		log_warn("auth_daemon_remote_user_token_verify(): kdf_pbkdf2_hash(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Check if password hashes match */
	if (memcmp(pwhash_s, pwhash_c, HASH_DIGEST_SIZE_SHA512)) {
		log_warn("auth_daemon_remote_user_token_verify(): Server and Client password hashes do not match.\n");
		errno = EINVAL;
		return -1;
	}

	/* Set encryption/decryption token */
	memcpy(token, key_agreed, HASH_DIGEST_SIZE_BLAKE2S);

	/* Set uid and gid */
	*uid = userinfo->uid;
	*gid = userinfo->gid;

	/* Cleanup data */
	memset(key, 0, sizeof(key));
	memset(key_agreed, 0, sizeof(key));
	memset(pwhash_c, 0, sizeof(pwhash_c));
	memset(pwhash_s, 0, sizeof(pwhash_s));
	memset(pwrehash, 0, sizeof(pwrehash));
	memset(pw_payload, 0, sizeof(pw_payload));
	memset(plain_passwd, 0, sizeof(plain_passwd));

	/* All good */
	return 0;
}

int auth_daemon_remote_user_token_create(
	const char *username,
	char *session,
	unsigned char *dh_shared,
	size_t dh_shared_size,
	unsigned char *nonce,
	unsigned char *token)
{
	int errsv = 0, rounds = CONFIG_USCHED_SEC_KDF_ROUNDS;
	struct usched_config_userinfo *userinfo = NULL;
	char *session_pos = session + sizeof(rund.sec.key_pub);
	unsigned char pwhash[HASH_DIGEST_SIZE_SHA512 + 3];
	unsigned char key[HASH_DIGEST_SIZE_BLAKE2S];
	unsigned char client_hash[HASH_DIGEST_SIZE_BLAKE2S];
	unsigned char client_hash_tmp[HASH_DIGEST_SIZE_SHA512 * 2];
	unsigned char client_token[HASH_DIGEST_SIZE_BLAKE2S];
	unsigned char server_token[HASH_DIGEST_SIZE_BLAKE2S];
	size_t out_len = 0;

	/* Session data contents
	 *
	 * | pubkey (512 bytes) | encrypted token (32 bytes) |
	 *
	 * Total session size: 544 bytes
	 *
	 */

	/* Get userinfo data from current configuration */
	if (!(userinfo = rund.config.users.list->search(rund.config.users.list, (struct usched_config_userinfo [1]) { { (char *) username, NULL, NULL, 0, 0 } }))) {
		log_warn("auth_daemon_remote_user_token_create(): No such username: %s\n", username);
		errno = EINVAL;
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

	/* Re-hash the pwhash to achieve the decryption key for client token */
	if (!hash_buffer_blake2s(key, pwhash, sizeof(pwhash) - 3)) {
		errsv = errno;
		log_warn("auth_daemon_remote_user_token_create(): hash_buffer_blake2s(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Decrypt client token */
	if (!crypt_decrypt_otp(client_token, &out_len, (unsigned char *) session_pos, HASH_DIGEST_SIZE_BLAKE2S, NULL, key)) {
		errsv = errno;
		log_warn("auth_daemon_remote_user_token_create(): crypt_decrypt_otp(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Compute a client hash */
	if (!kdf_pbkdf2_hash(client_hash_tmp, hash_buffer_sha512, HASH_DIGEST_SIZE_SHA512, HASH_BLOCK_SIZE_SHA512, pwhash, sizeof(pwhash) - 3, client_token, sizeof(client_token), rounds, HASH_DIGEST_SIZE_SHA512 * 2) < 0) {
		errsv = errno;
		log_warn("auth_daemon_remote_user_token_create(): kdf_pbkdf2_hash(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Shrink the temporary client hash */
	if (!hash_buffer_blake2s(client_hash, client_hash_tmp, HASH_DIGEST_SIZE_SHA512 * 2)) {
		errsv = errno;
		log_warn("auth_daemon_remote_user_token_create(): hash_buffer_blake2s(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Encrypt pwhash with re-hashed version of client token */
	if (!crypt_encrypt_otp(server_token, &out_len, key, HASH_DIGEST_SIZE_BLAKE2S, NULL, client_hash)) {
		errsv = errno;
		log_warn("auth_daemon_remote_user_token_create(): crypt_encrypt_otp(): %s\n", strerror(errno));
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

	/* Shrink the shared key with a blake2s digest in order to match the encryption key size */
	if (!hash_buffer_blake2s(key, dh_shared, dh_shared_size)) {
		errsv = errno;
		log_warn("auth_daemon_remote_user_token_create(): hash_buffer_blake2s(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Encrypt the session token with the resulting blake2s digest as key */
	if (!crypt_encrypt_xsalsa20poly1305((unsigned char *) (session_pos + CRYPT_NONCE_SIZE_XSALSA20), &out_len, server_token, HASH_DIGEST_SIZE_BLAKE2S, nonce, key)) {
		errsv = errno;
		log_warn("auth_daemon_remote_user_token_create(): crypt_encrypt_xsalsa20poly1305(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Craft session field */
	memcpy(session_pos, nonce, CRYPT_NONCE_SIZE_XSALSA20);

	/* Session contents:
	 *
	 * | pubkey (512 bytes) | nonce (24 bytes) | encrypted token (16 + 32 bytes) |
	 *
	 * Total size of session field: 584 bytes
	 */

	/* Cleanup data */
	memset(pwhash, 0, sizeof(pwhash));
	memset(key, 0, sizeof(key));
	memset(client_hash, 0, sizeof(client_hash));
	memset(client_hash_tmp, 0, sizeof(client_hash_tmp));
	memset(client_token, 0, sizeof(client_token));
	memset(server_token, 0, sizeof(server_token));

	/* All good */
	return 0;
}

