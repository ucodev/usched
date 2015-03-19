/**
 * @file core.c
 * @brief uSched
 *        Core configuration and administration interface
 *
 * Date: 19-03-2015
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

#include <fsop/path.h>
#include <fsop/file.h>

#include "config.h"
#include "core.h"
#include "file.h"
#include "log.h"
#include "mm.h"
#include "usched.h"
#include "print.h"
#include "ipc.h"

int core_admin_commit(void) {
	int errsv = 0;

	/* Check if services are running and stop processing if they are */
	if (fsop_path_exists(CONFIG_USCHED_DAEMON_PID_FILE)) {
		log_crit("core_admin_commit(): uSched services are running (usd). You must stop all uSched services before you can commit core configuration changes.\n");
		errno = EBUSY;
		return -1;
	}

	if (fsop_path_exists(CONFIG_USCHED_EXEC_PID_FILE)) {
		log_crit("core_admin_commit(): uSched services are running (use). You must stop all uSched services before you can commit core configuration changes.\n");
		errno = EBUSY;
		return -1;
	}

	/* Destroy the current message queue */
	ipc_admin_delete();

	/* Destroy the current configuration */
	config_admin_destroy();

	/* delta.noexec */
	if (fsop_cp(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_CORE "/." CONFIG_USCHED_FILE_CORE_DELTA_NOEXEC, CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_CORE "/" CONFIG_USCHED_FILE_CORE_DELTA_NOEXEC, 128) < 0) {
		errsv = errno;
		log_crit("core_admin_commit(): fsop_cp(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* delta.reload */
	if (fsop_cp(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_CORE "/." CONFIG_USCHED_FILE_CORE_DELTA_RELOAD, CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_CORE "/" CONFIG_USCHED_FILE_CORE_DELTA_RELOAD, 128) < 0) {
		errsv = errno;
		log_crit("core_admin_commit(): fsop_cp(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* jail.dir */
	if (fsop_cp(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_CORE "/." CONFIG_USCHED_FILE_CORE_JAIL_DIR, CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_CORE "/" CONFIG_USCHED_FILE_CORE_JAIL_DIR, 128) < 0) {
		errsv = errno;
		log_crit("core_admin_commit(): fsop_cp(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* ipc.msgmax */
	if (fsop_cp(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_CORE "/." CONFIG_USCHED_FILE_CORE_IPC_MSGMAX, CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_CORE "/" CONFIG_USCHED_FILE_CORE_IPC_MSGMAX, 128) < 0) {
		errsv = errno;
		log_crit("core_admin_commit(): fsop_cp(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* ipc.msgsize */
	if (fsop_cp(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_CORE "/." CONFIG_USCHED_FILE_CORE_IPC_MSGSIZE, CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_CORE "/" CONFIG_USCHED_FILE_CORE_IPC_MSGSIZE, 128) < 0) {
		errsv = errno;
		log_crit("core_admin_commit(): fsop_cp(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* ipc.name */
	if (fsop_cp(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_CORE "/." CONFIG_USCHED_FILE_CORE_IPC_NAME, CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_CORE "/" CONFIG_USCHED_FILE_CORE_IPC_NAME, 128) < 0) {
		errsv = errno;
		log_crit("core_admin_commit(): fsop_cp(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* privdrop.user */
	if (fsop_cp(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_CORE "/." CONFIG_USCHED_FILE_CORE_PRIVDROP_USER, CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_CORE "/" CONFIG_USCHED_FILE_CORE_PRIVDROP_USER, 128) < 0) {
		errsv = errno;
		log_crit("core_admin_commit(): fsop_cp(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* privdrop.group */
	if (fsop_cp(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_CORE "/." CONFIG_USCHED_FILE_CORE_PRIVDROP_GROUP, CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_CORE "/" CONFIG_USCHED_FILE_CORE_PRIVDROP_GROUP, 128) < 0) {
		errsv = errno;
		log_crit("core_admin_commit(): fsop_cp(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* serialize.file */
	if (fsop_cp(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_CORE "/." CONFIG_USCHED_FILE_CORE_SERIALIZE_FILE, CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_CORE "/" CONFIG_USCHED_FILE_CORE_SERIALIZE_FILE, 128) < 0) {
		errsv = errno;
		log_crit("core_admin_commit(): fsop_cp(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* thread.priority */
	if (fsop_cp(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_CORE "/." CONFIG_USCHED_FILE_CORE_THREAD_PRIORITY, CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_CORE "/" CONFIG_USCHED_FILE_CORE_THREAD_PRIORITY, 128) < 0) {
		errsv = errno;
		log_crit("core_admin_commit(): fsop_cp(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* thread.workers */
	if (fsop_cp(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_CORE "/." CONFIG_USCHED_FILE_CORE_THREAD_WORKERS, CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_CORE "/" CONFIG_USCHED_FILE_CORE_THREAD_WORKERS, 128) < 0) {
		errsv = errno;
		log_crit("core_admin_commit(): fsop_cp(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Re-initialize the configuration */
	if (config_admin_init() < 0) {
		errsv = errno;
		log_crit("core_admin_commit(): config_admin_init(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Create the message queue */
	if (ipc_admin_create() < 0) {
		errsv = errno;
		log_crit("core_admin_commit(): ipc_admin_create(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* All good */
	return 0;
}

int core_admin_rollback(void) {
	int errsv = 0;

	/* delta.noexec */
	if (fsop_cp(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_CORE "/" CONFIG_USCHED_FILE_CORE_DELTA_NOEXEC, CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_CORE "/." CONFIG_USCHED_FILE_CORE_DELTA_NOEXEC, 128) < 0) {
		errsv = errno;
		log_crit("core_admin_rollback(): fsop_cp(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* delta.reload */
	if (fsop_cp(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_CORE "/" CONFIG_USCHED_FILE_CORE_DELTA_RELOAD, CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_CORE "/." CONFIG_USCHED_FILE_CORE_DELTA_RELOAD, 128) < 0) {
		errsv = errno;
		log_crit("core_admin_rollback(): fsop_cp(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* jail.dir */
	if (fsop_cp(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_CORE "/" CONFIG_USCHED_FILE_CORE_JAIL_DIR, CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_CORE "/." CONFIG_USCHED_FILE_CORE_JAIL_DIR, 128) < 0) {
		errsv = errno;
		log_crit("core_admin_rollback(): fsop_cp(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* ipc.msgmax */
	if (fsop_cp(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_CORE "/" CONFIG_USCHED_FILE_CORE_IPC_MSGMAX, CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_CORE "/." CONFIG_USCHED_FILE_CORE_IPC_MSGMAX, 128) < 0) {
		errsv = errno;
		log_crit("core_admin_rollback(): fsop_cp(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* ipc.msgsize */
	if (fsop_cp(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_CORE "/" CONFIG_USCHED_FILE_CORE_IPC_MSGSIZE, CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_CORE "/." CONFIG_USCHED_FILE_CORE_IPC_MSGSIZE, 128) < 0) {
		errsv = errno;
		log_crit("core_admin_rollback(): fsop_cp(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* ipc.name */
	if (fsop_cp(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_CORE "/" CONFIG_USCHED_FILE_CORE_IPC_NAME, CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_CORE "/." CONFIG_USCHED_FILE_CORE_IPC_NAME, 128) < 0) {
		errsv = errno;
		log_crit("core_admin_rollback(): fsop_cp(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* privdrop.user */
	if (fsop_cp(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_CORE "/" CONFIG_USCHED_FILE_CORE_PRIVDROP_USER, CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_CORE "/." CONFIG_USCHED_FILE_CORE_PRIVDROP_USER, 128) < 0) {
		errsv = errno;
		log_crit("core_admin_rollback(): fsop_cp(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* privdrop.group */
	if (fsop_cp(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_CORE "/" CONFIG_USCHED_FILE_CORE_PRIVDROP_GROUP, CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_CORE "/." CONFIG_USCHED_FILE_CORE_PRIVDROP_GROUP, 128) < 0) {
		errsv = errno;
		log_crit("core_admin_rollback(): fsop_cp(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* serialize.file */
	if (fsop_cp(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_CORE "/" CONFIG_USCHED_FILE_CORE_SERIALIZE_FILE, CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_CORE "/." CONFIG_USCHED_FILE_CORE_SERIALIZE_FILE, 128) < 0) {
		errsv = errno;
		log_crit("core_admin_rollback(): fsop_cp(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* thread.priority */
	if (fsop_cp(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_CORE "/" CONFIG_USCHED_FILE_CORE_THREAD_PRIORITY, CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_CORE "/." CONFIG_USCHED_FILE_CORE_THREAD_PRIORITY, 128) < 0) {
		errsv = errno;
		log_crit("core_admin_rollback(): fsop_cp(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* thread.workers */
	if (fsop_cp(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_CORE "/" CONFIG_USCHED_FILE_CORE_THREAD_WORKERS, CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_CORE "/." CONFIG_USCHED_FILE_CORE_THREAD_WORKERS, 128) < 0) {
		errsv = errno;
		log_crit("core_admin_rollback(): fsop_cp(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* All good */
	return 0;
}

int core_admin_show(void) {
	int errsv = 0;

	if (core_admin_delta_noexec_show() < 0) {
		errsv = errno;
		log_crit("core_admin_show(): core_admin_delta_noexec_show(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	if (core_admin_delta_reload_show() < 0) {
		errsv = errno;
		log_crit("core_admin_show(): core_admin_delta_reload_show(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	if (core_admin_jail_dir_show() < 0) {
		errsv = errno;
		log_crit("core_admin_show(): core_admin_jail_dir_show(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	if (core_admin_ipc_msgmax_show() < 0) {
		errsv = errno;
		log_crit("core_admin_show(): core_admin_ipc_msgmax_show(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	if (core_admin_ipc_msgsize_show() < 0) {
		errsv = errno;
		log_crit("core_admin_show(): core_admin_ipc_msgsize_show(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	if (core_admin_ipc_name_show() < 0) {
		errsv = errno;
		log_crit("core_admin_show(): core_admin_ipc_name_show(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	if (core_admin_ipc_key_show() < 0) {
		errsv = errno;
		log_crit("core_admin_show(): core_admin_ipc_key_show(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	if (core_admin_privdrop_group_show() < 0) {
		errsv = errno;
		log_crit("core_admin_show(): core_admin_privdrop_group_show(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	if (core_admin_privdrop_user_show() < 0) {
		errsv = errno;
		log_crit("core_admin_show(): core_admin_privdrop_user_show(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	if (core_admin_serialize_file_show() < 0) {
		errsv = errno;
		log_crit("core_admin_show(): core_admin_serialize_file_show(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	if (core_admin_thread_priority_show() < 0) {
		errsv = errno;
		log_crit("core_admin_show(): core_admin_thread_priority_show(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	if (core_admin_thread_workers_show() < 0) {
		errsv = errno;
		log_crit("core_admin_show(): core_admin_thread_workers_show(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	return 0;
}

int core_admin_delta_noexec_show(void) {
	int errsv = 0;
	char *value = NULL, *value_tmp = NULL, *value_print = NULL;

	if (!(value_tmp = file_read_line_single(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_CORE "/." CONFIG_USCHED_FILE_CORE_DELTA_NOEXEC))) {
		errsv = errno;
		log_crit("core_admin_delta_noexec_show(): file_read_line_single(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	if (!(value = file_read_line_single(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_CORE "/" CONFIG_USCHED_FILE_CORE_DELTA_NOEXEC))) {
		errsv = errno;
		log_crit("core_admin_delta_noexec_show(): file_read_line_single(): %s\n", strerror(errno));
		mm_free(value_tmp);
		errno = errsv;
		return -1;
	}

	/* Check which value to print */
	if (!strcmp(value, value_tmp)) {
		if (!(value_print = mm_alloc(strlen(value) + 1))) {
			errsv = errno;
			log_crit("core_admin_delta_noexec_show(): mm_alloc(): %s\n", strerror(errno));
			mm_free(value);
			mm_free(value_tmp);
			errno = errsv;
			return -1;
		}

		strcpy(value_print, value);
	} else {
		if (!(value_print = mm_alloc(strlen(value_tmp) + 2))) {
			errsv = errno;
			log_crit("core_admin_delta_noexec_show(): mm_alloc(): %s\n", strerror(errno));
			mm_free(value);
			mm_free(value_tmp);
			errno = errsv;
			return -1;
		}

		/* Show the temporary value with a trailing '*' */
		strcpy(value_print, value_tmp);
		strcat(value_print, "*");
	}

	/* Print the output */
	print_admin_category_var_value(USCHED_CATEGORY_CORE_STR, CONFIG_USCHED_FILE_CORE_DELTA_NOEXEC, value_print);

	/* Free memory */
	mm_free(value);
	mm_free(value_tmp);
	mm_free(value_print);

	/* All good */
	return 0;
}

int core_admin_delta_noexec_change(const char *delta_noexec) {
	int errsv = 0;

	if (file_write_line_single(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_CORE "/." CONFIG_USCHED_FILE_CORE_DELTA_NOEXEC, delta_noexec) < 0) {
		errsv = errno;
		log_crit("core_admin_delta_noexec_change(): file_write_line_single(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	if (core_admin_delta_noexec_show() < 0) {
		errsv = errno;
		log_crit("core_admin_delta_noexec_change(): core_admin_delta_noexec_show(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	return 0;
}

int core_admin_delta_reload_show(void) {
	int errsv = 0;
	char *value = NULL, *value_tmp = NULL, *value_print = NULL;

	if (!(value_tmp = file_read_line_single(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_CORE "/." CONFIG_USCHED_FILE_CORE_DELTA_RELOAD))) {
		errsv = errno;
		log_crit("core_admin_delta_reload_show(): file_read_line_single(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	if (!(value = file_read_line_single(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_CORE "/" CONFIG_USCHED_FILE_CORE_DELTA_RELOAD))) {
		errsv = errno;
		log_crit("core_admin_delta_reload_show(): file_read_line_single(): %s\n", strerror(errno));
		mm_free(value_tmp);
		errno = errsv;
		return -1;
	}

	/* Check which value to print */
	if (!strcmp(value, value_tmp)) {
		if (!(value_print = mm_alloc(strlen(value) + 1))) {
			errsv = errno;
			log_crit("core_admin_delta_reload_show(): mm_alloc(): %s\n", strerror(errno));
			mm_free(value);
			mm_free(value_tmp);
			errno = errsv;
			return -1;
		}

		strcpy(value_print, value);
	} else {
		if (!(value_print = mm_alloc(strlen(value_tmp) + 2))) {
			errsv = errno;
			log_crit("core_admin_delta_reload_show(): mm_alloc(): %s\n", strerror(errno));
			mm_free(value);
			mm_free(value_tmp);
			errno = errsv;
			return -1;
		}

		/* Show the temporary value with a trailing '*' */
		strcpy(value_print, value_tmp);
		strcat(value_print, "*");
	}

	/* Print the output */
	print_admin_category_var_value(USCHED_CATEGORY_CORE_STR, CONFIG_USCHED_FILE_CORE_DELTA_RELOAD, value_print);

	/* Free memory */
	mm_free(value);
	mm_free(value_tmp);
	mm_free(value_print);

	/* All good */
	return 0;
}

int core_admin_delta_reload_change(const char *delta_reload) {
	int errsv = 0;

	if (file_write_line_single(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_CORE "/." CONFIG_USCHED_FILE_CORE_DELTA_RELOAD, delta_reload) < 0) {
		errsv = errno;
		log_crit("core_admin_delta_reload_change(): file_write_line_single(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	if (core_admin_delta_reload_show() < 0) {
		errsv = errno;
		log_crit("core_admin_delta_reload_change(): core_admin_delta_reload_show(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	return 0;
}

int core_admin_jail_dir_show(void) {
	int errsv = 0;
	char *value = NULL, *value_tmp = NULL, *value_print = NULL;

	if (!(value_tmp = file_read_line_single(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_CORE "/." CONFIG_USCHED_FILE_CORE_JAIL_DIR))) {
		errsv = errno;
		log_crit("core_admin_jail_dir_show(): file_read_line_single(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	if (!(value = file_read_line_single(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_CORE "/" CONFIG_USCHED_FILE_CORE_JAIL_DIR))) {
		errsv = errno;
		log_crit("core_admin_jail_dir_show(): file_read_line_single(): %s\n", strerror(errno));
		mm_free(value_tmp);
		errno = errsv;
		return -1;
	}

	/* Check which value to print */
	if (!strcmp(value, value_tmp)) {
		if (!(value_print = mm_alloc(strlen(value) + 1))) {
			errsv = errno;
			log_crit("core_admin_jail_dir_show(): mm_alloc(): %s\n", strerror(errno));
			mm_free(value);
			mm_free(value_tmp);
			errno = errsv;
			return -1;
		}

		strcpy(value_print, value);
	} else {
		if (!(value_print = mm_alloc(strlen(value_tmp) + 2))) {
			errsv = errno;
			log_crit("core_admin_jail_dir_show(): mm_alloc(): %s\n", strerror(errno));
			mm_free(value);
			mm_free(value_tmp);
			errno = errsv;
			return -1;
		}

		/* Show the temporary value with a trailing '*' */
		strcpy(value_print, value_tmp);
		strcat(value_print, "*");
	}

	/* Print the output */
	print_admin_category_var_value(USCHED_CATEGORY_CORE_STR, CONFIG_USCHED_FILE_CORE_JAIL_DIR, value_print);

	/* Free memory */
	mm_free(value);
	mm_free(value_tmp);
	mm_free(value_print);

	/* All good */
	return 0;
}

int core_admin_jail_dir_change(const char *jail_dir) {
	int errsv = 0;

	if (file_write_line_single(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_CORE "/." CONFIG_USCHED_FILE_CORE_JAIL_DIR, jail_dir) < 0) {
		errsv = errno;
		log_crit("core_admin_jail_dir_change(): file_write_line_single(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	if (core_admin_jail_dir_show() < 0) {
		errsv = errno;
		log_crit("core_admin_jail_dir_change(): core_admin_jail_dir_show(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	return 0;
}

int core_admin_ipc_msgmax_show(void) {
	int errsv = 0;
	char *value = NULL, *value_tmp = NULL, *value_print = NULL;

	if (!(value_tmp = file_read_line_single(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_CORE "/." CONFIG_USCHED_FILE_CORE_IPC_MSGMAX))) {
		errsv = errno;
		log_crit("core_admin_ipc_msgmax_show(): file_read_line_single(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	if (!(value = file_read_line_single(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_CORE "/" CONFIG_USCHED_FILE_CORE_IPC_MSGMAX))) {
		errsv = errno;
		log_crit("core_admin_ipc_msgmax_show(): file_read_line_single(): %s\n", strerror(errno));
		mm_free(value_tmp);
		errno = errsv;
		return -1;
	}

	/* Check which value to print */
	if (!strcmp(value, value_tmp)) {
		if (!(value_print = mm_alloc(strlen(value) + 1))) {
			errsv = errno;
			log_crit("core_admin_ipc_msgmax_show(): mm_alloc(): %s\n", strerror(errno));
			mm_free(value);
			mm_free(value_tmp);
			errno = errsv;
			return -1;
		}

		strcpy(value_print, value);
	} else {
		if (!(value_print = mm_alloc(strlen(value_tmp) + 2))) {
			errsv = errno;
			log_crit("core_admin_ipc_msgmax_show(): mm_alloc(): %s\n", strerror(errno));
			mm_free(value);
			mm_free(value_tmp);
			errno = errsv;
			return -1;
		}

		/* Show the temporary value with a trailing '*' */
		strcpy(value_print, value_tmp);
		strcat(value_print, "*");
	}

	/* Print the output */
	print_admin_category_var_value(USCHED_CATEGORY_CORE_STR, CONFIG_USCHED_FILE_CORE_IPC_MSGMAX, value_print);

	/* Free memory */
	mm_free(value);
	mm_free(value_tmp);
	mm_free(value_print);

	/* All good */
	return 0;
}

int core_admin_ipc_msgmax_change(const char *ipc_msgmax) {
	int errsv = 0;

	if (file_write_line_single(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_CORE "/." CONFIG_USCHED_FILE_CORE_IPC_MSGMAX, ipc_msgmax) < 0) {
		errsv = errno;
		log_crit("core_admin_ipc_msgmax_change(): file_write_line_single(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	if (core_admin_ipc_msgmax_show() < 0) {
		errsv = errno;
		log_crit("core_admin_ipc_msgmax_change(): core_admin_ipc_msgmax_show(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	return 0;
}

int core_admin_ipc_msgsize_show(void) {
	int errsv = 0;
	char *value = NULL, *value_tmp = NULL, *value_print = NULL;

	if (!(value_tmp = file_read_line_single(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_CORE "/." CONFIG_USCHED_FILE_CORE_IPC_MSGSIZE))) {
		errsv = errno;
		log_crit("core_admin_ipc_msgsize_show(): file_read_line_single(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	if (!(value = file_read_line_single(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_CORE "/" CONFIG_USCHED_FILE_CORE_IPC_MSGSIZE))) {
		errsv = errno;
		log_crit("core_admin_ipc_msgsize_show(): file_read_line_single(): %s\n", strerror(errno));
		mm_free(value_tmp);
		errno = errsv;
		return -1;
	}

	/* Check which value to print */
	if (!strcmp(value, value_tmp)) {
		if (!(value_print = mm_alloc(strlen(value) + 1))) {
			errsv = errno;
			log_crit("core_admin_ipc_msgsize_show(): mm_alloc(): %s\n", strerror(errno));
			mm_free(value);
			mm_free(value_tmp);
			errno = errsv;
			return -1;
		}

		strcpy(value_print, value);
	} else {
		if (!(value_print = mm_alloc(strlen(value_tmp) + 2))) {
			errsv = errno;
			log_crit("core_admin_ipc_msgsize_show(): mm_alloc(): %s\n", strerror(errno));
			mm_free(value);
			mm_free(value_tmp);
			errno = errsv;
			return -1;
		}

		/* Show the temporary value with a trailing '*' */
		strcpy(value_print, value_tmp);
		strcat(value_print, "*");
	}

	/* Print the output */
	print_admin_category_var_value(USCHED_CATEGORY_CORE_STR, CONFIG_USCHED_FILE_CORE_IPC_MSGSIZE, value_print);

	/* Free memory */
	mm_free(value);
	mm_free(value_tmp);
	mm_free(value_print);

	/* All good */
	return 0;
}

int core_admin_ipc_msgsize_change(const char *ipc_msgsize) {
	int errsv = 0;

	if (file_write_line_single(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_CORE "/." CONFIG_USCHED_FILE_CORE_IPC_MSGSIZE, ipc_msgsize) < 0) {
		errsv = errno;
		log_crit("core_admin_ipc_msgsize_change(): file_write_line_single(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	if (core_admin_ipc_msgsize_show() < 0) {
		errsv = errno;
		log_crit("core_admin_ipc_msgsize_change(): core_admin_ipc_msgsize_show(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	return 0;
}

int core_admin_ipc_name_show(void) {
	int errsv = 0;
	char *value = NULL, *value_tmp = NULL, *value_print = NULL;

	if (!(value_tmp = file_read_line_single(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_CORE "/." CONFIG_USCHED_FILE_CORE_IPC_NAME))) {
		errsv = errno;
		log_crit("core_admin_ipc_name_show(): file_read_line_single(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	if (!(value = file_read_line_single(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_CORE "/" CONFIG_USCHED_FILE_CORE_IPC_NAME))) {
		errsv = errno;
		log_crit("core_admin_ipc_name_show(): file_read_line_single(): %s\n", strerror(errno));
		mm_free(value_tmp);
		errno = errsv;
		return -1;
	}

	/* Check which value to print */
	if (!strcmp(value, value_tmp)) {
		if (!(value_print = mm_alloc(strlen(value) + 1))) {
			errsv = errno;
			log_crit("core_admin_ipc_name_show(): mm_alloc(): %s\n", strerror(errno));
			mm_free(value);
			mm_free(value_tmp);
			errno = errsv;
			return -1;
		}

		strcpy(value_print, value);
	} else {
		if (!(value_print = mm_alloc(strlen(value_tmp) + 2))) {
			errsv = errno;
			log_crit("core_admin_ipc_name_show(): mm_alloc(): %s\n", strerror(errno));
			mm_free(value);
			mm_free(value_tmp);
			errno = errsv;
			return -1;
		}

		/* Show the temporary value with a trailing '*' */
		strcpy(value_print, value_tmp);
		strcat(value_print, "*");
	}

	/* Print the output */
	print_admin_category_var_value(USCHED_CATEGORY_CORE_STR, CONFIG_USCHED_FILE_CORE_IPC_NAME, value_print);

	/* Free memory */
	mm_free(value);
	mm_free(value_tmp);
	mm_free(value_print);

	/* All good */
	return 0;
}

int core_admin_ipc_name_change(const char *ipc_name) {
	int errsv = 0;

	if (file_write_line_single(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_CORE "/." CONFIG_USCHED_FILE_CORE_IPC_NAME, ipc_name) < 0) {
		errsv = errno;
		log_crit("core_admin_ipc_name_change(): file_write_line_single(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	if (core_admin_ipc_name_show() < 0) {
		errsv = errno;
		log_crit("core_admin_ipc_name_change(): core_admin_ipc_name_show(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	return 0;
}

int core_admin_ipc_key_show(void) {
	int errsv = 0;
	char *value = NULL, *value_tmp = NULL, *value_print = NULL;

	if (!(value_tmp = file_read_line_single(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_CORE "/." CONFIG_USCHED_FILE_CORE_IPC_KEY))) {
		errsv = errno;
		log_crit("core_admin_ipc_key_show(): file_read_line_single(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	if (!(value = file_read_line_single(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_CORE "/" CONFIG_USCHED_FILE_CORE_IPC_KEY))) {
		errsv = errno;
		log_crit("core_admin_ipc_key_show(): file_read_line_single(): %s\n", strerror(errno));
		mm_free(value_tmp);
		errno = errsv;
		return -1;
	}

	/* Check which value to print */
	if (!strcmp(value, value_tmp)) {
		if (!(value_print = mm_alloc(strlen(value) + 1))) {
			errsv = errno;
			log_crit("core_admin_ipc_key_show(): mm_alloc(): %s\n", strerror(errno));
			mm_free(value);
			mm_free(value_tmp);
			errno = errsv;
			return -1;
		}

		strcpy(value_print, value);
	} else {
		if (!(value_print = mm_alloc(strlen(value_tmp) + 2))) {
			errsv = errno;
			log_crit("core_admin_ipc_key_show(): mm_alloc(): %s\n", strerror(errno));
			mm_free(value);
			mm_free(value_tmp);
			errno = errsv;
			return -1;
		}

		/* Show the temporary value with a trailing '*' */
		strcpy(value_print, value_tmp);
		strcat(value_print, "*");
	}

	/* Print the output */
	print_admin_category_var_value(USCHED_CATEGORY_CORE_STR, CONFIG_USCHED_FILE_CORE_IPC_KEY, value_print);

	/* Free memory */
	mm_free(value);
	mm_free(value_tmp);
	mm_free(value_print);

	/* All good */
	return 0;
}

int core_admin_ipc_key_change(const char *ipc_key) {
	int errsv = 0;

	if (file_write_line_single(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_CORE "/." CONFIG_USCHED_FILE_CORE_IPC_KEY, ipc_key) < 0) {
		errsv = errno;
		log_crit("core_admin_ipc_key_change(): file_write_line_single(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	if (core_admin_ipc_key_show() < 0) {
		errsv = errno;
		log_crit("core_admin_ipc_key_change(): core_admin_ipc_key_show(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	return 0;
}


int core_admin_privdrop_group_show(void) {
	int errsv = 0;
	char *value = NULL, *value_tmp = NULL, *value_print = NULL;

	if (!(value_tmp = file_read_line_single(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_CORE "/." CONFIG_USCHED_FILE_CORE_PRIVDROP_GROUP))) {
		errsv = errno;
		log_crit("core_admin_privdrop_group_show(): file_read_line_single(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	if (!(value = file_read_line_single(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_CORE "/" CONFIG_USCHED_FILE_CORE_PRIVDROP_GROUP))) {
		errsv = errno;
		log_crit("core_admin_privdrop_group_show(): file_read_line_single(): %s\n", strerror(errno));
		mm_free(value_tmp);
		errno = errsv;
		return -1;
	}

	/* Check which value to print */
	if (!strcmp(value, value_tmp)) {
		if (!(value_print = mm_alloc(strlen(value) + 1))) {
			errsv = errno;
			log_crit("core_admin_privdrop_group_show(): mm_alloc(): %s\n", strerror(errno));
			mm_free(value);
			mm_free(value_tmp);
			errno = errsv;
			return -1;
		}

		strcpy(value_print, value);
	} else {
		if (!(value_print = mm_alloc(strlen(value_tmp) + 2))) {
			errsv = errno;
			log_crit("core_admin_privdrop_group_show(): mm_alloc(): %s\n", strerror(errno));
			mm_free(value);
			mm_free(value_tmp);
			errno = errsv;
			return -1;
		}

		/* Show the temporary value with a trailing '*' */
		strcpy(value_print, value_tmp);
		strcat(value_print, "*");
	}

	/* Print the output */
	print_admin_category_var_value(USCHED_CATEGORY_CORE_STR, CONFIG_USCHED_FILE_CORE_PRIVDROP_GROUP, value_print);

	/* Free memory */
	mm_free(value);
	mm_free(value_tmp);
	mm_free(value_print);

	/* All good */
	return 0;
}

int core_admin_privdrop_group_change(const char *privdrop_group) {
	int errsv = 0;

	if (file_write_line_single(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_CORE "/." CONFIG_USCHED_FILE_CORE_PRIVDROP_GROUP, privdrop_group) < 0) {
		errsv = errno;
		log_crit("core_admin_privdrop_group_change(): file_write_line_single(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	if (core_admin_privdrop_group_show() < 0) {
		errsv = errno;
		log_crit("core_admin_privdrop_group_change(): core_admin_privdrop_group_show(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	return 0;
}

int core_admin_privdrop_user_show(void) {
	int errsv = 0;
	char *value = NULL, *value_tmp = NULL, *value_print = NULL;

	if (!(value_tmp = file_read_line_single(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_CORE "/." CONFIG_USCHED_FILE_CORE_PRIVDROP_USER))) {
		errsv = errno;
		log_crit("core_admin_privdrop_user_show(): file_read_line_single(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	if (!(value = file_read_line_single(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_CORE "/" CONFIG_USCHED_FILE_CORE_PRIVDROP_USER))) {
		errsv = errno;
		log_crit("core_admin_privdrop_user_show(): file_read_line_single(): %s\n", strerror(errno));
		mm_free(value_tmp);
		errno = errsv;
		return -1;
	}

	/* Check which value to print */
	if (!strcmp(value, value_tmp)) {
		if (!(value_print = mm_alloc(strlen(value) + 1))) {
			errsv = errno;
			log_crit("core_admin_privdrop_user_show(): mm_alloc(): %s\n", strerror(errno));
			mm_free(value);
			mm_free(value_tmp);
			errno = errsv;
			return -1;
		}

		strcpy(value_print, value);
	} else {
		if (!(value_print = mm_alloc(strlen(value_tmp) + 2))) {
			errsv = errno;
			log_crit("core_admin_privdrop_user_show(): mm_alloc(): %s\n", strerror(errno));
			mm_free(value);
			mm_free(value_tmp);
			errno = errsv;
			return -1;
		}

		/* Show the temporary value with a trailing '*' */
		strcpy(value_print, value_tmp);
		strcat(value_print, "*");
	}

	/* Print the output */
	print_admin_category_var_value(USCHED_CATEGORY_CORE_STR, CONFIG_USCHED_FILE_CORE_PRIVDROP_USER, value_print);

	/* Free memory */
	mm_free(value);
	mm_free(value_tmp);
	mm_free(value_print);

	/* All good */
	return 0;
}

int core_admin_privdrop_user_change(const char *privdrop_user) {
	int errsv = 0;

	if (file_write_line_single(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_CORE "/." CONFIG_USCHED_FILE_CORE_PRIVDROP_USER, privdrop_user) < 0) {
		errsv = errno;
		log_crit("core_admin_privdrop_user_change(): file_write_line_single(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	if (core_admin_privdrop_user_show() < 0) {
		errsv = errno;
		log_crit("core_admin_privdrop_user_change(): core_admin_privdrop_user_show(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	return 0;
}

int core_admin_serialize_file_show(void) {
	int errsv = 0;
	char *value = NULL, *value_tmp = NULL, *value_print = NULL;

	if (!(value_tmp = file_read_line_single(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_CORE "/." CONFIG_USCHED_FILE_CORE_SERIALIZE_FILE))) {
		errsv = errno;
		log_crit("core_admin_serialize_file_show(): file_read_line_single(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	if (!(value = file_read_line_single(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_CORE "/" CONFIG_USCHED_FILE_CORE_SERIALIZE_FILE))) {
		errsv = errno;
		log_crit("core_admin_serialize_file_show(): file_read_line_single(): %s\n", strerror(errno));
		mm_free(value_tmp);
		errno = errsv;
		return -1;
	}

	/* Check which value to print */
	if (!strcmp(value, value_tmp)) {
		if (!(value_print = mm_alloc(strlen(value) + 1))) {
			errsv = errno;
			log_crit("core_admin_serialize_file_show(): mm_alloc(): %s\n", strerror(errno));
			mm_free(value);
			mm_free(value_tmp);
			errno = errsv;
			return -1;
		}

		strcpy(value_print, value);
	} else {
		if (!(value_print = mm_alloc(strlen(value_tmp) + 2))) {
			errsv = errno;
			log_crit("core_admin_serialize_file_show(): mm_alloc(): %s\n", strerror(errno));
			mm_free(value);
			mm_free(value_tmp);
			errno = errsv;
			return -1;
		}

		/* Show the temporary value with a trailing '*' */
		strcpy(value_print, value_tmp);
		strcat(value_print, "*");
	}

	/* Print the output */
	print_admin_category_var_value(USCHED_CATEGORY_CORE_STR, CONFIG_USCHED_FILE_CORE_SERIALIZE_FILE, value_print);

	/* Free memory */
	mm_free(value);
	mm_free(value_tmp);
	mm_free(value_print);

	/* All good */
	return 0;
}

int core_admin_serialize_file_change(const char *serialize_file) {
	int errsv = 0;

	if (file_write_line_single(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_CORE "/." CONFIG_USCHED_FILE_CORE_SERIALIZE_FILE, serialize_file) < 0) {
		errsv = errno;
		log_crit("core_admin_serialize_file_change(): file_write_line_single(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	if (core_admin_serialize_file_show() < 0) {
		errsv = errno;
		log_crit("core_admin_serialize_file_change(): core_admin_serialize_file_show(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	return 0;
}

int core_admin_thread_priority_show(void) {
	int errsv = 0;
	char *value = NULL, *value_tmp = NULL, *value_print = NULL;

	if (!(value_tmp = file_read_line_single(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_CORE "/." CONFIG_USCHED_FILE_CORE_THREAD_PRIORITY))) {
		errsv = errno;
		log_crit("core_admin_thread_priority_show(): file_read_line_single(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	if (!(value = file_read_line_single(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_CORE "/" CONFIG_USCHED_FILE_CORE_THREAD_PRIORITY))) {
		errsv = errno;
		log_crit("core_admin_thread_priority_show(): file_read_line_single(): %s\n", strerror(errno));
		mm_free(value_tmp);
		errno = errsv;
		return -1;
	}

	/* Check which value to print */
	if (!strcmp(value, value_tmp)) {
		if (!(value_print = mm_alloc(strlen(value) + 1))) {
			errsv = errno;
			log_crit("core_admin_thread_priority_show(): mm_alloc(): %s\n", strerror(errno));
			mm_free(value);
			mm_free(value_tmp);
			errno = errsv;
			return -1;
		}

		strcpy(value_print, value);
	} else {
		if (!(value_print = mm_alloc(strlen(value_tmp) + 2))) {
			errsv = errno;
			log_crit("core_admin_thread_priority_show(): mm_alloc(): %s\n", strerror(errno));
			mm_free(value);
			mm_free(value_tmp);
			errno = errsv;
			return -1;
		}

		/* Show the temporary value with a trailing '*' */
		strcpy(value_print, value_tmp);
		strcat(value_print, "*");
	}

	/* Print the output */
	print_admin_category_var_value(USCHED_CATEGORY_CORE_STR, CONFIG_USCHED_FILE_CORE_THREAD_PRIORITY, value_print);

	/* Free memory */
	mm_free(value);
	mm_free(value_tmp);
	mm_free(value_print);

	/* All good */
	return 0;
}

int core_admin_thread_priority_change(const char *thread_priority) {
	int errsv = 0;

	if (file_write_line_single(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_CORE "/." CONFIG_USCHED_FILE_CORE_THREAD_PRIORITY, thread_priority) < 0) {
		errsv = errno;
		log_crit("core_admin_thread_priority_change(): file_write_line_single(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	if (core_admin_thread_priority_show() < 0) {
		errsv = errno;
		log_crit("core_admin_thread_priority_change(): core_admin_thread_priority_show(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	return 0;
}

int core_admin_thread_workers_show(void) {
	int errsv = 0;
	char *value = NULL, *value_tmp = NULL, *value_print = NULL;

	if (!(value_tmp = file_read_line_single(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_CORE "/." CONFIG_USCHED_FILE_CORE_THREAD_WORKERS))) {
		errsv = errno;
		log_crit("core_admin_thread_workers_show(): file_read_line_single(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	if (!(value = file_read_line_single(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_CORE "/" CONFIG_USCHED_FILE_CORE_THREAD_WORKERS))) {
		errsv = errno;
		log_crit("core_admin_thread_workers_show(): file_read_line_single(): %s\n", strerror(errno));
		mm_free(value_tmp);
		errno = errsv;
		return -1;
	}

	/* Check which value to print */
	if (!strcmp(value, value_tmp)) {
		if (!(value_print = mm_alloc(strlen(value) + 1))) {
			errsv = errno;
			log_crit("core_admin_thread_workers_show(): mm_alloc(): %s\n", strerror(errno));
			mm_free(value);
			mm_free(value_tmp);
			errno = errsv;
			return -1;
		}

		strcpy(value_print, value);
	} else {
		if (!(value_print = mm_alloc(strlen(value_tmp) + 2))) {
			errsv = errno;
			log_crit("core_admin_thread_workers_show(): mm_alloc(): %s\n", strerror(errno));
			mm_free(value);
			mm_free(value_tmp);
			errno = errsv;
			return -1;
		}

		/* Show the temporary value with a trailing '*' */
		strcpy(value_print, value_tmp);
		strcat(value_print, "*");
	}

	/* Print the output */
	print_admin_category_var_value(USCHED_CATEGORY_CORE_STR, CONFIG_USCHED_FILE_CORE_THREAD_WORKERS, value_print);

	/* Free memory */
	mm_free(value);
	mm_free(value_tmp);
	mm_free(value_print);

	/* All good */
	return 0;
}

int core_admin_thread_workers_change(const char *thread_workers) {
	int errsv = 0;

	if (file_write_line_single(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_CORE "/." CONFIG_USCHED_FILE_CORE_THREAD_WORKERS, thread_workers) < 0) {
		errsv = errno;
		log_crit("core_admin_thread_workers_change(): file_write_line_single(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	if (core_admin_thread_workers_show() < 0) {
		errsv = errno;
		log_crit("core_admin_thread_workers_change(): core_admin_thread_workers_show(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	return 0;
}

