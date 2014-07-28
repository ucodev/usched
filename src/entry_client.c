/**
 * @file entry_client.c
 * @brief uSched
 *        Entry handling interface - Client
 *
 * Date: 28-07-2014
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

#include "config.h"
#include "mm.h"
#include "entry.h"
#include "log.h"

struct usched_entry *entry_client_init(uid_t uid, gid_t gid, time_t trigger, void *payload, size_t psize) {
	int errsv = 0;
	struct usched_entry *entry = NULL;

	if (!(entry = mm_alloc(sizeof(struct usched_entry)))) {
		errsv = errno;
		log_warn("entry_init(): mm_alloc(): %s\n", strerror(errno));
		errno = errsv;
		return NULL;
	}

	memset(entry, 0, sizeof(struct usched_entry));

	entry_set_uid(entry, uid);
	entry_set_gid(entry, gid);
	entry_set_trigger(entry, trigger);

	if (entry_set_payload(entry, payload, psize) < 0) {
		errsv = errno;
		log_warn("entry_init(): entry_set_payload(): %s\n", strerror(errno));
		mm_free(entry);
		errno = errsv;
		return NULL;
	}

	return entry;
}

