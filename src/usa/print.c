/**
 * @file print.c
 * @brief uSched
 *        Printing interface - Admin
 *
 * Date: 20-02-2015
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
#include <stdint.h>

#include "config.h"
#include "print.h"

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

void print_admin_config_users(const struct usched_config_users *users) {
	struct usched_config_userinfo *user = NULL;

	printf("         username |    uid |    gid\n");

	for (users->list->rewind(users->list, 0); (user = users->list->iterate(users->list)); ) {
		if (user->username[0] == '.')
			user->username[0] = '*';

		printf(	" %16s | " \
			"%6u | " \
			"%6u\n",
			user->username,
			user->uid,
			user->gid);

		if (user->username[0] == '*')
			user->username[0] = '.';
	}
}

void print_admin_category_var_value(const char *component, const char *var, const char *value) {
	printf("%s.%s = %s\n", component, var, value);
}

