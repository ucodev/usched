/**
 * @file str.c
 * @brief uSched
 *        String helper interface
 *
 * Date: 06-02-2015
 * 
 * Copyright 2014-2015 Pedro A. Hortas (pah@ucodev.org)
 *
 * This file is part of usched.
 *
 * usched is mm_free software: you can redistribute it and/or modify
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
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "str.h"
#include "mm.h"

char *strrepl(const char *haystack, const char *needle, const char *rcontent) {
	char *occurrence = NULL, *str_new = NULL;
	size_t len_new = 0;

	/* Reject any argument that is NULL */
	if (!haystack || !needle || !rcontent)
		return NULL;

	/* Search for at least one occurrence */
	if (!(occurrence = strstr(haystack, needle)))
		return NULL;

	/* Compute the length of the result */
	len_new = strlen(haystack) - strlen(needle) + strlen(rcontent);

	/* Alloc enough memoery for the result */
	if (!(str_new = mm_alloc(len_new + 1)))
		return NULL;

	/* Reset all memory */
	memset(str_new, 0, len_new + 1);

	/* Craft the first part of the result (if any) */
	memcpy(str_new, haystack, occurrence - haystack);

	/* Replace the occurrence */
	memcpy(str_new + (occurrence - haystack), rcontent, strlen(rcontent));

	/* Craft the third part of the result (if any) */
	strcat(str_new, occurrence + strlen(needle));

	/* Return the result */
	return str_new;
}

char *strreplall(const char *haystack, const char *needle, const char *rcontent) {
	const char *str_new = NULL, *prev = haystack;

	/* Keep replacing the haystack while we find needles on it */
	while ((str_new = strrepl(prev, needle, rcontent))) {
		if (prev != haystack)
			mm_free((void *) prev); /* Safe to consider it non-const */

		prev = str_new;
	}

	/* If prev != haystack, it's safe to consider it non-const */
	return (prev == haystack) ? NULL : (char *) prev;
}

