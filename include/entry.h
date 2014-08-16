/**
 * @file entry.h
 * @brief uSched
 *        Entry handling interface header
 *
 * Date: 16-08-2014
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


#ifndef USCHED_ENTRY_H
#define USCHED_ENTRY_H

#include <stddef.h>
#include <time.h>

#include <sys/types.h>

#include <psched/sched.h>

#include <psec/crypt.h>

#include "config.h"
#include "usched.h"

/* Entry flags */
typedef enum USCHED_ENTRY_FLAGS {
	/* Remote flags - Allowed to be handled by client */
	USCHED_ENTRY_FLAG_NEW = 1,
	USCHED_ENTRY_FLAG_DEL,
	USCHED_ENTRY_FLAG_GET,

	/* Local flags - Allowed to be handled by daemon */
	USCHED_ENTRY_FLAG_INIT,
	USCHED_ENTRY_FLAG_PROGRESS,
	USCHED_ENTRY_FLAG_AUTHORIZED,
	USCHED_ENTRY_FLAG_FINISH,
	USCHED_ENTRY_FLAG_COMPLETE
} usched_entry_flag_t;

/* uSched Entry Structure */
#define usched_entry_id(id) 	((struct usched_entry [1]) { { id, } })
#define usched_entry_hdr_size()	(offsetof(struct usched_entry, payload))
#pragma pack(push)
#pragma pack(4)
struct usched_entry {
	/* Entry request header */
	uint64_t id;
	uint32_t flags;
	uint32_t uid;
	uint32_t gid;
	uint32_t trigger;
	uint32_t step;
	uint32_t expire;
	uint32_t psize;		/* Payload size */

	/* Authentication Header */
	char username[CONFIG_USCHED_AUTH_USERNAME_MAX];
	char session[CONFIG_USCHED_AUTH_SESSION_MAX];

	/* Entry payload */
	char *payload;

	/* Entry properties */
	uint32_t subj_size;
	char *subj;

	/* Reserved */
	pschedid_t psched_id;	/* The libpsched entry identifier */

	/* Cryptographic Data Context */
	unsigned char token[CRYPT_KEY_SIZE_XSALSA20];
	unsigned char nonce[CRYPT_NONCE_SIZE_XSALSA20];

	unsigned char remote_key_pub[CONFIG_USCHED_SEC_PUBKEY_SIZE];
	unsigned char key_shr[CONFIG_USCHED_SEC_PUBKEY_SIZE];
};
#pragma pack(pop)

/* Prototypes */
struct usched_entry *entry_client_init(uid_t uid, gid_t gid, time_t trigger, void *payload, size_t psize);
int entry_client_remote_session_create(struct usched_entry *entry);
int entry_client_remote_session_process(struct usched_entry *entry, const char *password);
int entry_client_payload_encrypt(struct usched_entry *entry);
void entry_set_id(struct usched_entry *entry, uint32_t id);
void entry_set_flags(struct usched_entry *entry, uint32_t flags);
void entry_unset_flags_local(struct usched_entry *entry);
int entry_has_flag(struct usched_entry *entry, usched_entry_flag_t flag);
void entry_set_flag(struct usched_entry *entry, usched_entry_flag_t flag);
void entry_unset_flag(struct usched_entry *entry, usched_entry_flag_t flag);
void entry_set_uid(struct usched_entry *entry, uid_t uid);
void entry_set_gid(struct usched_entry *entry, gid_t gid);
void entry_set_trigger(struct usched_entry *entry, time_t trigger);
void entry_set_step(struct usched_entry *entry, time_t step);
void entry_set_expire(struct usched_entry *entry, time_t expire);
void entry_set_psize(struct usched_entry *entry, size_t size);
void entry_set_subj_size(struct usched_entry *entry, size_t size);
int entry_set_payload(struct usched_entry *entry, const char *payload, size_t len);
void entry_unset_payload(struct usched_entry *entry);
int entry_set_subj(struct usched_entry *entry, const char *subj, size_t len);
int entry_copy(struct usched_entry *dest, struct usched_entry *src);
int entry_compare(const void *e1, const void *e2);
int entry_daemon_authorize(struct usched_entry *entry, int fd);
int entry_daemon_remote_session_create(struct usched_entry *entry);
int entry_daemon_remote_session_process(struct usched_entry *entry);
int entry_daemon_payload_decrypt(struct usched_entry *entry);
void entry_daemon_pmq_dispatch(void *arg);
void entry_destroy(void *elem);
int entry_daemon_serialize(pall_fd_t fd, void *entry);
void *entry_daemon_unserialize(pall_fd_t fd);

#endif

