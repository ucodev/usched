/**
 * @file auth.h
 * @brief uSched
 *        Authentication and Authorization interface header
 *        Authentication configuration and administration header
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


#ifndef USCHED_AUTH_H
#define USCHED_AUTH_H

#include <unistd.h>

#include "config.h"

/* Prototypes */
#if CONFIG_CLIENT_ONLY == 0
int auth_daemon_local(int fd, uid_t *uid, gid_t *gid);
int auth_daemon_remote_session_create(const char *username, unsigned char *session, unsigned char *context);
int auth_daemon_remote_session_verify(const char *username, const unsigned char *session, unsigned char *context, unsigned char *agreed_key, uid_t *uid, gid_t *gid);
#endif /* CONFIG_CLIENT_ONLY == 0 */
int auth_client_remote_session_create(unsigned char *session, const char *username, const char *plain_passwd, unsigned char *context);
int auth_client_remote_session_process(unsigned char *session, const char *username, const char *plain_passwd, unsigned char *context, unsigned char *agreed_key);

/* Auth admin prototypes (usa only) */
char *auth_admin_use_local_get(void);
int auth_admin_use_local_set(const char *use_local);
char *auth_admin_users_remote_get(void);
int auth_admin_users_remote_set(const char *users_remote);

#endif

