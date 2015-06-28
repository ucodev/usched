/**
 * @file conn.c
 * @brief uSched
 *        Connections interface - Common
 *
 * Date: 28-06-2015
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

#include "debug.h"
#include "conn.h"
#include "log.h"

int conn_is_local(sock_t fd) {
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

int conn_is_remote(sock_t fd) {
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

int conn_set_nonblock(sock_t fd) {
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

ssize_t conn_read_blocking(sock_t fd, void *buf, size_t count) {
	int errsv = 0;
	ssize_t len = 0, count_local = 0;

	for (len = 0, count_local = count; count_local; count_local -= len) {
		len = panet_read(fd, ((char *) buf) + (count - count_local), count_local);

		/* EOF ? */
		if (!len)
			break;

		/* Check for errors */
		if (len < 0) {
			errsv = errno;
			log_warn("conn_read_blocking(): panet_read(): %s\n", strerror(errno));
			errno = errsv;
			return -1;
		}
	}

	debug_printf(DEBUG_INFO, "conn_read_blocking(): %zd. (count: %zu, count_local: %zd)\n", count - count_local, count, count_local);

	return count - count_local;
}

ssize_t conn_write_blocking(sock_t fd, const void *buf, size_t count) {
	int errsv = 0;
	ssize_t len = 0, count_local = 0;

	for (len = 0, count_local = count; count_local; count_local -= len) {
		len = panet_write(fd, ((char *) buf) + (count - count_local), count_local);

		/* EOF ? */
		if (!len)
			break;

		/* Check for errors */
		if (len < 0) {
			errsv = errno;
			log_warn("conn_write_blocking(): panet_write(): %s\n", strerror(errno));
			errno = errsv;
			return -1;
		}
	}

	debug_printf(DEBUG_INFO, "conn_write_blocking(): %zd. (count: %zu, count_local: %zd)\n", count - count_local, count, count_local);

	return count - count_local;
}

