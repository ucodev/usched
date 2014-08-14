/**
 * @file input.c
 * @brief uSched
 *        Terminal input interface
 *
 * Date: 14-08-2014
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

#include "term.h"

int input_password(char *password, size_t max_len) {
	size_t len = 0;
	memset(password, 0, max_len);

	if (term_local_echo_unset() < 0)
		return -1;

	fgets(password, max_len - 1, stdin);

	len = strlen(password);

	while (password[len - 1] == '\n')
		password[-- len] = 0;

	if (term_local_echo_set() < 0)
		return -1;

	return 0;
}

