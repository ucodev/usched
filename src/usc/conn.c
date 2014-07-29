/**
 * @file conn.c
 * @brief uSched
 *        Connections interface - Client
 *
 * Date: 30-07-2014
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

#include <panet/panet.h>

#include "debug.h"
#include "config.h"
#include "mm.h"
#include "runtime.h"
#include "conn.h"
#include "log.h"
#include "print.h"
#include "process.h"


int conn_client_init(void) {
	int errsv = 0;

	if ((runc.fd = panet_client_unix(CONFIG_USCHED_CONN_USER_NAMED_SOCKET, PANET_PROTO_UNIX_STREAM)) < 0) {
		errsv = errno;
		log_crit("conn_client_init(): panet_client_unix(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	return 0;
}

int conn_client_process(void) {
	int errsv = 0, ret = 0;
	struct usched_entry *cur = NULL;
	size_t payload_len = 0;

	while ((cur = runc.epool->pop(runc.epool))) {
		payload_len = cur->psize;

		/* Convert endianness to network byte order */
		cur->id = htonll(cur->id);
		cur->flags = htonl(cur->flags);
		cur->uid = htonl(cur->uid);
		cur->gid = htonl(cur->gid);
		cur->trigger = htonl(cur->trigger);
		cur->step = htonl(cur->step);
		cur->expire = htonl(cur->expire);
		cur->psize = htonl(cur->psize);

		if (write(runc.fd, cur, usched_entry_hdr_size()) != usched_entry_hdr_size()) {
			errsv = errno;
			log_crit("conn_client_process(): write() != %d: %s\n", usched_entry_hdr_size(), strerror(errno));
			entry_destroy(cur);
			errno = errsv;
			return -1;
		}

		if (write(runc.fd, cur->payload, payload_len) != payload_len) {
			errsv = errno;
			log_crit("conn_client_process(): write() != payload_len: %s\n", strerror(errno));
			entry_destroy(cur);
			errno = errsv;
			return -1;
		}

		/* Revert endianness back to host byte order */
		cur->id = ntohll(cur->id);
		cur->flags = ntohl(cur->flags);
		cur->uid = ntohl(cur->uid);
		cur->gid = ntohl(cur->gid);
		cur->trigger = ntohl(cur->trigger);
		cur->step = ntohl(cur->step);
		cur->expire = ntohl(cur->expire);
		cur->psize = ntohl(cur->psize);

		/* Process the response */
		if (entry_has_flag(cur, USCHED_ENTRY_FLAG_NEW)) {
			ret = process_client_recv_run();
		} else if (entry_has_flag(cur, USCHED_ENTRY_FLAG_DEL)) {
			ret = process_client_recv_stop();
		} else if (entry_has_flag(cur, USCHED_ENTRY_FLAG_GET)) {
			ret = process_client_recv_show();
		} else {
			errno = EINVAL;
			ret = -1;
		}

		/* Check if a valid response was received */
		if (ret < 0) {
			errsv = errno;
			log_crit("conn_client_process(): %s\n", strerror(errno));
			entry_destroy(cur);
			errno = errsv;
			return -1;
		}

		entry_destroy(cur);
	}

	return 0;
}

void conn_client_destroy(void) {
	panet_safe_close(runc.fd);
}

