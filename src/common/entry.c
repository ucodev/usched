/**
 * @file entry.c
 * @brief uSched
 *        Entry handling interface - Common
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
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#include "config.h"

#include <sys/types.h>

#include <psec/crypt.h>
#include <psec/hash/low.h>

#include "debug.h"
#include "mm.h"
#include "entry.h"
#include "bitops.h"
#include "log.h"
#include "conn.h"
#include "str.h"

void entry_cleanup_session(struct usched_entry *entry) {
	memset(entry->session, 0, sizeof(entry->session));
}

void entry_cleanup_crypto(struct usched_entry *entry) {
	memset(&entry->crypto, 0, sizeof(entry->crypto));
}

void entry_update_signature(struct usched_entry *entry) {
	psec_low_hash_t context;

	hash_low_blake2s_init(&context);

	hash_low_blake2s_update(&context, (unsigned char *) &entry->id, sizeof(entry->id));
	hash_low_blake2s_update(&context, (unsigned char *) &entry->uid, sizeof(entry->uid));
	hash_low_blake2s_update(&context, (unsigned char *) &entry->gid, sizeof(entry->gid));
	hash_low_blake2s_update(&context, (unsigned char *) entry->subj, entry->subj_size);
	hash_low_blake2s_update(&context, (unsigned char *) &entry->create_time, sizeof(entry->create_time));

	hash_low_blake2s_final(&context, entry->signature);
}

int entry_check_signature(struct usched_entry *entry) {
	psec_low_hash_t context;
	unsigned char signature[HASH_DIGEST_SIZE_BLAKE2S];

	hash_low_blake2s_init(&context);

	hash_low_blake2s_update(&context, (unsigned char *) &entry->id, sizeof(entry->id));
	hash_low_blake2s_update(&context, (unsigned char *) &entry->uid, sizeof(entry->uid));
	hash_low_blake2s_update(&context, (unsigned char *) &entry->gid, sizeof(entry->gid));
	hash_low_blake2s_update(&context, (unsigned char *) entry->subj, entry->subj_size);
	hash_low_blake2s_update(&context, (unsigned char *) &entry->create_time, sizeof(entry->create_time));

	hash_low_blake2s_final(&context, signature);

	return !memcmp(entry->signature, signature, sizeof(signature));
}

void entry_set_id(struct usched_entry *entry, uint32_t id) {
	entry->id = id;
}

void entry_set_flags(struct usched_entry *entry, uint32_t flags) {
	entry->flags = flags;
}

void entry_unset_flags_local(struct usched_entry *entry) {
	unsigned int n = 0;

	/* Clear all local flags */
	for (n = USCHED_ENTRY_FLAG_INIT; n < (unsigned int) (sizeof(entry->flags) - 1); n ++)
		entry_unset_flag(entry, n);
}

unsigned int entry_has_flag(const struct usched_entry *entry, usched_entry_flag_t flag) {
	return bit_test(&entry->flags, flag);
}

void entry_set_flag(struct usched_entry *entry, usched_entry_flag_t flag) {
	bit_set(&entry->flags, flag);
}

void entry_unset_flag(struct usched_entry *entry, usched_entry_flag_t flag) {
	bit_clear(&entry->flags, flag);
}

void entry_set_uid(struct usched_entry *entry, uid_t uid) {
	entry->uid = uid;
}

void entry_set_gid(struct usched_entry *entry, gid_t gid) {
	entry->gid = gid;
}

void entry_set_trigger(struct usched_entry *entry, time_t trigger) {
	entry->trigger = (uint32_t) trigger;
}

void entry_set_step(struct usched_entry *entry, time_t step) {
	entry->step = (uint32_t) step;
}

void entry_set_expire(struct usched_entry *entry, time_t expire) {
	entry->expire = (uint32_t) expire;
}

void entry_set_psize(struct usched_entry *entry, size_t size) {
	entry->psize = (uint32_t) size;
}

void entry_set_subj_size(struct usched_entry *entry, size_t size) {
	entry->subj_size = (uint32_t) size;
}

