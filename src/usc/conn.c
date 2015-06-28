/**
 * @file conn.c
 * @brief uSched
 *        Connections interface - Client
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
#endif

#include <panet/panet.h>

#include <psec/crypt.h>

#include "debug.h"
#include "mm.h"
#include "runtime.h"
#include "conn.h"
#include "log.h"
#include "print.h"
#include "process.h"


int conn_client_init(void) {
	int errsv = 0;

	if (runc.opt.remote_hostname[0]) {
		if ((runc.fd = panet_client_ipv4(runc.opt.remote_hostname, runc.opt.remote_port, PANET_PROTO_TCP, 5)) == (sock_t) -1) {
			errsv = errno;
			log_crit("conn_client_init(): panet_client_ipv4(): %s\n", strerror(errno));
			errno = errsv;
			return -1;
		}
#ifndef COMPILE_WIN32
	} else if ((runc.fd = panet_client_unix(runc.config.network.sock_name, PANET_PROTO_UNIX_STREAM)) < 0) {
		errsv = errno;
		log_crit("conn_client_init(): panet_client_unix(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
#endif
	}
#ifdef COMPILE_WIN32
	else {
		errno = EINVAL;
		return -1;
	}
#endif

	return 0;
}

int conn_client_process(void) {
	int errsv = 0, ret = 0;
	struct usched_entry *cur = NULL;
	char *aaa_payload_data = NULL;

	while ((cur = runc.epool->pop(runc.epool))) {
		/* If this is a remote connection, the payload will be encrypted, so we need to
		 * inform the remote party of size of the encrypted payload and not the current
		 * (plain) size.
		 */
		if (conn_is_remote(runc.fd)) {
			/* TODO or FIXME: If we're going to encrypt this entry, why not do it right
			 * here instead of doing lots of calculations on entry->psize along this
			 * rountine?
			 */
			cur->psize += CRYPT_EXTRA_SIZE_CHACHA20POLY1305;
		}

		/* Convert endianness to network byte order */
		cur->id = htonll(cur->id);
		cur->flags = htonl(cur->flags);

		if (conn_is_remote(runc.fd)) {
			/* For remote connections, UID and GID must be set to 0xff */
			cur->uid = htonl(0xff);
			cur->gid = htonl(0xff);
		} else {
			cur->uid = htonl(cur->uid);
			cur->gid = htonl(cur->gid);
		}

		cur->trigger = htonl(cur->trigger);
		cur->step = htonl(cur->step);
		cur->expire = htonl(cur->expire);
		/* We can ignore pid, status, exec_time, latency, outdata_len and outdata here */
		cur->psize = htonl(cur->psize);

		/* Set username and session data if this is a remote connection */
		if (conn_is_remote(runc.fd)) {
			/* Set username */
			if (strlen(runc.opt.remote_username) >= sizeof(cur->username)) {
				log_crit("conn_client_process(): The requested username is too long to be processed: %s\n", runc.opt.remote_username);
				/* The payload size is in network byte order. We need to revert it
				 * before calling entry_destroy().
				 */
				cur->psize = ntohl(cur->psize) - CRYPT_EXTRA_SIZE_CHACHA20POLY1305;
				entry_destroy(cur);
				errno = EINVAL;
				return -1;
			}

			strcpy(cur->username, runc.opt.remote_username);

			/* Set the session data */
			if (entry_client_remote_session_create(cur, runc.opt.remote_password) < 0) {
				errsv = errno;
				log_crit("conn_client_process(): entry_client_remote_session_create(): %s\n", strerror(errno));
				/* The payload size is in network byte order. We need to revert it
				 * before calling entry_destroy().
				 */
				cur->psize = ntohl(cur->psize) - CRYPT_EXTRA_SIZE_CHACHA20POLY1305;
				entry_destroy(cur);
				errno = errsv;
				return -1;
			}
		}

		/* Send the first entry block */
		if (conn_write_blocking(runc.fd, cur, usched_entry_hdr_size()) != (ssize_t) usched_entry_hdr_size()) {
			errsv = errno;
			log_crit("conn_client_process(): conn_write_blocking() != %d: %s\n", usched_entry_hdr_size(), strerror(errno));
			/* The payload size is in network byte order. We need to revert it before
			 * calling entry_destroy().
			 */
			cur->psize = ntohl(cur->psize) - (conn_is_remote(runc.fd) ? CRYPT_EXTRA_SIZE_CHACHA20POLY1305 : 0);
			entry_destroy(cur);
			errno = errsv;
			return -1;
		}

		/* Convert endianness to host byte order */
		cur->id = ntohll(cur->id);
		cur->flags = ntohl(cur->flags);
		cur->uid = ntohl(cur->uid);
		cur->gid = ntohl(cur->gid);
		cur->trigger = ntohl(cur->trigger);
		cur->step = ntohl(cur->step);
		cur->expire = ntohl(cur->expire);
		cur->psize = ntohl(cur->psize) - (conn_is_remote(runc.fd) ? CRYPT_EXTRA_SIZE_CHACHA20POLY1305 : 0); /* Set the original payload size if the connection is remote. */

		/* Read the session token into the session field for further processing */
		if (conn_read_blocking(runc.fd, cur->session, sizeof(cur->session)) != (ssize_t) sizeof(cur->session)) {
			errsv = errno;
			log_crit("conn_client_process(): conn_read_blocking() != sizeof(cur->session): %s\n", strerror(errno));
			entry_destroy(cur);
			errno = errsv;
			return -1;
		}

		/* If this a remote connection, read the session field, which contains session data,
		 * and rewrite it with authentication information.
		 */
		if (conn_is_remote(runc.fd)) {
			/* Process remote session data */
			if (entry_client_remote_session_process(cur, runc.opt.remote_password) < 0) {
				errsv = errno;
				log_crit("conn_client_process(): entry_client_remote_session_process(): %s\n", strerror(errno));
				entry_destroy(cur);
				errno = errsv;
				return -1;
			}

			/* Encrypt payload */
			if (entry_payload_encrypt(cur, 0) < 0) {
				errsv = errno;	
				log_crit("conn_client_process(): entry_payload_encrypt(): %s\n", strerror(errno));
				entry_destroy(cur);
				errno = errsv;
				return -1;
			}
		}

		/* Craft the token and payload together */
		if (!(aaa_payload_data = mm_alloc(sizeof(cur->session) + cur->psize))) {
			errsv = errno;
			log_crit("conn_client_process(): mm_alloc(): %s\n", strerror(errno));
			entry_destroy(cur);
			errno = errsv;
			return -1;
		}

		/* Craft the session/authentication information along with the payload */
		memcpy(aaa_payload_data, cur->session, sizeof(cur->session));
		memcpy(aaa_payload_data + sizeof(cur->session), cur->payload, cur->psize);

		/* Send the authentication and authorization data along entry payload */
		if (conn_write_blocking(runc.fd, aaa_payload_data, sizeof(cur->session) + cur->psize) != (ssize_t) (sizeof(cur->session) + cur->psize)) {
			errsv = errno;
			log_crit("conn_client_process(): conn_write_blocking() != (sizeof(cur->session) + cur->psize): %s\n", strerror(errno));
			entry_destroy(cur);
			mm_free(aaa_payload_data);
			errno = errsv;
			return -1;
		}

		/* Reset and free aaa_payload_data */
		memset(aaa_payload_data, 0, sizeof(cur->session) + cur->psize);
		mm_free(aaa_payload_data);

		/* Process the response */
		if (entry_has_flag(cur, USCHED_ENTRY_FLAG_NEW)) {
			ret = process_client_recv_run(cur);
		} else if (entry_has_flag(cur, USCHED_ENTRY_FLAG_DEL)) {
			ret = process_client_recv_stop(cur);
		} else if (entry_has_flag(cur, USCHED_ENTRY_FLAG_GET)) {
			ret = process_client_recv_show(cur);
		} else {
			log_warn("conn_client_process(): Unexpected value found in entry->flags\n");
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

