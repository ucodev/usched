/**
 * @file all.c
 * @brief uSched
 *       ALL Category administration interface
 *
 * Date: 18-02-2015
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

#include "all.h"
#include "auth.h"
#include "core.h"
#include "network.h"
#include "users.h"

void all_admin_show(void) {
	auth_admin_show();
	core_admin_show();
	network_admin_show();
	users_admin_config_show();
}

int all_admin_commit(void) {
	return -1;
}

int all_admin_rollback(void) {
	return -1;
}

