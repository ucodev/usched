/**
 * @file conn.h
 * @brief uSched
 *        Connections interface header
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


#ifndef USCHED_CONN_H
#define USCHED_CONN_H

#include <string.h>

#include "config.h"

#ifndef COMPILE_WIN32
#include <arpa/inet.h>
#endif

/* Macros */
#define ntohll(netlonglong)	(htonll(netlonglong))
#ifdef __clang__
#define htonll(hostlonglong) 	(*(unsigned char *) (unsigned int [1]) { 1 }) ? ((uint64_t) htonl(((uint32_t *) &((uint64_t [1]) { (hostlonglong) }[0]))[0]) << 32) | htonl(((uint32_t *) &((uint64_t [1]) { (hostlonglong) }[0]))[1]) : (hostlonglong)
#else
static inline uint64_t htonll(uint64_t hostlonglong) {
	uint32_t *ptr = (uint32_t *) &hostlonglong;
	uint32_t hi = 0, lo = 0;

	if (*(unsigned char *) (unsigned int [1]) { 1 }) {
		memcpy(&lo, (uint32_t [1]) { htonl(ptr[1]) }, 4);
		memcpy(&hi, (uint32_t [1]) { htonl(ptr[0]) }, 4);
		memcpy(ptr, &lo, 4);
		memcpy(&ptr[1], &hi, 4);
	}

	return hostlonglong;
}
#endif

/* Prototypes */
int conn_client_init(void);
int conn_client_process(void);
void conn_client_destroy(void);
#if CONFIG_CLIENT_ONLY == 0
int conn_daemon_init(void);
int conn_daemon_process_all(void);
void conn_daemon_client_close(int fd);
void conn_daemon_destroy(void);
#endif /* CONFIG_CLIENT_ONLY == 0 */
int conn_is_local(int fd);
int conn_is_remote(int fd);
int conn_set_nonblock(int fd);

#endif

