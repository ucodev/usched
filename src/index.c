/**
 * @file index.c
 * @brief uSched
 *        Indexing interface
 *
 * Date: 24-06-2014
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

#include "mm.h"
#include "entry.h"
#include "hash.h"

int index_entry_create(struct usched_entry *e) {
	char *str = NULL;
	size_t len = strlen(e->payload) + 1 + 60 + 1;

	if (!(str = mm_alloc(len)))
		return -1;

	memset(str, 0, len);

	snprintf(str, len - 1, "%s%u%u%u", e->payload, e->trigger, e->step, e->expire);

	e->id = hash_string_create(str);

	mm_free(str);

	return 0;
}

