/**
 * @file conn_client.c
 * @brief uSched
 *        Connections interface - Client
 *
 * Date: 28-07-2014
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

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <rtsaio/rtsaio.h>
#include <panet/panet.h>

#include "debug.h"
#include "config.h"
#include "mm.h"
#include "runtime.h"
#include "notify.h"
#include "conn.h"
#include "log.h"
#include "print.h"

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

static int _conn_client_process_recv_run(void) {
	int errsv = 0;
	uint64_t entry_id = 0;

	/* Read the response in order to obtain the entry id */
	if (read(runc.fd, &entry_id, sizeof(entry_id)) != sizeof(entry_id)) {
		errsv = errno;
		log_crit("conn_client_process_recv_run(): read() != %zu: %s\n", sizeof(entry_id), strerror(errno));
		errno = errsv;
		return -1;
	}

	entry_id = ntohll(entry_id);

	debug_printf(DEBUG_INFO, "Received Entry ID: 0x%llX\n", entry_id);

	print_result_run(entry_id);

	return 0;
}

static int _conn_client_process_recv_stop(void) {
	int i = 0, errsv = 0;
	uint32_t entry_list_nmemb = 0;
	uint64_t *entry_list = NULL;

	/* Read te number of elements to receive */
	if (read(runc.fd, &entry_list_nmemb, sizeof(entry_list_nmemb)) != sizeof(entry_list_nmemb)) {
		errsv = errno;
		log_crit("conn_client_process_recv_stop(): read() != %zu: %s\n", sizeof(entry_list_nmemb), strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Network to Host byte order */
	entry_list_nmemb = ntohl(entry_list_nmemb);

	if (!entry_list_nmemb) {
		log_info("conn_client_process_recv_stop(): No entries were deleted.\n");
		return 0;
	}

	/* Alloc sufficient memory to receive the deleted entries list */
	if (!(entry_list = mm_alloc(entry_list_nmemb * sizeof(uint64_t)))) {
		errsv = errno;
		log_crit("conn_client_process_recv_stop(): mm_alloc(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Receive the deleted entries list */
	if (read(runc.fd, entry_list, entry_list_nmemb * sizeof(uint64_t)) != (entry_list_nmemb * sizeof(uint64_t))) {
		errsv = errno;
		log_crit("conn_client_process_recv_stop(): read() != %zu: %s\n", entry_list_nmemb * sizeof(uint64_t), strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Iterate the received entries list */
	for (i = 0; i < entry_list_nmemb; i ++) {
		/* Network to Host byte order */
		entry_list[i] = ntohll(entry_list[i]);

		debug_printf(DEBUG_INFO, "Entry ID 0x%llX was deleted\n", entry_list[i]);
	}

	/* Print the deleted entries */
	print_result_del(entry_list, entry_list_nmemb);

	/* Free entry_list memory */
	mm_free(entry_list);

	return 0;
}

static int _conn_client_process_recv_show(void) {
	int i = 0, errsv = 0, ret = -1;
	uint32_t entry_list_nmemb = 0;
	struct usched_entry *entry_list = NULL;

	/* Read te number of elements to receive */
	if (read(runc.fd, &entry_list_nmemb, sizeof(entry_list_nmemb)) != sizeof(entry_list_nmemb)) {
		errsv = errno;
		log_crit("conn_client_process_recv_show(): read() != %zu: %s\n", sizeof(entry_list_nmemb), strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Network to Host byte order */
	entry_list_nmemb = ntohl(entry_list_nmemb);

	if (!entry_list_nmemb) {
		log_info("conn_client_process_recv_show(): No entries were found.\n");
		return 0;
	}

	/* Alloc the entries array */
	if (!(entry_list = mm_alloc(entry_list_nmemb * sizeof(struct usched_entry)))) {
		errsv = errno;
		log_crit("conn_client_process_recv_show(): mm_alloc(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Reset array memory */
	memset(entry_list, 0, entry_list_nmemb * sizeof(struct usched_entry));

	/* Receive the entries */
	for (i = 0; i < entry_list_nmemb; i ++) {
		/* Read the first block of the entry */
		if (read(runc.fd, &entry_list[i], offsetof(struct usched_entry, psize)) != offsetof(struct usched_entry, psize)) {
			errsv = errno;
			log_crit("conn_client_process_recv_show(): read() != %zu: %s\n", offsetof(struct usched_entry, psize), strerror(errno));
			goto _recv_show_finish;
		}

		/* Convert Network to Host byte order */
		entry_list[i].id = ntohll(entry_list[i].id);
		entry_list[i].flags = ntohl(entry_list[i].flags);
		entry_list[i].uid = ntohl(entry_list[i].uid);
		entry_list[i].gid = ntohl(entry_list[i].gid);
		entry_list[i].trigger = ntohl(entry_list[i].trigger);
		entry_list[i].step = ntohl(entry_list[i].step);
		entry_list[i].expire = ntohl(entry_list[i].expire);

		/* Read the entry username */
		if (read(runc.fd, entry_list[i].username, CONFIG_USCHED_AUTH_USERNAME_MAX) != CONFIG_USCHED_AUTH_USERNAME_MAX) {
			errsv = errno;
			log_crit("conn_client_process_recv_show(): read() != %zu: %s\n", CONFIG_USCHED_AUTH_USERNAME_MAX, strerror(errno));
			goto _recv_show_finish;
		}

		/* Read the subject size */
		if (read(runc.fd, &entry_list[i].subj_size, 4) != 4) {
			errsv = errno;
			log_crit("conn_client_process_recv_show(): read() != 4: %s\n", strerror(errno));
			goto _recv_show_finish;
		}

		/* Convert Network to Host byte order */
		entry_list[i].subj_size = ntohl(entry_list[i].subj_size);

		/* Allocate the subject memory */
		if (!(entry_list[i].subj = mm_alloc(entry_list[i].subj_size + 1))) {
			errsv = errno;
			log_crit("conn_client_process_recv_show(): mm_alloc(%d): %s\n", entry_list[i].subj_size + 1, strerror(errno));
			goto _recv_show_finish;
		}

		/* Reset subject memory */
		memset(entry_list[i].subj, 0, entry_list[i].subj_size + 1);

		/* Read the subject contents */
		if (read(runc.fd, entry_list[i].subj, entry_list[i].subj_size + 1) != (entry_list[i].subj_size + 1)) {
			errsv = errno;
			log_crit("conn_client_process_recv_show(): read() != %d: %s\n", entry_list[i].subj_size + 1, strerror(errno));
			goto _recv_show_finish;
		}
	}

	/* Print the received entries */
	print_result_show(entry_list, entry_list_nmemb);

	ret = 0;
	i --;

_recv_show_finish:
	for (; i >= 0; i --) {
		if (entry_list[i].subj)
			mm_free(entry_list[i].subj);
	}

	mm_free(entry_list);

	errno = errsv;

	return ret;
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
			ret = _conn_client_process_recv_run();
		} else if (entry_has_flag(cur, USCHED_ENTRY_FLAG_DEL)) {
			ret = _conn_client_process_recv_stop();
		} else if (entry_has_flag(cur, USCHED_ENTRY_FLAG_GET)) {
			ret = _conn_client_process_recv_show();
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

