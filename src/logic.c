/**
 * @file logic.c
 * @brief uSched
 *        Logic Analyzer interface
 *
 * Date: 16-07-2014
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
#include <stdlib.h>
#include <time.h>

#include "mm.h"
#include "debug.h"
#include "runtime.h"
#include "entry.h"
#include "logic.h"
#include "conn.h"


int logic_process_run(void) {
	struct usched_request *cur = NULL;
	struct usched_entry *entry = NULL;
	time_t time_ref = runc.t;

	for (cur = runc.req; cur; cur = cur->next, runc.epool->push(runc.epool, entry)) {
		/* Allocate a new scheduling entry with subject as its payload. */
		if (!(entry = entry_client_init(cur->uid, cur->gid, time_ref + cur->arg, cur->subj, strlen(cur->subj) + 1)))
			return -1;

		/* This is a new entry */
		entry_set_flag(entry, USCHED_ENTRY_FLAG_NEW);

		/* Check if this is a THEN conjunction */
		if (cur->conj == USCHED_CONJ_THEN) {
			if (!cur->next)
				return -1;

			cur = cur->next;

			/* Expect the EVERY preposition after a THEN conjunction */
			if (cur->prep != USCHED_PREP_EVERY)
				return -1;

			entry_set_step(entry, cur->arg);
		}

		/* Check if this is an UNTIL conjunction */
		if (cur->conj == USCHED_CONJ_UNTIL) {
			if (!cur->next)
				return -1;

			cur = cur->next;

			/* Expect the TO preposition after an UNTIL conjunction */
			if (cur->prep != USCHED_PREP_TO)
				return -1;

			entry_set_expire(entry, time_ref + cur->arg);
		} else if (cur->conj == USCHED_CONJ_WHILE) {
			/* If this is a WHILE conjunction */
			if (!cur->next)
				return -1;

			cur = cur->next;

			/* Expect the IN preposition after a WHILE conjunction */
			if (cur->prep != USCHED_PREP_IN)
				return -1;

			entry_set_expire(entry, time_ref + cur->arg);
		}

		/* If there's a conjuntion, it's expected to be AND */
		if (cur->conj && (cur->conj != USCHED_CONJ_AND))
			return -1;
	}

	/* Logic accepted */
	return 0;
}

int logic_process_stop(void) {
	char *ptr = NULL, *saveptr = NULL, *endptr = NULL;
	struct usched_request *cur = NULL;
	struct usched_entry *entry = NULL;
	uint64_t *entry_list = NULL;
	uint64_t entry_id = 0;
	size_t entry_list_nmemb = 0;

	/* Validate the if the current request list has at least one valid entry */
	if (!(cur = runc.req))
		return -1;

	/* Validate if this request refers to all entries beloging to this user */
	if (!strcasecmp(cur->subj, "all")) {
		entry_list_nmemb ++;

		if (!(entry_list = mm_alloc(sizeof(uint64_t))))
			return -1;

		entry_list[0] = 0;	/* 0 means all entries belonging to this user */
	} else {
		/* Iterate the current request list in order to craft an entry payload */
		for (ptr = cur->subj, entry_list_nmemb = 0; (ptr = strtok_r(ptr, ",", &saveptr)); ptr = NULL) {
			entry_list_nmemb ++;

			/* Realloc the entry_list size */
			if (!(entry_list = mm_realloc(entry_list, entry_list_nmemb * sizeof(uint64_t))))
				return -1;

			/* If the requested entry id to be deleted is 0 or invalid, fail to accept logic */
			if (!(entry_id = strtoull(ptr, &endptr, 16)) || (*endptr) || (endptr == ptr)) {
				mm_free(entry_list);
				return -1;
			}

			debug_printf(DEBUG_INFO, "OP == STOP: entry_id == 0x%llX\n", entry_id);

			/* Append the extracted entry id to the current entry list */
			entry_list[entry_list_nmemb - 1] = htonll(entry_id);
		}
	}

	/* No conjunctions are accepted in a STOP operation */
	if (cur->conj) {
		mm_free(entry_list);
		return -1;
	}

	/* Initialize the entry to be transmitted */
	if (!(entry = entry_client_init(cur->uid, cur->gid, 0, entry_list, entry_list_nmemb * sizeof(uint64_t)))) {
		mm_free(entry_list);
		return -1;
	}

	/* Set this entry to be deleted */
	entry_set_flag(entry, USCHED_ENTRY_FLAG_DEL);

	/* Push the entry into the entries pool */
	if (runc.epool->push(runc.epool, entry) < 0) {
		mm_free(entry_list);
		entry_destroy(entry);
		return -1;
	}

	/* Logic accepted */
	return 0;
}

int logic_process_show(void) {
	char *ptr = NULL, *saveptr = NULL, *endptr = NULL;
	struct usched_request *cur = NULL;
	struct usched_entry *entry = NULL;
	uint64_t *entry_list = NULL;
	uint64_t entry_id = 0;
	size_t entry_list_nmemb = 0;

	/* Validate the if the current request list has at least one valid entry */
	if (!(cur = runc.req))
		return -1;

	/* Validate if this request refers to all entries beloging to this user */
	if (!strcasecmp(cur->subj, "all")) {
		entry_list_nmemb ++;

		if (!(entry_list = mm_alloc(sizeof(uint64_t))))
			return -1;

		entry_list[0] = 0;	/* 0 means all entries belonging to this user */
	} else {
		/* Iterate the current request list in order to craft an entry payload */
		for (ptr = cur->subj, entry_list_nmemb = 0; (ptr = strtok_r(ptr, ",", &saveptr)); ptr = NULL) {
			entry_list_nmemb ++;

			/* Realloc the entry_list size */
			if (!(entry_list = mm_realloc(entry_list, entry_list_nmemb * sizeof(uint64_t))))
				return -1;

			/* If the requested entry id to be deleted is 0 or invalid, fail to accept logic */
			if (!(entry_id = strtoull(ptr, &endptr, 16)) || (*endptr) || (endptr == ptr)) {
				mm_free(entry_list);
				return -1;
			}

			debug_printf(DEBUG_INFO, "OP == SHOW: entry_id == 0x%llX\n", entry_id);

			/* Append the extracted entry id to the current entry list */
			entry_list[(entry_list_nmemb * sizeof(uint64_t)) - 1] = htonll(entry_id);

			/* No conjunctions are accepted in a STOP operation */
			if (cur->conj) {
				mm_free(entry_list);
				return -1;
			}
		}
	}

	/* Initialize the entry to be transmitted */
	if (!(entry = entry_client_init(cur->uid, cur->gid, 0, entry_list, entry_list_nmemb * sizeof(uint64_t)))) {
		mm_free(entry_list);
		return -1;
	}

	/* Set this entry to be deleted */
	entry_set_flag(entry, USCHED_ENTRY_FLAG_GET);

	/* Push the entry into the entries pool */
	if (runc.epool->push(runc.epool, entry) < 0) {
		mm_free(entry_list);
		entry_destroy(entry);
		return -1;
	}

	/* Logic accepted */
	return 0;

}


