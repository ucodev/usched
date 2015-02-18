/**
 * @file str.c
 * @brief uSched
 *        String helper interface
 *
 * Date: 18-02-2015
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
#include <string.h>

#include "config.h"
#include "mm.h"
#include "str.h"

static char *_strrepl_generic(
		const char *haystack,
		const char *needle,
		const char *rcontent,
		size_t *start_index)
{
	char *occurrence = NULL, *str_new = NULL;
	size_t len_haystack = 0, len_needle = 0, len_rcontent = 0, len_new = 0;

	/* Reject any argument that is NULL */
	if (!haystack || !needle || !rcontent)
		return NULL;

	/* Search for at least one occurrence */
	if (!(occurrence = strstr(haystack + *start_index, needle)))
		return NULL;

	/* Compute the arguments length */
	len_haystack = strlen(haystack);
	len_needle = strlen(needle);
	len_rcontent = strlen(rcontent);

	/* Length of haystack and needle must be non-zero */
	if (!len_haystack || !len_needle)
		return NULL;

	/* Compute the length of the result */
	len_new = len_haystack - len_needle + len_rcontent;

	/* Alloc enough memoery for the result */
	if (!(str_new = mm_alloc(len_new + 1)))
		return NULL;

	/* Reset all memory */
	memset(str_new, 0, len_new + 1);

	/* Craft the first part of the result (if any) */
	memcpy(str_new, haystack, occurrence - haystack);

	/* Replace the occurrence */
	memcpy(str_new + (occurrence - haystack), rcontent, len_rcontent);

	/* Craft the third part of the result (if any) */
	strcat(str_new, occurrence + len_needle);

	/* Update start index */
	*start_index = occurrence - haystack + len_rcontent;

	/* Return the result */
	return str_new;
}

char *strrepl(const char *haystack, const char *needle, const char *rcontent) {
	return _strrepl_generic(haystack, needle, rcontent, (size_t [1]) { 0 });
}

char *strreplall(const char *haystack, const char *needle, const char *rcontent) {
	const char *str_new = NULL, *prev = haystack;
	size_t start_index = 0;

	/* Keep replacing the haystack while we find needles on it */
	while ((str_new = _strrepl_generic(prev, needle, rcontent, &start_index))) {
		if (prev != haystack)
			mm_free((void *) prev); /* Safe to consider it non-const */

		prev = str_new;
	}

	/* If prev != haystack, it's safe to consider it non-const */
	return (prev == haystack) ? NULL : (char *) prev;
}

int strisnum(const char *s) {
	for ( ; *s; s ++) if ((*s < 48) || (*s > 57)) return 0;

	return 1;
}

int strrtrim(char *s, const char *trail) {
	size_t len = strlen(s);
	char ch = trail[0];
	int i = 0;

	while (ch) ch = (s[len - 1] == ch) ? trail[i = s[-- len] = 0] : trail[++ i];

	return strlen(s) - len;
}

