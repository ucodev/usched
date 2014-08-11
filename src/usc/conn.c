/**
 * @file conn.c
 * @brief uSched
 *        Connections interface - Client
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
#include "auth.h"


int conn_client_init(void) {
	int errsv = 0;

	if (runc.opt.remote_hostname[0]) {
		if ((runc.fd = panet_client_ipv4(runc.opt.remote_hostname, runc.opt.remote_port, PANET_PROTO_TCP, 5)) < 0) {
			errsv = errno;
			log_crit("conn_client_init(): panet_client_ipv4(): %s\n", strerror(errno));
			errsv = errno;
			return -1;
		}
	} else if ((runc.fd = panet_client_unix(runc.config.network.sock_named, PANET_PROTO_UNIX_STREAM)) < 0) {
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
	char *aaa_payload_data = NULL;

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

		/* Set username */
		strcpy(cur->username, runc.opt.remote_username);

		/* Send the first entry block */
		if (write(runc.fd, cur, usched_entry_hdr_size()) != usched_entry_hdr_size()) {
			errsv = errno;
			log_crit("conn_client_process(): write() != %d: %s\n", usched_entry_hdr_size(), strerror(errno));
			entry_destroy(cur);
			errno = errsv;
			return -1;
		}

		/* Read the session token into the password field for further processing */
		if (read(runc.fd, cur->password, sizeof(cur->password)) != sizeof(cur->password)) {
			errsv = errno;
			log_crit("conn_client_process(): read() != sizeof(cur->password): %s\n", strerror(errno));
			entry_destroy(cur);
			errno = errsv;
			return -1;
		}

		/* If this a remote connection, read the password field, which contains session data,
		 * and rewrite it with authentication information.
		 */
		if (conn_is_remote(runc.fd)) {
			if (auth_client_remote_user_token_process(cur->password, runc.opt.remote_password) < 0) {
				log_crit("conn_client_process(): auth_client_remote_user_token_process(): %s\n", strerror(errno));
				entry_destroy(cur);
				errno = errsv;
				return -1;
			}
		}

		/* Craft the token and payload together */
		if (!(aaa_payload_data = mm_alloc(sizeof(cur->password) + payload_len))) {
			errsv = errno;
			log_crit("conn_client_process(): malloc(): %s\n", strerror(errno));
			entry_destroy(cur);
			errno = errsv;
			return -1;
		}

		/* Craft the authentication information along with the payload */
		memcpy(aaa_payload_data, cur->password, sizeof(cur->password));
		memcpy(aaa_payload_data + sizeof(cur->password), cur->payload, payload_len);

		/* Send the authentication and authorization data along entry payload */
		if (write(runc.fd, aaa_payload_data, sizeof(cur->password) + payload_len) != (sizeof(cur->password) + payload_len)) {
			errsv = errno;
			log_crit("conn_client_process(): write() != (sizeof(cur->password) + payload_len): %s\n", strerror(errno));
			entry_destroy(cur);
			mm_free(aaa_payload_data);
			errno = errsv;
			return -1;
		}

		/* Reset and free aaa_payload_data */
		memset(aaa_payload_data, 0, sizeof(cur->password) + payload_len);
		mm_free(aaa_payload_data);

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

