/**
 * @file entry.c
 * @brief uSched
 *        Entry handling interface - Common
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
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#include <sys/types.h>

#include "config.h"
#include "mm.h"
#include "entry.h"
#include "bitops.h"
#include "log.h"

void entry_set_id(struct usched_entry *entry, uint32_t id) {
	entry->id = id;
}

void entry_set_flags(struct usched_entry *entry, uint32_t flags) {
	entry->flags = flags;
}

void entry_unset_flags_local(struct usched_entry *entry) {
	/* Clear all local flags */
	entry_unset_flag(entry, USCHED_ENTRY_FLAG_INIT);
	entry_unset_flag(entry, USCHED_ENTRY_FLAG_AUTHORIZED);
	entry_unset_flag(entry, USCHED_ENTRY_FLAG_FINISH);
}

int entry_has_flag(struct usched_entry *entry, usched_entry_flag_t flag) {
	return bit_test(&entry->flags, flag);
}

void entry_set_flag(struct usched_entry *entry, usched_entry_flag_t flag) {
	bit_set(&entry->flags, flag);
}

void entry_unset_flag(struct usched_entry *entry, usched_entry_flag_t flag) {
	bit_clear(&entry->flags, flag);
}

void entry_set_uid(struct usched_entry *entry, uid_t uid) {
	entry->uid = uid;
}

void entry_set_gid(struct usched_entry *entry, gid_t gid) {
	entry->gid = gid;
}

void entry_set_trigger(struct usched_entry *entry, time_t trigger) {
	entry->trigger = (uint32_t) trigger;
}

void entry_set_step(struct usched_entry *entry, time_t step) {
	entry->step = (uint32_t) step;
}

void entry_set_expire(struct usched_entry *entry, time_t expire) {
	entry->expire = (uint32_t) expire;
}

void entry_set_psize(struct usched_entry *entry, size_t size) {
	entry->psize = (uint32_t) size;
}

void entry_set_subj_size(struct usched_entry *entry, size_t size) {
	entry->subj_size = (uint32_t) size;
}

int entry_set_payload(struct usched_entry *entry, const char *payload, size_t len) {
	int errsv = 0;

	if (!(entry->payload = mm_alloc(len))) {
		errsv = errno;
		log_warn("entry_set_payload(): mm_alloc(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	memset(entry->payload, 0, len);

	memcpy(entry->payload, payload, len);

	entry_set_psize(entry, len);

	return 0;
}

void entry_unset_payload(struct usched_entry *entry) {
	if (entry->payload) {
		mm_free(entry->payload);
		entry->payload = NULL;
	}

	entry->psize = 0;
}

int entry_set_subj(struct usched_entry *entry, const char *subj, size_t len) {
	int errsv = 0;

	if (!len)
		len = strlen(subj) + 1;

	if (!(entry->subj = mm_alloc(len + 1))) {
		errsv = errno;
		log_warn("entry_set_subj(): mm_alloc(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	memset(entry->subj, 0, len + 1);

	memcpy(entry->subj, subj, len);

	entry_set_subj_size(entry, len);

	return 0;
}

int entry_copy(struct usched_entry *dest, struct usched_entry *src) {
	int errsv = 0;

	memcpy(dest, src, sizeof(struct usched_entry));

	if (src->subj && src->subj_size) {
		if (entry_set_subj(dest, src->subj, src->subj_size) < 0) {
			errsv = errno;
			log_warn("entry_copy(): entry_set_subj(): %s\n", strerror(errno));
			errno = errsv;
			return -1;
		}
	}

	if (src->payload && src->psize) {
		if (entry_set_payload(dest, src->payload, src->psize) < 0) {
			errsv = errno;
			log_warn("entry_copy(): entry_set_payload(): %s\n", strerror(errno));
			mm_free(dest->subj);
			errno = errsv;
			return -1;
		}
	}

	return 0;
}

int entry_compare(const void *e1, const void *e2) {
	const struct usched_entry *pe1 = (struct usched_entry *) e1, *pe2 = (struct usched_entry *) e2;

	if (pe1->id > pe2->id)
		return 1;

	if (pe1->id < pe2->id)
		return -1;

	return 0;
}

void entry_destroy(void *elem) {
	struct usched_entry *entry = elem;

	if (entry->payload)
		mm_free(entry->payload);

	if (entry->subj)
		mm_free(entry->subj);

	mm_free(entry);
}

