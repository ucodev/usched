/**
 * @file input.c
 * @brief uSched
 *        Terminal input interface
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

#include <stdio.h>
#include <string.h>

#ifdef COMPILE_WIN32
#include <conio.h>
#endif

#include "config.h"
#include "term.h"
#include "str.h"

int input_password(char *password, size_t max_len) {
#ifdef COMPILE_WIN32
	int ch = 0;
	size_t count = 0;

	memset(password, 0, max_len);

	for (;;) {
		if (count >= max_len)
			break;

		ch = _getch();

		if (ch == '\r' || ch == '\n')
			break;

		password[count ++] = ch;
	}
#else
	memset(password, 0, max_len);

	if (term_local_echo_unset() < 0)
		return -1;

	(void) fgets(password, (int) max_len - 1, stdin);

	/* Strip '\n' and/or '\r' */
	(void) strrtrim(password, "\n\r");

	if (term_local_echo_set() < 0)
		return -1;

#endif
	return 0;
}

