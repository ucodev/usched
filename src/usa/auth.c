/**
 * @file auth.c
 * @brief uSched
 *        Auth configuration and administration interface
 *
 * Date: 03-02-2015
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

#include "config.h"
#include "auth.h"


int auth_admin_local_use_show(void) {
	return -1;
}

int auth_admin_local_use_change(const char *use_local) {
	return -1;
}

int auth_admin_remote_users_show(void) {
	return -1;
}

int auth_admin_remote_users_change(const char *users_remote) {
	return -1;
}

