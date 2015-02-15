/**
 * @file auth.c
 * @brief uSched
 *        Auth configuration and administration interface
 *
 * Date: 15-02-2015
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
#include <errno.h>

#include <pall/cll.h>

#include "config.h"
#include "auth.h"
#include "file.h"
#include "log.h"
#include "mm.h"
#include "usched.h"
#include "print.h"
#include "str.h"

static void _l_destroy(void *data) {
	mm_free(data);
}

static int _l_compare(const void *l1, const void *l2) {
	const char *bl1 = l1, *bl2 = l2;

	return strcmp(bl1, bl2);
}

void auth_admin_show(void) {
	auth_admin_blacklist_gid_show();
	auth_admin_blacklist_uid_show();
	auth_admin_local_use_show();
	auth_admin_remote_users_show();
	auth_admin_whitelist_gid_show();
	auth_admin_whitelist_uid_show();
}

int auth_admin_blacklist_gid_show(void) {
	int errsv = 0;
	struct cll_handler *l = NULL;
	char *s_val = NULL, *output = NULL;
	size_t output_len = 0;
	int output_alloc = 0;

	/* Read the values from file */
	if (!(l = file_read_line_all_ordered(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_AUTH "/" CONFIG_USCHED_FILE_AUTH_BL_GID))) {
		errsv = errno;
		log_crit("auth_admin_blacklist_gid_show(): file_read_line_all_ordered(\"%s\"): %s\n", CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_AUTH "/" CONFIG_USCHED_FILE_AUTH_BL_GID, strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Iterate the list and create an output string */
	for (l->rewind(l, 0); (s_val = l->iterate(l)); ) {
		/* Compute the new output length */
		output_len += strlen(s_val) + 1; /* ',' */

		/* Reallocate the output memory */
		if (!(output = mm_realloc(output, output_len + 1))) {
			errsv = errno;
			log_crit("auth_admin_blacklist_gid_show(): mm_realloc(): %s\n", strerror(errno));
			pall_cll_destroy(l);
			errno = errsv;
			return -1;
		}

		/* Check if output was already alloc'd */
		if (!output_alloc) {
			/* If not, zero it's memory and inform next iterations that it's now
			 * alloc'd
			 */
			memset(output, 0, output_len + 1);
			output_alloc = 1;
		}

		/* Concat the value to the current output... */
		strcat(output, s_val);

		/* ... and add a trailing "," separator */
		strcat(output, ",");
	}

	/* Check if we've some values to print out */
	if (output) {
		/* Remove trailing "," */
		output[output_len - 1] = 0;

		/* Print the output */
		print_admin_category_var_value(USCHED_CATEGORY_AUTH_STR, CONFIG_USCHED_FILE_AUTH_BL_GID, output);

		/* Free the memory */
		mm_free(output);
	} else {
		/* Empty value */
		output = "";

		/* Print the output */
		print_admin_category_var_value(USCHED_CATEGORY_AUTH_STR, CONFIG_USCHED_FILE_AUTH_BL_GID, output);
	}

	/* Destroy the lines list */
	pall_cll_destroy(l);

	/* All good */
	return 0;
}

int auth_admin_blacklist_gid_change(const char *blacklist_gid_list) {
	int errsv = 0;
	char *ptr = NULL, *saveptr = NULL, *line = NULL, *input = NULL;
	struct cll_handler *l = NULL;

	/* Initialize the lines list */
	if (!(l = pall_cll_init(&_l_compare, &_l_destroy, NULL, NULL))) {
		errsv = errno;
		log_crit("auth_admin_blacklist_gid_change(): pall_cll_init(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Duplicate the the input value to get rid of the const (avoid strdup() to allow the use
	 * of mm_*() interface
	 */
	if (!(input = mm_alloc(strlen(blacklist_gid_list) + 1))) {
		errsv = errno;
		log_crit("auth_admin_blacklist_gid_change(): mm_alloc(): %s\n", strerror(errno));
		pall_cll_destroy(l);
		errno = errsv;
		return -1;
	}

	/* Populate input */
	strcpy(input, blacklist_gid_list);

	/* Parse the input value */
	for (ptr = input; (ptr = strtok_r(ptr, ",", &saveptr)); ptr = NULL) {
		/* Check if value is numerical */
		if (!strisnum(ptr)) {
			log_crit("auth_admin_blacklist_gid_change(): Value '%s' isn't numerical.", ptr);
			pall_cll_destroy(l);
			errno = EINVAL;
			return -1;
		}

		/* Allocate line memory */
		if (!(line = mm_alloc(strlen(ptr) + 1))) {
			errsv = errno;
			log_crit("auth_admin_blacklist_gid_change(): mm_alloc(): %s\n", strerror(errno));
			pall_cll_destroy(l);
			errno = errsv;
			return -1;
		}

		/* Populate line */
		strcpy(line, ptr);

		/* Insert line into the lines list */
		if (l->insert(l, line) < 0) {
			errsv = errno;
			log_crit("auth_admin_blacklist_gid_change(): mm_alloc(): %s\n", strerror(errno));
			mm_free(l);
			pall_cll_destroy(l);
			errno = errsv;
			return -1;
		}
	}

	/* Free input memory */
	mm_free(input);

	/* Write lines to file */
	if (file_write_line_all_ordered(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_AUTH "/" CONFIG_USCHED_FILE_AUTH_BL_GID, l) < 0) {
		errsv = errno;
		log_crit("auth_admin_blacklist_gid_change(): file_write_line_all_ordered(): %s\n", strerror(errno));
		pall_cll_destroy(l);
		errno = errsv;
		return -1;
	}

	/* Cleanup lines list */
	pall_cll_destroy(l);

	/* Show the new configuration value */
	if (auth_admin_blacklist_gid_show() < 0) {
		errsv = errno;
		log_crit("auth_admin_blacklist_gid_change(): auth_admin_blacklist_gid_show(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* All good */
	return 0;
}

int auth_admin_blacklist_gid_add(const char *blacklist_gid) {
	int errsv = 0;
	char *val = NULL;
	struct cll_handler *l = NULL;

	/* Check if the value is valid */
	if (!strisnum(blacklist_gid)) {
		log_crit("auth_admin_blacklist_gid_add(): Value '%s' isn't numerical\n", blacklist_gid);
		errno = EINVAL;
		return -1;
	}

	/* Alocate value memory */
	if (!(val = mm_alloc(strlen(blacklist_gid) + 1))) {
		errsv = errno;
		log_crit("auth_admin_blacklist_gid_add(): mm_alloc(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Populate value */
	strcpy(val, blacklist_gid);

	/* Read the values from file */
	if (!(l = file_read_line_all_ordered(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_AUTH "/" CONFIG_USCHED_FILE_AUTH_BL_GID))) {
		errsv = errno;
		log_crit("auth_admin_blacklist_gid_add(): file_read_line_all_ordered(\"%s\"): %s\n", CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_AUTH "/" CONFIG_USCHED_FILE_AUTH_BL_GID, strerror(errno));
		mm_free(val);
		errno = errsv;
		return -1;
	}

	/* Check if there's a match */
	if (l->search(l, val)) {
		log_crit("auth_admin_blacklist_gid_add(): Value '%s' already present on the configuration list\n", val);
		mm_free(val);
		pall_cll_destroy(l);
		errno = EINVAL;
		return -1;
	}

	/* Insert value into the ordered list */
	if (l->insert(l, val) < 0) {
		errsv = errno;
		log_crit("auth_admin_blacklist_gid_add(): l->insert(): %s\n", strerror(errno));
		mm_free(val);
		pall_cll_destroy(l);
		errno = errsv;
		return -1;
	}

	/* Write lines to file */
	if (file_write_line_all_ordered(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_AUTH "/" CONFIG_USCHED_FILE_AUTH_BL_GID, l) < 0) {
		errsv = errno;
		log_crit("auth_admin_blacklist_gid_add(): file_write_line_all_ordered(): %s\n", strerror(errno));
		pall_cll_destroy(l);
		errno = errsv;
		return -1;
	}

	/* Cleanup lines list */
	pall_cll_destroy(l);

	/* Show the new configuration value */
	if (auth_admin_blacklist_gid_show() < 0) {
		errsv = errno;
		log_crit("auth_admin_blacklist_gid_add(): auth_admin_blacklist_gid_show(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* All good */
	return 0;
}

int auth_admin_blacklist_gid_delete(const char *blacklist_gid) {
	int errsv = 0;
	struct cll_handler *l = NULL;

	/* Check if the value is valid */
	if (!strisnum(blacklist_gid)) {
		log_crit("auth_admin_blacklist_gid_del(): Value '%s' isn't numerical\n", blacklist_gid);
		errno = EINVAL;
		return -1;
	}

	/* Read the values from file */
	if (!(l = file_read_line_all_ordered(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_AUTH "/" CONFIG_USCHED_FILE_AUTH_BL_GID))) {
		errsv = errno;
		log_crit("auth_admin_blacklist_gid_del(): file_read_line_all_ordered(\"%s\"): %s\n", CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_AUTH "/" CONFIG_USCHED_FILE_AUTH_BL_GID, strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Check if there's a match */
	if (!l->search(l, (void *) blacklist_gid)) { /* Safe to discard const */
		log_crit("auth_admin_blacklist_gid_del(): Value '%s' not found on current configuration\n", blacklist_gid);
		pall_cll_destroy(l);
		errno = EINVAL;
		return -1;
	}

	/* Delete the value from list */
	l->del(l, (void *) blacklist_gid); /* Safe to discard const */

	/* Write lines to file */
	if (file_write_line_all_ordered(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_AUTH "/" CONFIG_USCHED_FILE_AUTH_BL_GID, l) < 0) {
		errsv = errno;
		log_crit("auth_admin_blacklist_gid_del(): file_write_line_all_ordered(): %s\n", strerror(errno));
		pall_cll_destroy(l);
		errno = errsv;
		return -1;
	}

	/* Cleanup lines list */
	pall_cll_destroy(l);

	/* Show the new configuration value */
	if (auth_admin_blacklist_gid_show() < 0) {
		errsv = errno;
		log_crit("auth_admin_blacklist_gid_del(): auth_admin_blacklist_gid_show(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* All good */
	return 0;
}

int auth_admin_blacklist_uid_show(void) {
	return -1;
}

int auth_admin_blacklist_uid_change(const char *blacklist_uid_list) {
	return -1;
}

int auth_admin_blacklist_uid_add(const char *blacklist_uid) {
	return -1;
}

int auth_admin_blacklist_uid_delete(const char *blacklist_uid) {
	return -1;
}

int auth_admin_local_use_show(void) {
	int errsv = 0;
	char *value = NULL;

	/* Retrieve the value from the configuration file */
	if (!(value = file_read_line_single(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_AUTH "/" CONFIG_USCHED_FILE_AUTH_LOCAL_USE))) {
		errsv = errno;
		log_crit("auth_admin_local_use_show(): file_read_line_single(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Print the value */
	print_admin_category_var_value(USCHED_CATEGORY_AUTH_STR, CONFIG_USCHED_FILE_AUTH_LOCAL_USE, value);

	/* Free the memory */
	mm_free(value);

	/* All good */
	return 0;
}

int auth_admin_local_use_change(const char *local_use) {
	int errsv = 0;

	/* Write the new value into the configuration file */
	if (file_write_line_single(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_AUTH "/" CONFIG_USCHED_FILE_AUTH_LOCAL_USE, local_use) < 0) {
		errsv = errno;
		log_crit("auth_admin_local_use_change(): file_write_line_single(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Show the new configuration value */
	if (auth_admin_local_use_show() < 0) {
		errsv = errno;
		log_crit("auth_admin_local_use_change(): auth_admin_local_use_show(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* All good */
	return 0;
}

int auth_admin_remote_users_show(void) {
	int errsv = 0;
	char *value = NULL;

	if (!(value = file_read_line_single(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_AUTH "/" CONFIG_USCHED_FILE_AUTH_REMOTE_USERS))) {
		errsv = errno;
		log_crit("auth_admin_remote_users_show(): file_read_line_single(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	print_admin_category_var_value(USCHED_CATEGORY_AUTH_STR, CONFIG_USCHED_FILE_AUTH_REMOTE_USERS, value);

	mm_free(value);

	return 0;
}

int auth_admin_remote_users_change(const char *remote_users) {
	int errsv = 0;

	if (file_write_line_single(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_AUTH "/" CONFIG_USCHED_FILE_AUTH_REMOTE_USERS, remote_users) < 0) {
		errsv = errno;
		log_crit("auth_admin_remote_users_change(): file_write_line_single(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	if (auth_admin_remote_users_show() < 0) {
		errsv = errno;
		log_crit("auth_admin_remote_users_change(): auth_admin_remote_users_show(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	return 0;
}

int auth_admin_whitelist_gid_show(void) {
	return -1;
}

int auth_admin_whitelist_gid_change(const char *blacklist_gid_list) {
	return -1;
}

int auth_admin_whitelist_gid_add(const char *blacklist_gid) {
	return -1;
}

int auth_admin_whitelist_gid_delete(const char *blacklist_gid) {
	return -1;
}

int auth_admin_whitelist_uid_show(void) {
	return -1;
}

int auth_admin_whitelist_uid_change(const char *blacklist_uid_list) {
	return -1;
}

int auth_admin_whitelist_uid_add(const char *blacklist_uid) {
	return -1;
}

int auth_admin_whitelist_uid_delete(const char *blacklist_uid) {
	return -1;
}

