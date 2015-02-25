/**
 * @file auth.c
 * @brief uSched
 *        Authentication and Authorization interface - Daemon
 *
 * Date: 25-02-2015
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
	uid_t local_uid = 0;
	gid_t local_gid = 0;

	if (local_fd_peer_cred(fd, &local_uid, &local_gid) < 0) {
		errsv = errno;
		log_warn("auth_daemon_local(): local_fd_peer_cred(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Grant that the UID or GID received from local client matches the real values for
	 * the user. We should log protocol manipulation attempts.
	 */
	if ((local_uid != *uid) || (local_gid != *gid)) {
		log_crit("auth_daemon_local(): The real UID and/or GID for the user requesting the entry doesn't match the values sent by the user's client.\n");
		errno = EINVAL;
		return -1;
	}

	return 0;
}

int auth_daemon_remote_session_verify(
	const char *username,
	const unsigned char *session,
	unsigned char *context,
	unsigned char *agreed_key,
	uid_t *uid,
	gid_t *gid)
{
	int errsv = 0;
	struct usched_config_userinfo *userinfo = NULL;
	unsigned char salt[HASH_DIGEST_SIZE_BLAKE2S];
	unsigned char salt_raw[CONFIG_USCHED_AUTH_USERNAME_MAX];
	unsigned char pwhash_s[HASH_DIGEST_SIZE_SHA512 + 3];
	size_t out_len = 0;

	/* Session data contents:
	 *
	 * | encrypted plain password (1 byte + 256 bytes) |
	 *
	 * Total session size: 257 bytes
	 *
	 */

	/* The UID and GID value passed to this function must be set to 0xff before the assignment.
	 * Any other value means that the request was crafted by a client that does not meet the
	 * specifications.
	 */
	if ((*uid != 0xff) || (*gid != 0xff)) {
		log_warn("auth_daemon_remote_session_verify(): The received user or group identification doesn't match the specification for remote requests. (Remote UID: %u, Remote GID: %u)\n", *uid, *gid);
		errno = EINVAL;
		return -1;
	}

	/* Check if username doesn't exceed the expected size */
	if (strlen(username) > sizeof(salt_raw)) {
		log_warn("auth_daemon_remote_session_verify(): strlen(username) > sizeof(salt_raw)\n");
		errno = EINVAL;
		return -1;
	}

	/* Craft raw salt */
	memset(salt_raw, 'x', sizeof(salt_raw));
	memcpy(salt_raw, username, strlen(username));

	/* Hash raw salt */
	if (!hash_buffer_blake2s(salt, salt_raw, sizeof(salt_raw))) {
		errsv = errno;
		log_warn("auth_daemon_remote_session_verify(): hash_buffer_blake2s(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Get userinfo data from current configuration */
	if (!(userinfo = rund.config.users.list->search(rund.config.users.list, (struct usched_config_userinfo [1]) { { (char *) username, NULL, NULL, 0, 0} }))) {
		errsv = errno;
		log_warn("auth_daemon_remote_session_verify(): No such username: %s\n", username);
		errno = errsv;
		return -1;
	}

	/* Grant that userinfo->password doesn't exceed the expected length */
	if (decode_size_base64(strlen(userinfo->password)) > sizeof(pwhash_s)) {
		log_warn("auth_daemon_remote_session_verify(): pwhash_s buffer is too small to receive the decoded user password.\n");
		errno = EINVAL;
		return -1;
	}

	/* Decode the base64 encoded password hash from current configuration */
	if (!decode_buffer_base64(pwhash_s, &out_len, (unsigned char *) userinfo->password, strlen(userinfo->password))) {
		errsv = errno;
		log_warn("auth_daemon_remote_session_verify(): decode_buffer_base64(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Authorize */
	if (ke_chreke_server_authorize(context, agreed_key, session, salt, sizeof(salt)) < 0) {
		errsv = errno;
		log_warn("auth_daemon_remote_session_verify(): ke_chreke_server_authorize(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Set effective UID and GID */
	*uid = userinfo->uid;
	*gid = userinfo->gid;
	
	/* All good */
	return 0;
}

int auth_daemon_remote_session_create(
	const char *username,
	unsigned char *session,
	unsigned char *context)
{
	int errsv = 0;
	struct usched_config_userinfo *userinfo = NULL;
	unsigned char pwhash[HASH_DIGEST_SIZE_SHA512 + 3];
	unsigned char server_session[KE_SERVER_SESSION_SIZE_CHREKE];
	size_t out_len = 0;

	/* Session data contents
	 *
	 * | pubkey (32 bytes) | encrypted token (32 bytes) |
	 *
	 * Total session size: 64 bytes
	 *
	 */

	/* Check if username starts with a '.' (as in, not commited, but an attempt to exploit it
	 * is being performed)
	 */
	if (username[0] == '.') {
		log_warn("auth_daemon_remote_session_create(): Invalid username: %s (possible exploitation attempt)\n", username);
		errno = EINVAL;
		return -1;
	}

	/* Get userinfo data from current configuration */
	if (!(userinfo = rund.config.users.list->search(rund.config.users.list, (struct usched_config_userinfo [1]) { { (char *) username, NULL, NULL, 0, 0 } }))) {
		log_warn("auth_daemon_remote_session_create(): No such username: %s\n", username);
		errno = EINVAL;
		return -1;
	}

	/* Grant that userinfo->password doesn't exceed the expected length */
	if (decode_size_base64(strlen(userinfo->password)) > sizeof(pwhash)) {
		log_warn("auth_daemon_remote_session_verify(): pwhash buffer is too small to receive the decoded user password.\n");
		errno = EINVAL;
		return -1;
	}

	/* Decode user password hash from base64 */
	if (!decode_buffer_base64(pwhash, &out_len, (unsigned char *) userinfo->password, strlen(userinfo->password))) {
		errsv = errno;
		log_warn("auth_daemon_remote_session_create(): decode_buffer_base64(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Initialize chreke server authentication */
	if (!ke_chreke_server_init(server_session, context, session, pwhash)) {
		errsv = errno;
		log_warn("auth_daemon_remote_session_create(): ke_chreke_server_init(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Set session data */
	memcpy(session, server_session, sizeof(server_session));

	/* Session contents:
	 *
	 * | pubkey (32 bytes) | encrypted server token (32 bytes) |
	 *
	 * Total size of session field: 64 bytes
	 */

	/* All good */
	return 0;
}

