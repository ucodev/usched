/**
 * @file auth.c
 * @brief uSched
 *        Auth configuration and administration interface
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
#include <errno.h>

#include "config.h"
#include "auth.h"
#include "file.h"
#include "log.h"
#include "mm.h"
#include "usched.h"
#include "print.h"


void auth_admin_show(void) {
	auth_admin_local_use_show();
	auth_admin_remote_users_show();
}

int auth_admin_local_use_show(void) {
	int errsv = 0;
	char *value = NULL;

	if (!(value = file_read_line_single(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_AUTH "/" CONFIG_USCHED_FILE_AUTH_LOCAL_USE))) {
		errsv = errno;
		log_crit("auth_admin_local_use_show(): file_read_line_single(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	print_admin_category_var_value(USCHED_CATEGORY_AUTH_STR, CONFIG_USCHED_FILE_AUTH_LOCAL_USE, value);

	mm_free(value);

	return 0;
}

int auth_admin_local_use_change(const char *local_use) {
	int errsv = 0;

	if (file_write_line_single(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_AUTH "/" CONFIG_USCHED_FILE_AUTH_LOCAL_USE, local_use) < 0) {
		errsv = errno;
		log_crit("auth_admin_local_use_change(): file_write_line_single(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	if (auth_admin_local_use_show() < 0) {
		errsv = errno;
		log_crit("auth_admin_local_use_change(): auth_admin_local_use_show(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

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

