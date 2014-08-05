/**
 * @file usage.c
 * @brief uSched
 *        Usage handlers interface - Client
 *
 * Date: 05-08-2014
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

#include "mm.h"
#include "runtime.h"
#include "usage.h"

/* usc run 'script' on day 30
 * 0      1   2        3  4   5
 */

static void _usage_client_error_print(void) {
	switch (runc.usage_err) {
		case USCHED_USAGE_CLIENT_ERR_INVALID_OP:
			fprintf(stderr, "%s: %s\n",
				USCHED_USAGE_CLIENT_ERR_INVALID_OP_STR,
				runc.usage_err_offending ? runc.usage_err_offending : "");
			break;
		case USCHED_USAGE_CLIENT_ERR_INVALID_PREP:
			fprintf(stderr, "%s: %s\n",
				USCHED_USAGE_CLIENT_ERR_INVALID_PREP_STR,
				runc.usage_err_offending ? runc.usage_err_offending : "");
			break;
		case USCHED_USAGE_CLIENT_ERR_INVALID_ADVERB:
			fprintf(stderr, "%s: %s\n",
				USCHED_USAGE_CLIENT_ERR_INVALID_ADVERB_STR,
				runc.usage_err_offending ? runc.usage_err_offending : "");
			break;
		case USCHED_USAGE_CLIENT_ERR_INVALID_CONJ:
			fprintf(stderr, "%s: %s\n",
				USCHED_USAGE_CLIENT_ERR_INVALID_CONJ_STR,
				runc.usage_err_offending ? runc.usage_err_offending : "");
			break;
		case USCHED_USAGE_CLIENT_ERR_INVALID_ARG:
			fprintf(stderr, "%s: %s\n",
				USCHED_USAGE_CLIENT_ERR_INVALID_ARG_STR,
				runc.usage_err_offending ? runc.usage_err_offending : "");
			break;
		case USCHED_USAGE_CLIENT_ERR_UNEXPECT_PREP:
			fprintf(stderr, "%s: %s\n",
				USCHED_USAGE_CLIENT_ERR_UNEXPECT_PREP_STR,
				runc.usage_err_offending ? runc.usage_err_offending : "");
			break;
		case USCHED_USAGE_CLIENT_ERR_UNEXPECT_CONJ:
			fprintf(stderr, "%s: %s\n",
				USCHED_USAGE_CLIENT_ERR_UNEXPECT_CONJ_STR,
				runc.usage_err_offending ? runc.usage_err_offending : "");
			break;
		case USCHED_USAGE_CLIENT_ERR_INSUFF_ARGS:
			fprintf(stderr, "%s: %s\n",
				USCHED_USAGE_CLIENT_ERR_INSUFF_ARGS_STR,
				"");
			break;
		case USCHED_USAGE_CLIENT_ERR_TOOMANY_ARGS:
			fprintf(stderr, "%s: %s\n",
				USCHED_USAGE_CLIENT_ERR_TOOMANY_ARGS_STR,
				"");
			break;
	}
}

void usage_client_show(void) {
	_usage_client_error_print();
		
	fprintf(stderr, "Usage: %s OP SUBJ { PREP [ ADVERB ARG | ARG ADVERB ] [ CONJ ... ] }\n", runc.argv[0]);
	fprintf(stderr, "\n");
	fprintf(stderr,     "\tOP\t\trun     | stop     | show\n");
	fprintf(stderr,   "\tPREP\t\tevery   | in       | now   | on    | to\n");
	fprintf(stderr, "\tADVERB\t\tseconds | minutes  | hours | days  | weeks    | months\n");
	fprintf(stderr,       "\t\t\tyears   | weekdays | time  | date  | datetime | timestamp\n");
	fprintf(stderr,   "\tCONJ\t\tand     | then     | until | while\n");
	fprintf(stderr, "\n");
}


void usage_client_error_set(usched_usage_client_err_t err, char *offending) {
	runc.usage_err = err;

	if (offending) {
		if (!(runc.usage_err_offending = mm_alloc(strlen(offending) + 1)))
			return; /* It's ok. We just won't be able to show the real cause */

		strcpy(runc.usage_err_offending, offending);
	}
}

