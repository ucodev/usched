/**
 * @file entry.c
 * @brief uSched
 *        Entry handling interface - Daemon
 *
 * Date: 03-04-2015
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
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>

#include <sys/types.h>

#include <psec/crypt.h>

#include <pall/cll.h>

#include "debug.h"
#include "config.h"
#include "usched.h"
#include "runtime.h"
#include "mm.h"
#include "entry.h"
#include "log.h"
#include "auth.h"
#include "conn.h"
#include "schedule.h"
#include "vars.h"

#if CONFIG_USE_IPC_PMQ == 1
 #include <time.h>
 #include <mqueue.h>
#elif CONFIG_USE_IPC_UNIX == 1 || CONFIG_USE_IPC_INET == 1
 #include <panet/panet.h>
#endif

static int _entry_daemon_authorize_local(struct usched_entry *entry, sock_t fd) {
	int errsv = 0;

	if (auth_daemon_local(fd, &entry->uid, &entry->gid) < 0) {
		errsv = errno;
		log_warn("entry_daemon_authorize_local(): auth_daemon_local(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	return 1;
}

static int _entry_daemon_authorize_remote(struct usched_entry *entry, sock_t fd) {
	int errsv = 0;

	/* Validate session data and compare user password */
	if (entry_daemon_remote_session_process(entry) < 0) {
		errsv = errno;
		log_warn("_entry_daemon_authorize_remote(): entry_daemon_remote_session_process(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	return 1;
}

int entry_daemon_authorize(struct usched_entry *entry, sock_t fd) {
	int errsv = 0;
	int ret = -1;
	struct cll_handler *bl = NULL, *wl = NULL;

	if ((ret = conn_is_local(fd)) < 0) {
		errsv = errno;
		log_warn("entry_daemon_authorize(): conn_is_local(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	} else if (ret == 1) {
		if ((ret = _entry_daemon_authorize_local(entry, fd)) < 0) {
			errsv = errno;
			log_warn("entry_daemon_authorize(): entry_daemon_authorize_local(): %s\n", strerror(errno));
			errno = errsv;
			return -1;
		}

		/* ret == 1: Authorized, ret == 0: Not authorized (connection will timeout) */
	} else if ((ret = conn_is_remote(fd)) < 0) {
		errsv = errno;
		log_warn("entry_daemon_authorize(): conn_is_remote(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	} else if (ret == 1) {
		if ((ret = _entry_daemon_authorize_remote(entry, fd)) < 0) {
			errsv = errno;
			log_warn("entry_daemon_authorize(): entry_daemon_authorize_remote(): %s\n", strerror(errno));
			errno = errsv;
			return -1;
		}

		/* ret == 1: Authorized, ret == 0: Not yet authorized (More data expected) */
	} else {
		/* Not authorized. No authentication mechanism available */
		errno = ENOSYS;
		return -1;
	}

	/* If unauthorized here, just quit */
	if (ret <= 0) {
		errno = errsv;
		return ret;
	}

	/* Check if UID is whitelisted or blacklisted */
	bl = rund.config.auth.blacklist_uid;
	wl = rund.config.auth.whitelist_uid;

	/* Blacklists always take precedence over whitelists */
	if (bl->count(bl)) {
		if (bl->search(bl, (unsigned int [1]) { entry->uid }))
			ret = 0;
	} else if (wl->count(wl)) {
		if (!wl->search(wl, (unsigned int [1]) { entry->uid }))
			ret = 0;
	}

	/* Check if GID is whitelisted or blacklisted */
	bl = rund.config.auth.blacklist_gid;
	wl = rund.config.auth.whitelist_gid;

	/* Blacklists always take precedence over whitelists */
	if (bl->count(bl)) {
		if (bl->search(bl, (unsigned int [1]) { entry->gid }))
			ret = 0;
	} else if (wl->count(wl)) {
		if (!wl->search(wl, (unsigned int [1]) { entry->gid }))
			ret = 0;
	}

	/* Set/Unset Authorization flag */
	if (ret == 1) {
		entry_set_flag(entry, USCHED_ENTRY_FLAG_AUTHORIZED);
	} else {
		entry_unset_flag(entry, USCHED_ENTRY_FLAG_AUTHORIZED);
	}

	/* Return authentication status */
	return ret;
}

int entry_daemon_remote_session_create(struct usched_entry *entry) {
	int errsv = 0;

	/* Initialize a new entry->session field to be sent to the client */
	if (auth_daemon_remote_session_create(entry->username, entry->session, entry->crypto.context) < 0) {
		errsv = errno;
		log_warn("entry_daemon_remote_session_create(): auth_daemon_remote_session_create(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	return 0;
}

int entry_daemon_remote_session_process(struct usched_entry *entry) {
	int errsv = 0;

	/* Verify remote client authentication */
	if (auth_daemon_remote_session_verify(entry->username, entry->session, entry->crypto.context, entry->crypto.agreed_key, &entry->uid, &entry->gid) < 0) {
		errsv = errno;
		log_warn("entry_daemon_remote_session_process(): auth_daemon_remote_session_verify(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Set nonce to 0 */
	entry->crypto.nonce = 0;

	/* All good */
	return 0;
}

void entry_daemon_exec_dispatch(void *arg) {
	int ret = 0, errsv = 0;
	char *buf = NULL, *cmd = NULL;
	struct usched_entry *entry = arg;
#if CONFIG_USE_IPC_PMQ == 1
	struct timespec pmq_timeout = { CONFIG_USCHED_IPC_TIMEOUT, 0 };
#endif

	/* Remove relative trigger flags, if any */
	entry_unset_flag(entry, USCHED_ENTRY_FLAG_RELATIVE_TRIGGER);

	/* Mark this entry as triggered (initial trigger was reached at least once) */
	entry_set_flag(entry, USCHED_ENTRY_FLAG_TRIGGERED);

	/* Check delta time before processing event (Absolute value is a safe check. Negative values
	 * won't ocurr here... hopefully).
	 */
	if ((unsigned int) labs((long) (time(NULL) - entry->trigger)) >= rund.config.core.delta_noexec) {
		log_warn("entry_daemon_exec_dispatch(): Entry delta T (%d seconds) is >= than the configured delta T for noexec (%d seconds). Ignoring execution...\n", time(NULL) - entry->trigger, rund.config.core.delta_noexec);

		/* Do not deliver this entry to the uSched executer (use) */
		goto _process;
	}

	/* Allocate message memory */
	if (!(buf = mm_alloc((size_t) rund.config.core.ipc_msgsize))) {
		log_warn("entry_daemon_exec_dispatch(): mm_alloc(): %s\n", strerror(errno));

		/* Force daemon to be restarted and reload a clean state */
		runtime_daemon_fatal();

		goto _finish;
	}

	memset(buf, 0, (size_t) rund.config.core.ipc_msgsize);

	/* Check if this entry is authorized */
	if (!entry_has_flag(entry, USCHED_ENTRY_FLAG_AUTHORIZED)) {
		log_warn("entry_daemon_exec_dispatch(): Unauthorized entry found. Discarding...\n");

		/* Remove this entry as it is invalid */
		goto _remove;
	}

	/* Check entry signature */
	if (!entry_check_signature(entry)) {
		log_warn("entry_daemon_exec_dispatch(): Entry ID 0x%016llX signature is invalid.\n", entry->id);

		/* Mark this entry as invalid. */
		entry_set_flag(entry, USCHED_ENTRY_FLAG_INVALID);

		/* Serializated data is now invalid. TODO: Serialize this entry... */
		entry_unset_flag(entry, USCHED_ENTRY_FLAG_SERIALIZED);

		goto _finish;
	}

	/* Replace subject variables with current values */
	if (!(cmd = vars_replace_all(entry->subj, (struct usched_vars [1]) { { entry->id, entry->username, entry->uid, entry->gid, entry->trigger, entry->step, entry->expire } })))
		cmd = entry->subj;

	/* Check if the message fits in the configured message size.
	 * Although this check was already performed when receiving the entry from the user,
	 * this one is required since now the variables are expanded.
	 */
	if ((strlen(cmd) + 21) > (size_t) rund.config.core.ipc_msgsize) {
		log_warn("entry_daemon_exec_dispatch(): msg_size > sizeof(buf) (Entry ID: 0x%016llX\n", entry->id);

		/* Mark this entry as invalid. */
		entry_set_flag(entry, USCHED_ENTRY_FLAG_INVALID);

		/* Serializated data is now invalid. TODO: Serialize this entry... */
		entry_unset_flag(entry, USCHED_ENTRY_FLAG_SERIALIZED);

		goto _finish;
	}

	/* Craft message */
	memcpy(buf, &entry->id, 8);
	memcpy(buf + 8, &entry->uid, 4);
	memcpy(buf + 12, &entry->gid, 4);
	memcpy(buf + 16, &entry->trigger, 4);
	memcpy(buf + 20, cmd, strlen(cmd));

	/* Free cmd memory if allocated by vars_replace_all() */
	if (cmd != entry->subj)
		mm_free(cmd);

	debug_printf(DEBUG_INFO, "Executing entry->id: 0x%016llX\n", entry->id);

#if CONFIG_USE_IPC_PMQ == 1
	/* Deliver message to uSched executer (use). Give up on timeout to avoid this notifier to
	 * stall in the case of a full message queue or unresponsive executer.
	 */
	if (mq_timedsend(rund.ipcd_use_wo, buf, (size_t) rund.config.core.ipc_msgsize, 0, &pmq_timeout) < 0) {
		log_warn("entry_daemon_exec_dispatch(): mq_send(): %s\n", strerror(errno));
#elif CONFIG_USE_IPC_UNIX == 1 || CONFIG_USE_IPC_INET == 1
	if (panet_write(rund.ipcd, buf, (size_t) rund.config.core.ipc_msgsize) != (ssize_t) rund.config.core.ipc_msgsize) {
		log_warn("entry_daemon_exec_dispatch(): panet_write(): %s\n", strerror(errno));
#else
 #error "No IPC mechanism defined."
#endif

		/* NOTE:
		 *
		 * We should not delete this entry from active pool if we're unable to write to
		 * the pqueue of the execution process.
		 *
		 * We should continue processing the scheduled entries as if nothing happened, and
		 * notify the user with a critical log entry. 
		 *
		 */

		log_crit("entry_daemon_exec_dispatch(): The Entry ID 0x%016llX was NOT executed at timestamp %u due to the previously reported error while performing an event write.\n", entry->id, entry->trigger);
	}

_process:
	/* This lock is required in order to sync the scheduling interface init/destroy engine with
	 * the async routines that may be triggered by libpsched. We must grant that the
	 * schedule_daemon_active() routine that is executed inside the schedule_entry_update() have
	 * the rund.mutex_apool lock acquired (which is the same lock that schedule_daemon_destroy()
	 * acquires).
	 */
	pthread_mutex_lock(&rund.mutex_apool);

	/* Update trigger, step and expire parameters of the entry based on psched library data */
	if ((ret = schedule_entry_update(entry)) == 1) {
		pthread_mutex_unlock(&rund.mutex_apool);

		/* Entry was successfully updated. */
		goto _finish;
	}

	errsv = errno;


	/* Check if an error ocurred */
	if (ret < 0) {
		errno = errsv;

		log_info("entry_daemon_exec_dispatch(): schedule_entry_update(): %s. (Entry ID: 0x%016llX)\n", strerror(errno), entry->id);

		/* Unlock only after the last use of 'entry' reference */
		pthread_mutex_unlock(&rund.mutex_apool);

		runtime_daemon_fatal();

		goto _finish;
	}

	pthread_mutex_unlock(&rund.mutex_apool);

	/* Entry was not found. This means that it wasn't a recurrent entry (no step).
	 * It should be deleted from the active pool.
	 */

	log_info("entry_daemon_exec_dispatch(): The Entry ID 0x%016llX isn't recurrent and will be deleted from the active pool.", entry->id);

_remove:
	/* Remove the entry from active pool */
	pthread_mutex_lock(&rund.mutex_apool);
	rund.apool->del(rund.apool, entry);
	pthread_mutex_unlock(&rund.mutex_apool);

_finish:
	if (buf)
		mm_free(buf);
}

int entry_daemon_serialize(pall_fd_t fd, void *data) {
	int errsv = 0;
	struct usched_entry *entry = data;
	char buf[sizeof(entry->id) + sizeof(entry->flags) + sizeof(entry->uid) + sizeof(entry->gid) + sizeof(entry->trigger) + sizeof(entry->step) + sizeof(entry->expire) + sizeof(entry->username) + sizeof(entry->subj_size) + sizeof(entry->create_time) + sizeof(entry->signature)];
	size_t offset = 0;

	/* If this entry is set to be REMOVED, do not serialize it */
	if (entry_has_flag(entry, USCHED_ENTRY_FLAG_REMOVED))
		return 0; /* No errors here, just ignore the entry and return success */

	/* Validate the signature agains the current entry data */
	if (!entry_check_signature(entry))
		log_crit("entry_daemon_serialize(): Entry ID 0x%016llX signature is invalid. The entry will be serialized, but it will fail to load on next daemon restart.\n", entry->id);

	/* Craft serialization buffer */
	memcpy(buf + offset, &entry->id, sizeof(entry->id));
	offset += sizeof(entry->id);

	memcpy(buf + offset, &entry->flags, sizeof(entry->flags));
	offset += sizeof(entry->flags);

	memcpy(buf + offset, &entry->uid, sizeof(entry->uid));
	offset += sizeof(entry->uid);

	memcpy(buf + offset, &entry->gid, sizeof(entry->gid));
	offset += sizeof(entry->gid);

	memcpy(buf + offset, &entry->trigger, sizeof(entry->trigger));
	offset += sizeof(entry->trigger);

	memcpy(buf + offset, &entry->step, sizeof(entry->step));
	offset += sizeof(entry->step);

	memcpy(buf + offset, &entry->expire, sizeof(entry->expire));
	offset += sizeof(entry->expire);

	memcpy(buf + offset, entry->username, sizeof(entry->username));
	offset += sizeof(entry->username);

	memcpy(buf + offset, &entry->subj_size, sizeof(entry->subj_size));
	offset += sizeof(entry->subj_size);

	memcpy(buf + offset, &entry->create_time, sizeof(entry->create_time));
	offset += sizeof(entry->create_time);

	memcpy(buf + offset, entry->signature, sizeof(entry->signature));

	/* Serialize data */
	if (write(fd, buf, sizeof(buf)) != (ssize_t) sizeof(buf)) {
		errsv = errno;
		log_crit("entry_daemon_serialize(): write(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Serialize subject */
	if (write(fd, entry->subj, entry->subj_size) != (ssize_t) entry->subj_size) {
		errsv = errno;
		log_crit("entry_daemon_serialize(): write(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Mark entry as serialized */
	entry_set_flag(entry, USCHED_ENTRY_FLAG_SERIALIZED);

	/* All good */
	return 0;
}

void *entry_daemon_unserialize(pall_fd_t fd) {
	int errsv = 0;
	struct usched_entry *entry = NULL;
	char buf[sizeof(entry->id) + sizeof(entry->flags) + sizeof(entry->uid) + sizeof(entry->gid) + sizeof(entry->trigger) + sizeof(entry->step) + sizeof(entry->expire) + sizeof(entry->username) + sizeof(entry->subj_size) + sizeof(entry->create_time) + sizeof(entry->signature)];
	size_t offset = 0;

	/* Allocate enough memory for the entry */
	if (!(entry = mm_alloc(sizeof(struct usched_entry)))) {
		errsv = errno;
		log_crit("entry_daemon_unserialize(): mm_alloc(): %s\n", strerror(errno));
		errno = errsv;
		return NULL;
	}

	memset(entry, 0, sizeof(struct usched_entry));

	/* Read serialized buffer */
	if (read(fd, buf, sizeof(buf)) != (ssize_t) sizeof(buf)) {
		errsv = errno;
		log_crit("entry_daemon_unserialize(): read(): %s\n", strerror(errno));
		entry_destroy(entry);
		errno = errsv;
		return NULL;
	}

	/* Populate entry fields */
	memcpy(&entry->id, buf + offset, sizeof(entry->id));
	offset += sizeof(entry->id);

	memcpy(&entry->flags, buf + offset, sizeof(entry->flags));
	offset += sizeof(entry->flags);

	memcpy(&entry->uid, buf + offset, sizeof(entry->uid));
	offset += sizeof(entry->uid);

	memcpy(&entry->gid, buf + offset, sizeof(entry->gid));
	offset += sizeof(entry->gid);

	memcpy(&entry->trigger, buf + offset, sizeof(entry->trigger));
	offset += sizeof(entry->trigger);

	memcpy(&entry->step, buf + offset, sizeof(entry->step));
	offset += sizeof(entry->step);

	memcpy(&entry->expire, buf + offset, sizeof(entry->expire));
	offset += sizeof(entry->expire);

	memcpy(entry->username, buf + offset, sizeof(entry->username));
	offset += sizeof(entry->username);

	memcpy(&entry->subj_size, buf + offset, sizeof(entry->subj_size));
	offset += sizeof(entry->subj_size);

	memcpy(&entry->create_time, buf + offset, sizeof(entry->create_time));
	offset += sizeof(entry->create_time);

	memcpy(entry->signature, buf + offset, sizeof(entry->signature));

	/* Allocate memory for entry subject */
	if (!(entry->subj = mm_alloc(entry->subj_size + 1))) {
		errsv = errno;
		log_crit("entry_daemon_unserialize(): mm_alloc(): %s\n", strerror(errno));
		entry_destroy(entry);
		errno = errsv;
		return NULL;
	}

	memset(entry->subj, 0, entry->subj_size + 1);

	/* Read the entry subject */
	if (read(fd, entry->subj, entry->subj_size) != (ssize_t) entry->subj_size) {
		errsv = errno;
		log_crit("entry_daemon_unserialize(): read(): %s\n", strerror(errno));
		entry_destroy(entry);
		errno = errsv;
		return NULL;
	}	

	/* Check entry signature */
	if (!entry_check_signature(entry)) {
		log_crit("entry_daemon_unserialize(): Entry ID 0x%016llX signature is invalid.\n", entry->id);
		entry_destroy(entry);
		errno = EINVAL;
		return NULL;
	}

	/* We've just unserialized this entry, so it is serialized */
	entry_set_flag(entry, USCHED_ENTRY_FLAG_SERIALIZED);

	/* All good */
	return entry;
}

