/**
 * @file file.c
 * @brief uSched
 *        File contents management interface
 *
 * Date: 05-02-2015
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
#include <unistd.h>
#include <errno.h>

#include "config.h"
#include "log.h"
#include "mm.h"
#include "file.h"


char *file_read_line_single(const char *file) {
	int errsv = 0;
	FILE *fp = NULL;
	char *line = NULL;

	if (!(fp = fopen(file, "r"))) {
		errsv = errno;
		log_crit("file_read_line_single(): fopen(\"%s\", \"r\"): %s\n", strerror(errno));
		errno = errsv;
		return NULL;
	}

	if (!(line = mm_alloc(8192))) {
		errsv = errno;
		log_crit("file_read_line_single(): mm_alloc(): %s\n", strerror(errno));
		errno = errsv;
		return NULL;
	}

	memset(line, 0, 8192);

	fgets(line, 8191, fp);

	fclose(fp);

	return line;
}

int file_write_line_single(const char *file, const char *line) {
	int errsv = 0;
	FILE *fp = NULL;

	if (unlink(file) < 0) {
		errsv = errno;
		log_crit("file_write_line_single(): unlink(\"%s\"): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	if (!(fp = fopen(file, "w+"))) {
		errsv = errno;
		log_crit("file_write_line_single(): fopen(\"%s\", \"r\"): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	fputs(line, fp);

	fclose(fp);

	return 0;
}

