/**
 * @file auth.c
 * @brief uSched
 *        Authentication and Authorization interface
 *
 * Date: 24-06-2014
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
#include <errno.h>
#include <unistd.h>

#include "log.h"
#include "local.h"

int auth_local(int fd, uid_t *uid, gid_t *gid) {
	if (local_fd_peer_cred(fd, uid, gid) < 0) {
		log_warn("auth_local(): local_fd_peer_cred(): %s\n", strerror(errno));
		return -1;
	}

	return 0;
}

int auth_remote(int fd, const char *user, const char *passwd) {
	errno = ENOSYS;

	return -1;
}

