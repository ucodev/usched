/**
 * @file usage.c
 * @brief uSched
 *        Usage handlers interface - Admin
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

static void _usage_admin_error_print(void) {
	switch (runa.usage_err) {
		case USCHED_USAGE_ADMIN_ERR_INVALID_OP:
			fprintf(stderr, "%s: %s\n",
				USCHED_USAGE_ADMIN_ERR_INVALID_OP_STR,
				runa.usage_err_offending ? runa.usage_err_offending : "");
			break;
		case USCHED_USAGE_ADMIN_ERR_INVALID_CATEGORY:
			fprintf(stderr, "%s: %s\n",
				USCHED_USAGE_ADMIN_ERR_INVALID_CATEGORY_STR,
				runa.usage_err_offending ? runa.usage_err_offending : "");
			break;
		case USCHED_USAGE_ADMIN_ERR_INVALID_ARG:
			fprintf(stderr, "%s: %s\n",
				USCHED_USAGE_ADMIN_ERR_INVALID_ARG_STR,
				runa.usage_err_offending ? runa.usage_err_offending : "");
			break;
		case USCHED_USAGE_ADMIN_ERR_INSUFF_ARGS:
			fprintf(stderr, "%s: %s\n",
				USCHED_USAGE_ADMIN_ERR_INSUFF_ARGS_STR,
				"");
			break;
		case USCHED_USAGE_ADMIN_ERR_TOOMANY_ARGS:
			fprintf(stderr, "%s: %s\n",
				USCHED_USAGE_ADMIN_ERR_TOOMANY_ARGS_STR,
				"");
			break;
	}
}
void usage_admin_show(void) {
	_usage_admin_error_print();

	fprintf(stderr, "Usage: %s OP CATEGORY [ ARG1 ARG2 ... ]\n", runa.argv[0]);
	fprintf(stderr, "\n");
	fprintf(stderr, "\tOP\t\tadd | delete | change | show\n");
	fprintf(stderr, "\tCATEGORY\tuser\n");
	fprintf(stderr, "\n");
}

void usage_admin_error_set(usched_usage_admin_err_t err, char *offending) {
	runa.usage_err = err;

	if (offending) {
		if (!(runa.usage_err_offending = mm_alloc(strlen(offending) + 1)))
			return; /* It's ok. We just won't be able to show the real cause */

		strcpy(runa.usage_err_offending, offending);
	}
}

