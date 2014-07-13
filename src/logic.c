/**
 * @file logic.c
 * @brief uSched
 *        Logic Analyzer interface
 *
 * Date: 11-07-2014
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
#include "runtime.h"
#include "entry.h"
#include "logic.h"


int logic_process_run(void) {
	struct usched_request *cur = NULL;
	struct usched_entry *entry = NULL;
	time_t time_ref = runc.t;

	for (cur = runc.req; cur; cur = cur->next, runc.epool->push(runc.epool, entry)) {
		/* Allocate a new scheduling entry with subject as its payload. */
		if (!(entry = entry_client_init(cur->uid, cur->gid, time_ref + cur->arg, cur->subj)))
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
	/* TODO: To be implemented */
	return -1;
}

int logic_process_show(void) {
	/* TODO: To be implemented */
	return -1;
}


