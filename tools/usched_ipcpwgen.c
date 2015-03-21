/**
 * @file usched_ipcpwgen.c
 * @brief uSched tools
 *        IPC password generator
 *
 * Date: 21-03-2015
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
#include <stdlib.h>

#include <psec/generate.h>

int main(int argc, char *argv[]) {
	size_t keylen = 128;
	unsigned char result[128 + 1];
	unsigned char dict[] = "ABCDEFGHIJKLMNOPQRTSUVWXYZabcdefghijklmnopqrstuvwxyz1234567890-_~/,.";

	if (argc > 2) {
		fprintf(stderr, "Usage: %s [ keylen ]\n", argv[0]);
		exit(EXIT_FAILURE);
	} else if (argc == 2) {
		if ((keylen = atoi(argv[1])) > (sizeof(result) - 1)) {
			fprintf(stderr, "Fatal: Key length too long (Maximum allowed size is %u).\n", (unsigned) sizeof(result) - 1);
			exit(EXIT_FAILURE);
		} else if (keylen < 32) {
			fprintf(stderr, "Fatal: Key length too short (Minimum allowed size is 32).\n");
			exit(EXIT_FAILURE);
		}
	}

	generate_dict_random(result, sizeof(result) - 1, dict, sizeof(dict) - 1);
	result[keylen] = 0;

	puts((char *) result);

	return 0;
}

