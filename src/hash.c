/**
 * @file hash.c
 * @brief uSched
 *        Hashing mechanisms interface
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


#include <stdint.h>

/* TODO: A strong collision resistance hashing mechanism is required. */
uint32_t hash_string_create(const char *str) {
	unsigned int i = 0;
	uint32_t ret = 0;

	for (i = 0; str[i]; i ++)
		ret = 31 * ret + str[i];

	return ret;
}

