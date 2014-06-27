/**
 * @file conn.h
 * @brief uSched
 *        Connections interface header
 *
 * Date: 27-06-2014
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


#ifndef USCHED_CONN_H
#define USCHED_CONN_H

#include <arpa/inet.h>

/* Macros */
#define htonll(val) 	(*(unsigned char *) (unsigned int [1]) { 1 }) ? ((uint64_t) htonl(((uint32_t *) &((uint64_t [1]) { (val) }[0]))[0]) << 32) | htonl(((uint32_t *) &((uint64_t [1]) { (val) }[0]))[1]) : (val)
#define ntohll(val)	(htonll(val))

/* Prototypes */
int conn_client_init(void);
int conn_client_process(void);
void conn_client_destroy(void);
int conn_daemon_init(void);
void conn_daemon_process(void);
void conn_daemon_destroy(void);
int conn_is_local(int fd);
int conn_is_remote(int fd);

#endif

