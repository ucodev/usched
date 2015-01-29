/**
 * @file conn.c
 * @brief uSched
 *        Connections interface - Common
 *
 * Date: 29-01-2015
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

#ifndef COMPILE_WIN32
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>

#include <panet/panet.h>

#include "conn.h"
#include "log.h"

int conn_is_local(int fd) {
#ifdef COMPILE_WIN32
	return 0;
#else
	int errsv = 0;
	int ret = panet_info_sock_family(fd);

	if (ret < 0) {
		errsv = errno;
		log_warn("conn_is_local(): panet_info_sock_family(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	return (ret == AF_UNIX);
#endif
}

int conn_is_remote(int fd) {
#ifdef COMPILE_WIN32
	return 1;
#else
	int errsv = 0;
	int ret = panet_info_sock_family(fd);

	if (ret < 0) {
		errsv = errno;
		log_warn("conn_is_remote(): panet_info_sock_family(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	return (ret != AF_UNIX);
#endif
}

int conn_set_nonblock(int fd) {
#ifdef COMPILE_WIN32
	return 0;
#else
	int errsv = 0, flags = 0;

	if ((flags = fcntl(fd, F_GETFL)) < 0) {
		errsv = errno;
		log_warn("conn_set_nonblock(): fcntl(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	if (!(flags & O_NONBLOCK)) {
		if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0) {
			errsv = errno;
			log_warn("conn_set_nonblock(): fcntl(): %s\n", strerror(errno));
			errno = errsv;
			return -1;
		}
	}

	return 0;
#endif
}
