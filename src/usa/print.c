/**
 * @file print.c
 * @brief uSched
 *        Printing interface - Admin
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
#include <stdint.h>
#include <errno.h>

#include "config.h"
#include "print.h"
#include "log.h"

void print_admin_error(void) {
	fprintf(stderr, "An error occured. Check your syslog entries for more details.\n");
}

void print_admin_no_priv(void) {
	fprintf(stderr, "Insufficient privileges. Make sure you're running this tool as root.\n");
}

void print_admin_config_user_added(const char *username) {
	printf("User \'%s\' successfuly added.\n", username);
}

void print_admin_config_user_deleted(const char *username) {
	printf("User \'%s\' successfuly deleted.\n", username);
}

void print_admin_config_user_changed(const char *username) {
	printf("User \'%s\' successfuly changed.\n", username);
}

void print_admin_config_users_header(void) {
	printf("          username |    uid |    gid\n");
}

void print_admin_config_users_from_file(const char *file, char modification) {
	FILE *fp = NULL;
	char buf[8192], *saveptr = NULL, *username = NULL, *uid = NULL, *gid = NULL;

	memset(buf, 0, sizeof(buf));

	/* Open user configuration file */
	if (!(fp = fopen(file, "r"))) {
		fprintf(stderr, "WARNING: Cannot read file: %s\n", file);
		log_crit("print_admin_config_users_from_file(): fopen(\"%s\", \"r\"): %s\n", strerror(errno));
		return;
	}

	/* Get file contents */
	(void) fgets(buf, (int) sizeof(buf) - 1, fp);

	/* Close file */
	fclose(fp);

	/* Retrieve username */
	if (!(username = strrchr(file, '/'))) {
		fprintf(stderr, "WARNING: Invalid data detected on file: %s\n", file);
		return;
	} else {
		if (*(++ username) == '.') username ++;
	}

	/* Retrieve UID */
	if (!(uid = strtok_r(buf, ":", &saveptr))) {
		fprintf(stderr, "WARNING: Invalid data detected on file: %s\n", file);
		return;
	}

	/* Retrieve GID */
	if (!(gid = strtok_r(NULL, ":", &saveptr))) {
		fprintf(stderr, "WARNING: Invalid data detected on file: %s\n", file);
		return;
	}

	printf( " %c%16s | " \
		 "%6s | " \
		 "%6s\n",
		modification,
		username,
		uid,
		gid);
}

void print_admin_category_var_value(const char *component, const char *var, const char *value) {
	printf("%s.%s = %s\n", component, var, value);
}

