/**
 * @file entry.c
 * @brief uSched
 *        Entry handling interface - Client
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
#include <time.h>

#include <sys/types.h>

#include <psec/crypt.h>

#include "debug.h"
#include "config.h"
#include "mm.h"
#include "entry.h"
#include "log.h"
#include "auth.h"

struct usched_entry *entry_client_init(uid_t uid, gid_t gid, time_t trigger, void *payload, size_t psize) {
	int errsv = 0;
	struct usched_entry *entry = NULL;

	if (!(entry = mm_alloc(sizeof(struct usched_entry)))) {
		errsv = errno;
		log_warn("entry_client_init(): mm_alloc(): %s\n", strerror(errno));
		errno = errsv;
		return NULL;
	}

	memset(entry, 0, sizeof(struct usched_entry));

	entry_set_uid(entry, uid);
	entry_set_gid(entry, gid);
	entry_set_trigger(entry, trigger);

	if (entry_set_payload(entry, payload, psize) < 0) {
		errsv = errno;
		log_warn("entry_client_init(): entry_set_payload(): %s\n", strerror(errno));
		mm_free(entry);
		errno = errsv;
		return NULL;
	}

	return entry;
}

int entry_client_remote_session_create(struct usched_entry *entry) {
	int errsv = 0;

	if (auth_client_remote_user_token_create(entry->session) < 0) {
		errsv = errno;
		log_warn("entry_client_remote_session_create(): auth_client_remote_user_token_create(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	return 0;
}

int entry_client_remote_session_process(struct usched_entry *entry, const char *password) {
	int errsv = 0;

	/* Process remote session data */
	if (auth_client_remote_user_token_process(entry->session, password, entry->nonce, entry->token) < 0) {
		errsv = errno;
		log_warn("entry_client_remote_session_process(): auth_client_remote_user_token_process(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* All good */
	return 0;
}

int entry_client_payload_encrypt(struct usched_entry *entry) {
	int errsv = 0;
	unsigned char *payload_enc = NULL;
	size_t out_len = 0;

	/* Alloc memory for encrypted payload */
	if (!(payload_enc = mm_alloc(entry->psize + CRYPT_EXTRA_SIZE_XSALSA20))) {
		errsv = errno;
		log_warn("entry_client_payload_encrypt(): mm_alloc(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Encrypt payload */
	if (!(crypt_encrypt_xsalsa20(payload_enc, &out_len, (unsigned char *) entry->payload, entry->psize, entry->nonce, entry->token))) {
		errsv = errno;
		log_warn("entry_client_payload_encrypt(): crypt_encrypt_xsalsa20(): %s\n", strerror(errno));
		mm_free(payload_enc);
		errno = errsv;
		return -1;
	}

	/* Set the psize */
	entry->psize = out_len;

	/* Free plaintext payload */
	mm_free(entry->payload);

	/* Set new payload */
	entry->payload = (char *) payload_enc;

	/* All good */
	return 0;
}

