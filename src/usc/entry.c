/**
 * @file entry.c
 * @brief uSched
 *        Entry handling interface - Client
 *
 * Date: 22-08-2014
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
#include <psec/generate.h>

#include "debug.h"
#include "runtime.h"
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

int entry_client_remote_session_create(struct usched_entry *entry, const char *password) {
	int errsv = 0;

	/* Insert client session token into session data */
	if (auth_client_remote_session_create(entry->session, entry->username, password, entry->context) < 0) {
		errsv = errno;
		log_warn("entry_client_remote_session_create(): auth_client_remote_session_create(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	return 0;
}

int entry_client_remote_session_process(struct usched_entry *entry, const char *password) {
	int errsv = 0;

	/* Process remote session data */
	if (auth_client_remote_session_process(entry->session, entry->username, password, entry->context, entry->agreed_key) < 0) {
		errsv = errno;
		log_warn("entry_client_remote_session_process(): auth_client_remote_session_process(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Set nonce to 0 */
	entry->nonce = 0;

	/* All good */
	return 0;
}