int entry_set_payload(struct usched_entry *entry, const char *payload, size_t len) {
	int errsv = 0;

	if (!(entry->payload = mm_alloc(len + 1))) {
		errsv = errno;
		log_warn("entry_set_payload(): mm_alloc(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Since payload sometimes may carry NULL terminated strings, lets play safe and always
	 * force a NULL termination. Even if it isn't required... it won't hurt.
	 */
	memset(entry->payload, 0, len + 1);

	memcpy(entry->payload, payload, len);

	entry_set_psize(entry, len);

	return 0;
}

int entry_payload_decrypt(struct usched_entry *entry) {
	int errsv = 0;
	unsigned char *payload_dec = NULL;
	size_t out_len = 0;

	debug_printf(DEBUG_INFO, "entry_payload_decrypt(): Decrypting...\n");

	/* Alloc memory for decrypted payload */
	if (!(payload_dec = mm_alloc(entry->psize - CRYPT_EXTRA_SIZE_CHACHA20POLY1305))) {
		errsv = errno;
		log_warn("entry_payload_decrypt(): mm_alloc(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Increment nonce */
	entry->crypto.nonce ++;

	/* Decrypt payload */
	if (!(crypt_decrypt_chacha20poly1305(payload_dec, &out_len, (unsigned char *) entry->payload, entry->psize, (unsigned char *) (uint64_t [1]) { htonll(entry->crypto.nonce) }, entry->crypto.agreed_key))) {
		errsv = errno;
		log_warn("entry_payload_decrypt(): crypt_decrypt_chacha20poly1305(): %s\n", strerror(errno));
		mm_free(payload_dec);
		errno = errsv;
		return -1;
	}

	/* Set payload size */
	entry->psize = out_len;

	/* Free encrypted payload */
	mm_free(entry->payload);

	/* Set the new payload */
	entry->payload = (char *) payload_dec;

	/* All good */
	return 0;
}


int entry_payload_encrypt(struct usched_entry *entry, size_t lpad) {
	int errsv = 0;
	unsigned char *payload_enc = NULL;
	size_t out_len = 0;

	/* Alloc memory for encrypted payload */
	if (!(payload_enc = mm_alloc(lpad + entry->psize + CRYPT_EXTRA_SIZE_CHACHA20POLY1305))) {
		errsv = errno;
		log_warn("entry_payload_encrypt(): mm_alloc(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Increment nonce */
	entry->crypto.nonce ++;

	/* Encrypt payload */
	if (!(crypt_encrypt_chacha20poly1305(payload_enc + lpad, &out_len, (unsigned char *) entry->payload, entry->psize, (unsigned char *) (uint64_t [1]) { htonll(entry->crypto.nonce) }, entry->crypto.agreed_key))) {
		errsv = errno;
		log_warn("entry_payload_encrypt(): crypt_encrypt_chacha20poly1305(): %s\n", strerror(errno));
		mm_free(payload_enc);
		errno = errsv;
		return -1;
	}

	/* Set the psize */
	entry->psize = out_len + lpad;

	/* Free plaintext payload */
	mm_free(entry->payload);

	/* Set new payload */
	entry->payload = (char *) payload_enc;

	/* All good */
	return 0;
}

void entry_unset_payload(struct usched_entry *entry) {
	if (entry->payload) {
		memset(entry->payload, 0, entry->psize);
		mm_free(entry->payload);
		entry->payload = NULL;
	}

	entry->psize = 0;
}

int entry_set_subj(struct usched_entry *entry, const char *subj, size_t len) {
	int errsv = 0;

	/* WARNING: 
	 *
	 * If length isn't specified, calculate it through strlen(). Note that this isn't
	 * recommended, since we have no idea from where the subject pointer is comming from.
	 * Although the payload is forced to be NULL terminated when set, it doesn't mean
	 * that we can assume that the *subj pointer is always NULL terminated.
	 *
	 * That said, we should issue a warning if it happens, in order to be corrected ASAP.
	 *
	 */
	if (!len) {
		len = strlen(subj) + 1;
		log_warn("entry_set_subj(): Subject length wasn't specified. This is not recommended. Computed subject size through strlen() is: %zu bytes.\n", len);
	}

	/* Validate if subject only contains ascii characters */
	if (!strisascii(subj, len)) {
		log_warn("entry_set_subj(): The specified subject contains non-ascii characters. This is now allowed.\n");
		errno = EINVAL;
		return -1;
	}

	/* Allocate subject memory */
	if (!(entry->subj = mm_alloc(len + 1))) {
		errsv = errno;
		log_warn("entry_set_subj(): mm_alloc(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	memset(entry->subj, 0, len + 1);

	memcpy(entry->subj, subj, len);

	/* Set subject size */
	entry_set_subj_size(entry, len);

	/* All good */
	return 0;
}

void entry_unset_subj(struct usched_entry *entry) {
	if (entry->subj) {
		memset(entry->subj, 0, strlen(entry->subj));
		mm_free(entry->subj);
		entry->subj = NULL;
	}
}

int entry_copy(struct usched_entry *dest, struct usched_entry *src) {
	int errsv = 0;

	memcpy(dest, src, sizeof(struct usched_entry));

	if (src->subj && src->subj_size) {
		if (entry_set_subj(dest, src->subj, src->subj_size) < 0) {
			errsv = errno;
			log_warn("entry_copy(): entry_set_subj(): %s\n", strerror(errno));
			errno = errsv;
			return -1;
		}
	}

	if (src->payload && src->psize) {
		if (entry_set_payload(dest, src->payload, src->psize) < 0) {
			errsv = errno;
			log_warn("entry_copy(): entry_set_payload(): %s\n", strerror(errno));
			mm_free(dest->subj);
			errno = errsv;
			return -1;
		}
	}

	return 0;
}

int entry_compare(const void *e1, const void *e2) {
	const struct usched_entry *pe1 = (struct usched_entry *) e1, *pe2 = (struct usched_entry *) e2;

	if (pe1->id > pe2->id)
		return 1;

	if (pe1->id < pe2->id)
		return -1;

	return 0;
}

void entry_zero(struct usched_entry *entry) {
	memset(entry, 0, sizeof(struct usched_entry));
}

void entry_destroy(void *elem) {
	struct usched_entry *entry = elem;

	entry_unset_payload(entry);
	entry_unset_subj(entry);
	entry_zero(entry);

	mm_free(entry);
}

