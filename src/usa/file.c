/**
 * @file file.c
 * @brief uSched
 *        File contents management interface
 *
 * Date: 21-02-2015
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

#include <pall/cll.h>

#include <fsop/path.h>

#include <psec/hash.h>

#include "config.h"
#include "log.h"
#include "mm.h"
#include "file.h"
#include "str.h"


static void _l_destroy(void *data) {
	mm_free(data);
}

static int _l_compare(const void *l1, const void *l2) {
	const char *bl1 = l1, *bl2 = l2;

	return strcmp(bl1, bl2);
}

char *file_read_line_single(const char *file) {
	int errsv = 0;
	FILE *fp = NULL;
	char *line = NULL;

	/* Open the file */
	if (!(fp = fopen(file, "r"))) {
		errsv = errno;
		log_crit("file_read_line_single(): fopen(\"%s\", \"r\"): %s\n", file, strerror(errno));
		errno = errsv;
		return NULL;
	}

	/* We need dynamic memory to be able to return it and grant thread safety */
	if (!(line = mm_alloc(8192))) {
		errsv = errno;
		log_crit("file_read_line_single(): mm_alloc(): %s\n", strerror(errno));
		fclose(fp);
		errno = errsv;
		return NULL;
	}

	/* Reset buffer memory, just in case */
	memset(line, 0, 8192);

	/* Fetch one line */
	fgets(line, 8191, fp);

	/* Get rid of trailling new lines */
	strrtrim(line, "\n\r");

	/* Close the file pointer */
	fclose(fp);

	/* All good */
	return line;
}

struct cll_handler *file_read_line_all_ordered(const char *file) {
	int errsv = 0;
	size_t len = 0;
	FILE *fp = NULL;
	char buf[8192], *line = NULL;
	struct cll_handler *l = NULL;

	/* Open the file */
	if (!(fp = fopen(file, "r"))) {
		errsv = errno;
		log_crit("file_read_line_all(): fopen(\"%s\", \"r\"): %s\n", file, strerror(errno));
		errno = errsv;
		return NULL;
	}

	/* Initialize the list */
	if (!(l = pall_cll_init(&_l_compare, &_l_destroy, NULL, NULL))) {
		errsv = errno;
		log_crit("file_read_line_all(): pall_cll_init(): %s\n", strerror(errno));
		fclose(fp);
		errno = errsv;
		return NULL;
	}

	/* Keep lines sorted */
	l->set_config(l, CONFIG_SEARCH_AUTO | CONFIG_INSERT_SORTED);

	/* Start reading the file */
	while (!feof(fp)) {
		/* Reset temporary buffer... just in case */
		memset(buf, 0, sizeof(buf));

		/* Fetch one line */
		fgets(buf, sizeof(buf) - 1, fp);

		/* If the line is empty, stop reading the file */
		if (!(len = strlen(buf)))
			break;

		/* Get rid of trailing new lines */
		len = strrtrim(buf, "\n\r");

		/* Allocate line memory */
		if (!(line = mm_alloc(len + 1))) {
			errsv = errno;
			log_crit("file_read_line_all(): mm_alloc(): %s\n", strerror(errno));
			pall_cll_destroy(l);
			fclose(fp);
			errno = errsv;
			return NULL;
		}

		/* Copy buffer into line */
		strcpy(line, buf);

		/* Insert line into the ordered list */
		if (l->insert(l, line) < 0) {
			errsv = errno;
			log_crit("file_read_line_all(): l->insert(): %s\n", strerror(errno));
			mm_free(line);
			pall_cll_destroy(l);
			fclose(fp);
			errno = errsv;
			return NULL;
		}
	}

	/* Close the file pointer */
	fclose(fp);

	/* All good */
	return l;
}

int file_write_line_single(const char *file, const char *line) {
	int errsv = 0;
	FILE *fp = NULL;

	/* Unlink the previous file */
	if (unlink(file) < 0) {
		errsv = errno;
		log_crit("file_write_line_single(): unlink(\"%s\"): %s\n", file, strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Open file for writting */
	if (!(fp = fopen(file, "w+"))) {
		errsv = errno;
		log_crit("file_write_line_single(): fopen(\"%s\", \"r\"): %s\n", file, strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Write the line */
	fputs(line, fp);

	/* Close the file pointer */
	fclose(fp);

	/* All good */
	return 0;
}

int file_write_line_all_ordered(const char *file, struct cll_handler *l) {
	int errsv = 0;
	FILE *fp = NULL;
	char *line = NULL;

	/* Unlink the previous file */
	if (unlink(file) < 0) {
		errsv = errno;
		log_crit("file_write_line_all_ordered(): unlink(\"%s\"): %s\n", file, strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Open file for writting */
	if (!(fp = fopen(file, "w+"))) {
		errsv = errno;
		log_crit("file_write_line_all_ordered(): fopen(\"%s\", \"w+\"): %s\n", file, strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Flush ordered data to file */
	for (l->rewind(l, 0); (line = l->iterate(l)); )
		fprintf(fp, "%s\n", line);

	/* Close file pointer */
	fclose(fp);

	/* All good */
	return 0;
}

int file_compare(const char *file1, const char *file2) {
	int errsv = 0, ret = 0;
	FILE *fp1 = NULL, *fp2 = NULL;
	unsigned char file1_hash[HASH_DIGEST_SIZE_BLAKE2B], file2_hash[HASH_DIGEST_SIZE_BLAKE2B];

	if (!(fp1 = fopen(file1, "r"))) {
		errsv = errno;
		log_crit("file_compare(): fopen(\"%s\", \"r\"): %s\n", file1, strerror(errno));
		errno = errsv;
		return -1;
	}

	if (!(fp2 = fopen(file2, "r"))) {
		errsv = errno;
		log_crit("file_compare(): fopen(\"%s\", \"r\"): %s\n", file2, strerror(errno));
		fclose(fp1);
		errno = errsv;
		return -1;
	}

	ret = !memcmp(hash_file_blake2b(file1_hash, fp1), hash_file_blake2b(file2_hash, fp2), HASH_DIGEST_SIZE_BLAKE2B);

	fclose(fp1);
	fclose(fp2);

	return ret;
}

int file_is_empty(const char *file) {
	int errsv = 0;
	struct stat st;

	if (stat(file, &st) < 0) {
		errsv = errno;
		log_crit("file_is_empty(): stat(\"%s\", ...): %s\n", file, strerror(errno));
		errno = errsv;
		return -1;
	}

	if (!S_ISREG(st.st_mode)) {
		log_crit("file_is_empty(): File '%s' isn't a regular file.\n");
		errno = EINVAL;
		return -1;
	}

	return !st.st_size;
}

