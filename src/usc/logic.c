/**
 * @file logic.c
 * @brief uSched
 *        Logic Analyzer interface - Client
 *
 * Date: 13-07-2015
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
#include <strings.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>

#include "config.h"
#include "mm.h"
#include "debug.h"
#include "runtime.h"
#include "entry.h"
#include "logic.h"
#include "conn.h"
#include "bitops.h"
#include "usched.h"


int logic_client_process_hold(void) {
	int errsv = 0;
	char *ptr = NULL, *saveptr = NULL, *endptr = NULL;
	struct usched_client_request *cur = NULL;
	struct usched_entry *entry = NULL;
	uint64_t *entry_list = NULL;
	uint64_t entry_id = 0;
	size_t entry_list_nmemb = 0;

	/* Validate the if the current request list has at least one valid entry */
	if (!(cur = runc.req)) {
		errno = EINVAL;
		return -1;
	}

	/* Validate if this request refers to all entries beloging to this user */
	if (!strcasecmp(cur->subj, USCHED_SUBJ_ALL_STR)) {
		entry_list_nmemb = 1;

		if (!(entry_list = mm_alloc(sizeof(uint64_t))))
			return -1;

		entry_list[0] = USCHED_SUBJ_ALL; /* means all entries belonging to this user */
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
				errno = EINVAL;
				return -1;
			}

			debug_printf(DEBUG_INFO, "OP == HOLD: entry_id == 0x%llX\n", entry_id);

			/* Append the extracted entry id to the current entry list */
			entry_list[entry_list_nmemb - 1] = htonll(entry_id);
		}
	}

	/* No conjunctions are accepted in a STOP operation */
	if (cur->conj) {
		mm_free(entry_list);
		errno = EINVAL;
		return -1;
	}

	/* Initialize the entry to be transmitted */
	if (!(entry = entry_client_init(cur->uid, cur->gid, 0, entry_list, entry_list_nmemb * sizeof(uint64_t)))) {
		errsv = errno;
		mm_free(entry_list);
		errno = errsv;
		return -1;
	}

	/* Free entry_list */
	mm_free(entry_list);

	/* Set this entry to be deleted */
	entry_set_flag(entry, USCHED_ENTRY_FLAG_PAUSE);

	/* Push the entry into the entries pool */
	if (runc.epool->push(runc.epool, entry) < 0) {
		errsv = errno;
		mm_free(entry_list);
		entry_destroy(entry);
		errno = errsv;
		return -1;
	}

	/* Logic accepted */
	return 0;
}

int logic_client_process_run(void) {
	struct usched_client_request *cur = NULL;
	struct usched_entry *entry = NULL;
	time_t time_ref = runc.t;

	/* Perliminary checks for the first entry */
	if (runc.req->prep == USCHED_PREP_EVERY) {
		/* First preposition cannot be EVERY */
		errno = EINVAL;
		return -1;
	}

	/* Check further logic */
	for (cur = runc.req; cur; cur = cur->next, runc.epool->push(runc.epool, entry)) {
		/* Allocate a new scheduling entry with subject as its payload. */
		if (!(entry = entry_client_init(cur->uid, cur->gid, time_ref + cur->arg, cur->subj, strlen(cur->subj) + 1)))
			return -1;

		/* This is a new entry */
		entry_set_flag(entry, USCHED_ENTRY_FLAG_NEW);

		/* Check if the initial trigger is relative to the current time
		 * This is only possible on IN prepositions
		 */
		if (cur->prep == USCHED_PREP_IN) {
			/* The initial trigger value is relative to the current time */
			entry_set_flag(entry, USCHED_ENTRY_FLAG_RELATIVE_TRIGGER);
		}

		/* Check if this is a THEN conjunction */
		if (cur->conj == USCHED_CONJ_THEN) {
			if (!cur->next) {
				errno = EINVAL;
				return -1;
			}

			cur = cur->next;

			/* Expect the EVERY preposition after a THEN conjunction */
			if (cur->prep != USCHED_PREP_EVERY) {
				errno = EINVAL;
				return -1;
			}

			/* Check and set align flags accordingly */
			if (bit_test(&cur->flags, USCHED_REQ_FLAG_MONTHDAY_ALIGN)) {
				/* Step must be aligned with the month day */
				entry_set_flag(entry, USCHED_ENTRY_FLAG_MONTHDAY_ALIGN);
			} else if (bit_test(&cur->flags, USCHED_REQ_FLAG_YEARDAY_ALIGN)) {
				/* Step must be aligned with the the year day */
				entry_set_flag(entry, USCHED_ENTRY_FLAG_YEARDAY_ALIGN);
			}

			/* Set entry step */
			entry_set_step(entry, (time_t) cur->arg);
		}

		/* Check if this is an UNTIL conjunction */
		if (cur->conj == USCHED_CONJ_UNTIL) {
			if (!cur->next) {
				errno = EINVAL;
				return -1;
			}

			cur = cur->next;

			/* Expect the TO preposition after an UNTIL conjunction */
			if (cur->prep != USCHED_PREP_TO) {
				errno = EINVAL;
				return -1;
			}

			entry_set_expire(entry, time_ref + cur->arg);
		} else if (cur->conj == USCHED_CONJ_WHILE) {
			/* If this is a WHILE conjunction */
			if (!cur->next) {
				errno = EINVAL;
				return -1;
			}

			cur = cur->next;

			/* Expect the IN preposition after a WHILE conjunction */
			if (cur->prep != USCHED_PREP_IN) {
				errno = EINVAL;
				return -1;
			}

			/* The expire value is relative to the current time */
			entry_set_flag(entry, USCHED_ENTRY_FLAG_RELATIVE_EXPIRE);

			/* Set the expire value */
			entry_set_expire(entry, time_ref + cur->arg);
		}

		/* If there's a conjuntion, it's expected to be AND */
		if (cur->conj && (cur->conj != USCHED_CONJ_AND)) {
			errno = EINVAL;
			return -1;
		}
	}

	/* Logic accepted */
	return 0;
}

