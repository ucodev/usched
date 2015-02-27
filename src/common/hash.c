/**
 * @file hash.c
 * @brief uSched
 *        Hashing mechanisms interface
 *
 * Date: 27-02-2015
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


#include <stdint.h>

#include "config.h"

#if CONFIG_USCHED_HASH_FNV1A == 1
static uint64_t _hash_fnv1a(const char *str) {
	int i = 0;
	uint64_t prime = (uint64_t) 0x100000001B3ULL;		/* FNV prime */
	uint64_t hash = (uint64_t) 0xCBF29CE484222325ULL; 	/* FNV offset basis */

	for (i = 0; str[i]; i ++) {
		hash ^= str[i];
		hash *= prime;
	}

	return hash;
}
#endif

#if CONFIG_USCHED_HASH_DJB2 == 1
static uint32_t _hash_djb2(const char *str) {
	unsigned int i = 0;
	uint32_t hash = 0;

	for (i = 0; str[i]; i ++)
		hash = 31 * hash + str[i];

	return hash;
}
#endif

uint64_t hash_string_create(const char *str) {
#if CONFIG_USCHED_HASH_DJB2 == 1 && CONFIG_USCHED_HASH_FNV1A == 0
	return _hash_djb2(str);
#elif CONFIG_USCHED_HASH_FNV1A == 1 && CONFIG_USCHED_HASH_DJB2 == 0
	return _hash_fnv1a(str);
#else
 #error "No hashing mechanism was configured or a conflict was detected. Check the include/config.h file."
#endif
}

