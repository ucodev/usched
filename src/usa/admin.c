/**
 * @file admin.c
 * @brief uSched
 *        Administration interface
 *
 * Date: 12-05-2015
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
#include <errno.h>
#include <stdlib.h>

#include "runtime.h"
#include "log.h"
#include "print.h"
#include "op.h"


static void _init(int argc, char **argv) {
	if (runtime_admin_init(argc, argv) < 0) {
		log_crit("_init(): runtime_admin_init(): %s\n", strerror(errno));
		print_admin_error();
		exit(EXIT_FAILURE);
	}
}

static void _do(void) {
	if (op_admin_process() < 0)
		print_admin_error();
}

static void _destroy(void) {
	runtime_admin_destroy();
}

int main(int argc, char *argv[]) {
	_init(argc, argv);

	_do();

	_destroy();

	return 0;
}

int admin_property_show(const char *category_dir, const char *category_str, const char *property_file) {
	int errsv = 0;
	char *value = NULL, *value_tmp = NULL, *value_print = NULL, *prop_path = NULL, *prop_path_tmp = NULL;
	size_t prop_path_tmp_len = sizeof(CONFIG_USCHED_DIR_BASE) + strlen(category_dir) + 2 + strlen(property_file) + 1;

	/* Allocate temporary property file path buffer */
	if (!(prop_path_tmp = mm_alloc(prop_path_tmp_len))) {
		errsv = errno;
		log_crit("admin_property_show(): mm_alloc(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Reset temporary buffer */
	memset(prop_path_tmp, 0, prop_path_len);

	/* Craft temporary property absolute file path */
	sprintf(prop_path_tmp, "%s/%s/.%s", CONFIG_USCHED_DIR_BASE, category_dir, property_file);

	/* Allocate effective property file path buffer */
	if (!(prop_path = mm_alloc(prop_path_tmp_len - 1))) {
		errsv = errno;
		log_crit("admin_property_show(): mm_alloc(): %s\n", strerror(errno));
		mm_free(prop_path_tmp);
		errno = errsv;
		return -1;
	}

	/* Reset effective buffer */
	memset(prop_path, 0, prop_path_tmp_len - 1);

	/* Craft effective property absolute file path */
	sprintf(prop_path, "%s/%s/%s", CONFIG_USCHED_DIR_BASE, category_dir, property_file);

	if (!(value_tmp = file_read_line_single(prop_path_tmp))) {
		errsv = errno;
		log_crit("admin_property_show(): file_read_line_single(): %s\n", strerror(errno));
		mm_free(prop_path_tmp);
		mm_free(prop_path);
		errno = errsv;
		return -1;
	}

	if (!(value = file_read_line_single(prop_path))) {
		errsv = errno;
		log_crit("admin_property_show(): file_read_line_single(): %s\n", strerror(errno));
		mm_free(prop_path_tmp);
		mm_free(prop_path);
		mm_free(value_tmp);
		errno = errsv;
		return -1;
	}

	mm_free(prop_path_tmp);
	mm_free(prop_path);

	/* Check which value to print */
	if (!strcmp(value, value_tmp)) {
		if (!(value_print = mm_alloc(strlen(value) + 1))) {
			errsv = errno;
			log_crit("admin_property_show(): mm_alloc(): %s\n", strerror(errno));
			mm_free(value);
			mm_free(value_tmp);
			errno = errsv;
			return -1;
		}

		strcpy(value_print, value);
	} else {
		if (!(value_print = mm_alloc(strlen(value_tmp) + 2))) {
			errsv = errno;
			log_crit("admin_property_show(): mm_alloc(): %s\n", strerror(errno));
			mm_free(value);
			mm_free(value_tmp);
			errno = errsv;
			return -1;
		}

		/* Show the temporary value with a trailing '*' */
		strcpy(value_print, value_tmp);
		strcat(value_print, "*");
	}

	/* Print the output */
	print_admin_category_var_value(category_str, property_file, value_print);

	/* Free memory */
	mm_free(value);
	mm_free(value_tmp);
	mm_free(value_print);

	/* All good */
	return 0;
}

int admin_property_change(const char *category_dir, const char *property_file, const char *value) {
	int errsv = 0;
	char *prop_path = NULL;
	size_t prop_path_len = sizeof(CONFIG_USCHED_DIR_BASE) + strlen(category_dir) + 2 + strlen(property_file) + 1;

	/* Allocate property absolute file path buffer */
	if (!(prop_path = mm_alloc(prop_path_len))) {
		errsv = errno;
		log_crit("admin_property_change(): mm_alloc(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Reset buffer */
	memset(prop_path, 0, prop_path_len);

	/* Craft property absolute file path */
	sprintf(prop_path, "%s/%s/.%s", CONFIG_USCHED_DIR_BASE, category_dir, property_file);

	/* Write changes to property file */
	if (file_write_line_single(prop_path, value) < 0) {
		errsv = errno;
		log_crit("admin_property_change(): file_write_line_single(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Release prop path buffer */
	mm_free(prop_path);

	/* All good */
	return 0;
}