int logic_client_process_stop(void) {
	int errsv = 0;
	char *ptr = NULL, *saveptr = NULL, *endptr = NULL;
	struct usched_client_request *cur = NULL;
	struct usched_entry *entry = NULL;
	uint64_t *entry_list = NULL;
	uint64_t entry_id = 0;
	size_t entry_list_nmemb = 0;

	/* Validate the if the current request list has at least one valid entry */
	if (!(cur = runc.req)) {
		errno = EINVAL;
		return -1;
	}

	/* Validate if this request refers to all entries beloging to this user */
	if (!strcasecmp(cur->subj, USCHED_SUBJ_ALL_STR)) {
		entry_list_nmemb = 1;

		if (!(entry_list = mm_alloc(sizeof(uint64_t))))
			return -1;

		entry_list[0] = USCHED_SUBJ_ALL; /* means all entries belonging to this user */
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
				errno = EINVAL;
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
		errno = EINVAL;
		return -1;
	}

	/* Initialize the entry to be transmitted */
	if (!(entry = entry_client_init(cur->uid, cur->gid, 0, entry_list, entry_list_nmemb * sizeof(uint64_t)))) {
		errsv = errno;
		mm_free(entry_list);
		errno = errsv;
		return -1;
	}

	/* Free entry_list */
	mm_free(entry_list);

	/* Set this entry to be deleted */
	entry_set_flag(entry, USCHED_ENTRY_FLAG_DEL);

	/* Push the entry into the entries pool */
	if (runc.epool->push(runc.epool, entry) < 0) {
		errsv = errno;
		mm_free(entry_list);
		entry_destroy(entry);
		errno = errsv;
		return -1;
	}

	/* Logic accepted */
	return 0;
}

int logic_client_process_show(void) {
	int errsv = 0;
	char *ptr = NULL, *saveptr = NULL, *endptr = NULL;
	struct usched_client_request *cur = NULL;
	struct usched_entry *entry = NULL;
	uint64_t *entry_list = NULL;
	uint64_t entry_id = 0;
	size_t entry_list_nmemb = 0;

	/* Validate the if the current request list has at least one valid entry */
	if (!(cur = runc.req)) {
		errno = EINVAL;
		return -1;
	}

	/* Validate if this request refers to all entries beloging to this user */
	if (!strcasecmp(cur->subj, USCHED_SUBJ_ALL_STR)) {
		entry_list_nmemb = 1;

		if (!(entry_list = mm_alloc(sizeof(uint64_t))))
			return -1;

		entry_list[0] = USCHED_SUBJ_ALL; /* means all entries belonging to this user */
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
				errno = EINVAL;
				return -1;
			}

			debug_printf(DEBUG_INFO, "OP == SHOW: entry_id == 0x%llX\n", entry_id);

			/* Append the extracted entry id to the current entry list */
			entry_list[entry_list_nmemb - 1] = htonll(entry_id);
		}
	}

	/* No conjunctions are accepted in a STOP operation */
	if (cur->conj) {
		mm_free(entry_list);
		errno = EINVAL;
		return -1;
	}

	/* Initialize the entry to be transmitted */
	if (!(entry = entry_client_init(cur->uid, cur->gid, 0, entry_list, entry_list_nmemb * sizeof(uint64_t)))) {
		errsv = errno;
		mm_free(entry_list);
		errno = errsv;
		return -1;
	}

	/* Free entry_list */
	mm_free(entry_list);

	/* Set this entry to be deleted */
	entry_set_flag(entry, USCHED_ENTRY_FLAG_GET);

	/* Push the entry into the entries pool */
	if (runc.epool->push(runc.epool, entry) < 0) {
		errsv = errno;
		mm_free(entry_list);
		entry_destroy(entry);
		errno = errsv;
		return -1;
	}

	/* Logic accepted */
	return 0;
}


