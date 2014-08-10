/**
 * @file auth.h
 * @brief uSched
 *        Authentication and Authorization interface header
 *
 * Date: 11-08-2014
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


#ifndef USCHED_AUTH_H
#define USCHED_AUTH_H

#include <unistd.h>

/* Prototypes */
int auth_daemon_local(int fd, uid_t *uid, gid_t *gid);
int auth_daemon_remote_user_token_verify(const char *username, const char *password, const char *token, uid_t *uid, gid_t *gid);
int auth_daemon_remote_user_token_create(const char *username, char *password, char *token);
int auth_client_remote_user_token_process(char *session_passwd, const char *plain_passwd);

#endif

