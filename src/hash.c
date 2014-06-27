/**
 * @file hash.c
 * @brief uSched
 *        Hashing mechanisms interface
 *
 * Date: 27-06-2014
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


#include <stdint.h>

#include "config.h"

#ifdef CONFIG_USCHED_HASH_FNV1A
static uint64_t _hash_fnv1a(const char *str) {
	int i = 0;
	uint64_t prime = 1099511628211ULL;		/* FNV prime */
	uint64_t hash = 14695981039346656037ULL; 	/* FNV offset basis */

	for (i = 0; str[i]; i ++) {
		hash ^= str[i];
		hash *= prime;
	}

	return hash;
}
#endif

#ifdef CONFIG_USCHED_HASH_DJB2
static uint32_t _hash_djb2(const char *str) {
	unsigned int i = 0;
	uint32_t hash = 0;

	for (i = 0; str[i]; i ++)
		hash = 31 * hash + str[i];

	return hash;
}
#endif

uint64_t hash_string_create(const char *str) {
#if defined(CONFIG_USCHED_HASH_DJB2) && !defined(CONFIG_USCHED_HASH_FNV1A)
	return _hash_djb2(str);
#elif defined(CONFIG_USCHED_HASH_FNV1A) && !defined(CONFIG_USCHED_HASH_DJB2)
	return _hash_fnv1a(str);
#else
 #error "No hashing mechanism was configured. Check the include/config.h file."
#endif
}

