/**
 * @file entry.h
 * @brief uSched
 *        Entry handling interface header
 *
 * Date: 13-07-2015
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


#ifndef USCHED_ENTRY_H
#define USCHED_ENTRY_H

#include <stddef.h>
#include <time.h>

#include <sys/types.h>

#include "config.h"

#if CONFIG_CLIENT_ONLY == 0
#include <psched/psched.h>
#endif /* CONFIG_CLIENT_ONLY == 0 */

#include <psec/crypt.h>
#include <psec/ke.h>
#include <psec/hash.h>

#include <panet/panet.h>

#include "usched.h"

/* Entry flags */
typedef enum USCHED_ENTRY_FLAGS {
	/* Remote flags - Allowed to be handled by client */
	USCHED_ENTRY_FLAG_NEW = 1,	/* Install a new entry */
	USCHED_ENTRY_FLAG_DEL,		/* Remove an existing entry */
	USCHED_ENTRY_FLAG_GET,		/* Fetch an existing entry */
	USCHED_ENTRY_FLAG_PAUSE,	/* Pause a running entry */

	/* Scheduling flags (remote) - Allowed to be handled by client */
	USCHED_ENTRY_FLAG_MONTHDAY_ALIGN,	/* Entry step must be aligned to month day */
	USCHED_ENTRY_FLAG_YEARDAY_ALIGN,	/* Entry step must be aligned to year day */

	USCHED_ENTRY_FLAG_RELATIVE_TRIGGER,	/* Trigger time is relative to it's creation date */
	USCHED_ENTRY_FLAG_RELATIVE_EXPIRE,	/* Expire time is relative to it's creation date */

	/* Local flags - Only allowed to be handled by daemon */
	USCHED_ENTRY_FLAG_INIT,		/* Entry state set to initialized */
	USCHED_ENTRY_FLAG_PROGRESS,	/* Entry is being processed */
	USCHED_ENTRY_FLAG_AUTHORIZED,	/* Entry is authorized */
	USCHED_ENTRY_FLAG_FINISH,	/* Cleanup routines are being executed */
	USCHED_ENTRY_FLAG_COMPLETE,	/* Entry is now completely processed */
	USCHED_ENTRY_FLAG_TRIGGERED,	/* Entry was triggered at least once by the scheduler */
	USCHED_ENTRY_FLAG_SERIALIZED,	/* Entry is serialized */
	USCHED_ENTRY_FLAG_INVALID,	/* Entry is in an invalid state */
	USCHED_ENTRY_FLAG_EXPIRED,	/* TODO: Entry is expired (entry.c:367) */
	USCHED_ENTRY_FLAG_PAUSED,	/* TODO: Entry is paused */
	USCHED_ENTRY_FLAG_REMOVED	/* Entry was marked to be removed */
} usched_entry_flag_t;

/* uSched Entry Structure */
#ifndef USCHED_NO_PRAGMA_PACK
 #pragma pack(push)
 #pragma pack(4)
#endif
union
#ifdef USCHED_NO_PRAGMA_PACK
__attribute__ ((packed, aligned(4)))
#endif
usched_entry_reserved {
#if CONFIG_CLIENT_ONLY == 0
	pschedid_t psched_id;		/* The libpsched entry identifier */
#endif
	unsigned char _reserved[32];
};
#ifndef USCHED_NO_PRAGMA_PACK
 #pragma pack(pop)
#endif
#ifndef USCHED_NO_PRAGMA_PACK
 #pragma pack(push)
 #pragma pack(4)
#endif
struct
#ifdef USCHED_NO_PRAGMA_PACK
__attribute__ ((packed, aligned(4)))
#endif
usched_entry_crypto {
	unsigned char context[KE_CONTEXT_SIZE_CHREKE];
	unsigned char agreed_key[KE_KEY_SIZE_CHREKE];
	uint64_t nonce;
};
#ifndef USCHED_NO_PRAGMA_PACK
 #pragma pack(pop)
#endif
#define usched_entry_id(id) 	((struct usched_entry [1]) { { id, } })
#define usched_entry_hdr_size()	(offsetof(struct usched_entry, payload))

/**
 * @struct usched_entry
 *
 * @brief
 *   uSched scheduler entry structure.
 *
 * @see usched_result_get_show()
 *
 * @var usched_entry::id
 *   The Entry ID
 *
 * @var usched_entry::uid
 *   The UID value associated to the scheduler entry.
 *
 * @var usched_entry::gid
 *   The GID value associated to the scheduler entry.
 *
 * @var usched_entry::trigger
 *   The timestamp value for the next entry execution.
 *
 * @var usched_entry::step
 *   The value that will be added to the trigger value after each execution.
 *
 * @var usched_entry::expire
 *   The timestamp that once triggered will force the scheduler entry to be removed.
 *
 * @var usched_entry::username
 *   The username used for the remote authentication. Local authentications will have this field
 *   unset.
 *
 * @var usched_entry::subj
 *   The subject of the scheduler entry. This is the command-line value that will be executed at the
 *   next trigger value.
 *
 */
