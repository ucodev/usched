/**
 * @file users.h
 * @brief uSched
 *        Users configuration interface header
 *
 * Date: 19-02-2015
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

#ifndef USCHED_USERS_H
#define USCHED_USERS_H

#include <sys/types.h>

/* Prototypes */
int users_admin_commit(void);
int users_admin_rollback(void);
int users_admin_add(const char *username, uid_t uid, gid_t gid, const char *password);
int users_admin_delete(const char *username);
int users_admin_change(const char *username, uid_t uid, gid_t gid, const char *password);
int users_admin_show(void);

#endif

