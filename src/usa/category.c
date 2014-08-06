/**
 * @file category.c
 * @brief uSched
 *        Category processing interface
 *
 * Date: 06-08-2014
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
#include <stdlib.h>
#include <errno.h>

#include <sys/types.h>

#include "log.h"
#include "users.h"
#include "usage.h"

int category_users_add(size_t argc, char **args) {
	int errsv = 0;
	char *endptr = NULL;
	char *username = NULL;
	char *password = NULL;
	uid_t uid = (uid_t) -1;
	gid_t gid = (gid_t) -1;

	/* Usage: <username> <uid> <gid> <password> */
	if (argc != 4) {
		usage_admin_error_set(USCHED_USAGE_ADMIN_ERR_INSUFF_ARGS, "add users");
		log_warn("category_users_add(): Insufficient arguments.");
		errno = EINVAL;
		return -1;
	}

	username = args[0];
	password = args[3];

	uid = strtoul(args[1], &endptr, 0);

	if ((*endptr) || (endptr == args[1]) || (errno == EINVAL) || (errno == ERANGE)) {
		log_warn("category_users_add(): Invalid UID: %s\n", args[1]);
		errno = EINVAL;
		return -1;
	}

	gid = strtoul(args[2], &endptr, 0);

	if ((*endptr) || (endptr == args[1]) || (errno == EINVAL) || (errno == ERANGE)) {
		log_warn("category_users_add(): Invalid GID: %s\n", args[1]);
		errno = EINVAL;
		return -1;
	}

	if (users_admin_config_add(username, uid, gid, password) < 0) {
		errsv = errno;
		log_warn("category_users_add(): users_admin_config_add(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	return 0;
}

int category_users_delete(size_t argc, char **args) {
	errno = ENOSYS;
	return -1;
}

int category_users_change(size_t argc, char **args) {
	errno = ENOSYS;
	return -1;
}

int category_users_show(size_t argc, char **args) {
	errno = ENOSYS;
	return -1;
}