#ifndef USCHED_NO_PRAGMA_PACK
 #pragma pack(push)
 #pragma pack(4)
#endif
struct
#ifdef USCHED_NO_PRAGMA_PACK
__attribute__ ((packed, aligned(4)))
#endif
usched_entry {
	/* Entry request header */
	uint64_t id;
	uint32_t flags;
	uint32_t uid;
	uint32_t gid;
	uint32_t trigger;
	uint32_t step;
	uint32_t expire;
	uint32_t pid;
	uint32_t status;
	uint64_t exec_time;	/* In nanoseconds */
	uint64_t latency;	/* In nanoseconds */
	uint32_t outdata_len;
	char outdata[CONFIG_USCHED_EXEC_OUTPUT_MAX];
	uint32_t psize;		/* Payload size */

	/* Authentication Header */
	char username[CONFIG_USCHED_AUTH_USERNAME_MAX];
	unsigned char session[CONFIG_USCHED_AUTH_SESSION_MAX];

	/* Entry payload */
	char *payload;

	/* Entry properties */
	uint32_t subj_size;
	char *subj;

	/* Reserved */
	union usched_entry_reserved reserved;

	/* Cryptographic Data Context */
	struct usched_entry_crypto crypto;

	/* The time when this entry was created */
	uint32_t create_time;

	/*
	 * The entry signature is set after USCHED_ENTRY_FLAG_COMPLETE is set.
	 * The entry signature is the resulting hash of the following fields concatenation:
	 *
	 * blake2s(entry->id + entry->uid + entry->gid + entry->subj + entry->create_time)
	 *
	 * This signature is always verified:
	 *
	 * - Before serialization
	 * - After unserialization
	 * - Before delivering entries to the uSched Executer
	 *
	 * The purpose of the entry signature is to identify severe data corruption due
	 * to hardware/driver issues and/or possible bugs present on the uSched Services,
	 * preventing the execution of a corrupted command with corrupted UIDs or GIDs.
	 *
	 * Note that this offers _VERY LITTLE_ to _NO_ security enhancements if a bug is present in
	 * the code that allows an attacker to mangle the data structure of the entry. Depending on
	 * the severity, if the most of the structure can be re-written, the effectiveness of the
	 * signature is reduced down to zero.
	 *
	 */
	unsigned char signature[HASH_DIGEST_SIZE_BLAKE2S];
};
#ifndef USCHED_NO_PRAGMA_PACK
 #pragma pack(pop)
#endif

/* Prototypes */
struct usched_entry *entry_client_init(uid_t uid, gid_t gid, time_t trigger, void *payload, size_t psize);
int entry_client_remote_session_create(struct usched_entry *entry, const char *password);
int entry_client_remote_session_process(struct usched_entry *entry, const char *password);
void entry_cleanup_session(struct usched_entry *entry);
void entry_cleanup_crypto(struct usched_entry *entry);
void entry_update_signature(struct usched_entry *entry);
int entry_check_signature(struct usched_entry *entry);
void entry_set_id(struct usched_entry *entry, uint32_t id);
void entry_set_flags(struct usched_entry *entry, uint32_t flags);
void entry_unset_flags_local(struct usched_entry *entry);
unsigned int entry_has_flag(const struct usched_entry *entry, usched_entry_flag_t flag);
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
int entry_payload_decrypt(struct usched_entry *entry);
int entry_payload_encrypt(struct usched_entry *entry, size_t lpad);
void entry_unset_payload(struct usched_entry *entry);
int entry_set_subj(struct usched_entry *entry, const char *subj, size_t len);
void entry_unset_subj(struct usched_entry *entry);
int entry_copy(struct usched_entry *dest, struct usched_entry *src);
int entry_compare(const void *e1, const void *e2);
int entry_daemon_authorize(struct usched_entry *entry, sock_t fd);
int entry_daemon_remote_session_create(struct usched_entry *entry);
int entry_daemon_remote_session_process(struct usched_entry *entry);
void entry_daemon_exec_dispatch(void *arg);
void entry_zero(struct usched_entry *entry);
void entry_destroy(void *elem);
int entry_daemon_serialize(pall_fd_t fd, void *entry);
void *entry_daemon_unserialize(pall_fd_t fd);

#endif

