/**
 * @file exec.c
 * @brief uSched
 *        Exec configuration and administration interface
 *
 * Date: 01-04-2015
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
#include "exec.h"
#include "file.h"
#include "log.h"
#include "mm.h"
#include "usched.h"
#include "print.h"
#include "ipc.h"

int exec_admin_commit(void) {
	int errsv = 0;

	/* Check if services are running and stop processing if they are */
	if (fsop_path_exists(CONFIG_USCHED_DAEMON_PID_FILE)) {
		log_crit("exec_admin_commit(): uSched services are running (usd). You must stop all uSched services before you can commit exec configuration changes.\n");
		errno = EBUSY;
		return -1;
	}

	if (fsop_path_exists(CONFIG_USCHED_EXEC_PID_FILE)) {
		log_crit("exec_admin_commit(): uSched services are running (use). You must stop all uSched services before you can commit exec configuration changes.\n");
		errno = EBUSY;
		return -1;
	}

	if (fsop_path_exists(CONFIG_USCHED_EXEC_PID_FILE)) {
		log_crit("exec_admin_commit(): uSched services are running (uss). You must stop all uSched services before you can commit exec configuration changes.\n");
		errno = EBUSY;
		return -1;
	}

	/* Destroy the current message queue */
	ipc_admin_delete();

	/* Destroy the current configuration */
	config_admin_destroy();

	/* ipc.msgmax */
	if (fsop_cp(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_EXEC "/." CONFIG_USCHED_FILE_EXEC_IPC_MSGMAX, CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_EXEC "/" CONFIG_USCHED_FILE_EXEC_IPC_MSGMAX, 128) < 0) {
		errsv = errno;
		log_crit("exec_admin_commit(): fsop_cp(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* ipc.msgsize */
	if (fsop_cp(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_EXEC "/." CONFIG_USCHED_FILE_EXEC_IPC_MSGSIZE, CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_EXEC "/" CONFIG_USCHED_FILE_EXEC_IPC_MSGSIZE, 128) < 0) {
		errsv = errno;
		log_crit("exec_admin_commit(): fsop_cp(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* ipc.name */
	if (fsop_cp(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_EXEC "/." CONFIG_USCHED_FILE_EXEC_IPC_NAME, CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_EXEC "/" CONFIG_USCHED_FILE_EXEC_IPC_NAME, 128) < 0) {
		errsv = errno;
		log_crit("exec_admin_commit(): fsop_cp(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* ipc.key */
	if (fsop_cp(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_EXEC "/." CONFIG_USCHED_FILE_EXEC_IPC_KEY, CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_EXEC "/" CONFIG_USCHED_FILE_EXEC_IPC_KEY, 128) < 0) {
		errsv = errno;
		log_crit("exec_admin_commit(): fsop_cp(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Re-initialize the configuration */
	if (config_admin_init() < 0) {
		errsv = errno;
		log_crit("exec_admin_commit(): config_admin_init(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Create the message queue */
	if (ipc_admin_create() < 0) {
		errsv = errno;
		log_crit("exec_admin_commit(): ipc_admin_create(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* All good */
	return 0;
}

int exec_admin_rollback(void) {
	int errsv = 0;

	/* ipc.msgmax */
	if (fsop_cp(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_EXEC "/" CONFIG_USCHED_FILE_EXEC_IPC_MSGMAX, CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_EXEC "/." CONFIG_USCHED_FILE_EXEC_IPC_MSGMAX, 128) < 0) {
		errsv = errno;
		log_crit("exec_admin_rollback(): fsop_cp(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* ipc.msgsize */
	if (fsop_cp(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_EXEC "/" CONFIG_USCHED_FILE_EXEC_IPC_MSGSIZE, CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_EXEC "/." CONFIG_USCHED_FILE_EXEC_IPC_MSGSIZE, 128) < 0) {
		errsv = errno;
		log_crit("exec_admin_rollback(): fsop_cp(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* ipc.name */
	if (fsop_cp(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_EXEC "/" CONFIG_USCHED_FILE_EXEC_IPC_NAME, CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_EXEC "/." CONFIG_USCHED_FILE_EXEC_IPC_NAME, 128) < 0) {
		errsv = errno;
		log_crit("exec_admin_rollback(): fsop_cp(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* ipc.key */
	if (fsop_cp(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_EXEC "/" CONFIG_USCHED_FILE_EXEC_IPC_KEY, CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_EXEC "/." CONFIG_USCHED_FILE_EXEC_IPC_KEY, 128) < 0) {
		errsv = errno;
		log_crit("exec_admin_rollback(): fsop_cp(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* All good */
	return 0;
}

int exec_admin_show(void) {
	int errsv = 0;

	if (exec_admin_ipc_msgmax_show() < 0) {
		errsv = errno;
		log_crit("exec_admin_show(): exec_admin_ipc_msgmax_show(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	if (exec_admin_ipc_msgsize_show() < 0) {
		errsv = errno;
		log_crit("exec_admin_show(): exec_admin_ipc_msgsize_show(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	if (exec_admin_ipc_name_show() < 0) {
		errsv = errno;
		log_crit("exec_admin_show(): exec_admin_ipc_name_show(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	if (exec_admin_ipc_key_show() < 0) {
		errsv = errno;
		log_crit("exec_admin_show(): exec_admin_ipc_key_show(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	return 0;
}

int exec_admin_ipc_msgmax_show(void) {
	int errsv = 0;
	char *value = NULL, *value_tmp = NULL, *value_print = NULL;

	if (!(value_tmp = file_read_line_single(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_EXEC "/." CONFIG_USCHED_FILE_EXEC_IPC_MSGMAX))) {
		errsv = errno;
		log_crit("exec_admin_ipc_msgmax_show(): file_read_line_single(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	if (!(value = file_read_line_single(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_EXEC "/" CONFIG_USCHED_FILE_EXEC_IPC_MSGMAX))) {
		errsv = errno;
		log_crit("exec_admin_ipc_msgmax_show(): file_read_line_single(): %s\n", strerror(errno));
		mm_free(value_tmp);
		errno = errsv;
		return -1;
	}

	/* Check which value to print */
	if (!strcmp(value, value_tmp)) {
		if (!(value_print = mm_alloc(strlen(value) + 1))) {
			errsv = errno;
			log_crit("exec_admin_ipc_msgmax_show(): mm_alloc(): %s\n", strerror(errno));
			mm_free(value);
			mm_free(value_tmp);
			errno = errsv;
			return -1;
		}

		strcpy(value_print, value);
	} else {
		if (!(value_print = mm_alloc(strlen(value_tmp) + 2))) {
			errsv = errno;
			log_crit("exec_admin_ipc_msgmax_show(): mm_alloc(): %s\n", strerror(errno));
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
	print_admin_category_var_value(USCHED_CATEGORY_EXEC_STR, CONFIG_USCHED_FILE_EXEC_IPC_MSGMAX, value_print);

	/* Free memory */
	mm_free(value);
	mm_free(value_tmp);
	mm_free(value_print);

	/* All good */
	return 0;
}

int exec_admin_ipc_msgmax_change(const char *ipc_msgmax) {
	int errsv = 0;

	if (file_write_line_single(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_EXEC "/." CONFIG_USCHED_FILE_EXEC_IPC_MSGMAX, ipc_msgmax) < 0) {
		errsv = errno;
		log_crit("exec_admin_ipc_msgmax_change(): file_write_line_single(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	if (exec_admin_ipc_msgmax_show() < 0) {
		errsv = errno;
		log_crit("exec_admin_ipc_msgmax_change(): exec_admin_ipc_msgmax_show(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	return 0;
}

int exec_admin_ipc_msgsize_show(void) {
	int errsv = 0;
	char *value = NULL, *value_tmp = NULL, *value_print = NULL;

	if (!(value_tmp = file_read_line_single(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_EXEC "/." CONFIG_USCHED_FILE_EXEC_IPC_MSGSIZE))) {
		errsv = errno;
		log_crit("exec_admin_ipc_msgsize_show(): file_read_line_single(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	if (!(value = file_read_line_single(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_EXEC "/" CONFIG_USCHED_FILE_EXEC_IPC_MSGSIZE))) {
		errsv = errno;
		log_crit("exec_admin_ipc_msgsize_show(): file_read_line_single(): %s\n", strerror(errno));
		mm_free(value_tmp);
		errno = errsv;
		return -1;
	}

	/* Check which value to print */
	if (!strcmp(value, value_tmp)) {
		if (!(value_print = mm_alloc(strlen(value) + 1))) {
			errsv = errno;
			log_crit("exec_admin_ipc_msgsize_show(): mm_alloc(): %s\n", strerror(errno));
			mm_free(value);
			mm_free(value_tmp);
			errno = errsv;
			return -1;
		}

		strcpy(value_print, value);
	} else {
		if (!(value_print = mm_alloc(strlen(value_tmp) + 2))) {
			errsv = errno;
			log_crit("exec_admin_ipc_msgsize_show(): mm_alloc(): %s\n", strerror(errno));
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
	print_admin_category_var_value(USCHED_CATEGORY_EXEC_STR, CONFIG_USCHED_FILE_EXEC_IPC_MSGSIZE, value_print);

	/* Free memory */
	mm_free(value);
	mm_free(value_tmp);
	mm_free(value_print);

	/* All good */
	return 0;
}

int exec_admin_ipc_msgsize_change(const char *ipc_msgsize) {
	int errsv = 0;

	if (file_write_line_single(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_EXEC "/." CONFIG_USCHED_FILE_EXEC_IPC_MSGSIZE, ipc_msgsize) < 0) {
		errsv = errno;
		log_crit("exec_admin_ipc_msgsize_change(): file_write_line_single(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	if (exec_admin_ipc_msgsize_show() < 0) {
		errsv = errno;
		log_crit("exec_admin_ipc_msgsize_change(): exec_admin_ipc_msgsize_show(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	return 0;
}

int exec_admin_ipc_name_show(void) {
	int errsv = 0;
	char *value = NULL, *value_tmp = NULL, *value_print = NULL;

	if (!(value_tmp = file_read_line_single(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_EXEC "/." CONFIG_USCHED_FILE_EXEC_IPC_NAME))) {
		errsv = errno;
		log_crit("exec_admin_ipc_name_show(): file_read_line_single(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	if (!(value = file_read_line_single(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_EXEC "/" CONFIG_USCHED_FILE_EXEC_IPC_NAME))) {
		errsv = errno;
		log_crit("exec_admin_ipc_name_show(): file_read_line_single(): %s\n", strerror(errno));
		mm_free(value_tmp);
		errno = errsv;
		return -1;
	}

	/* Check which value to print */
	if (!strcmp(value, value_tmp)) {
		if (!(value_print = mm_alloc(strlen(value) + 1))) {
			errsv = errno;
			log_crit("exec_admin_ipc_name_show(): mm_alloc(): %s\n", strerror(errno));
			mm_free(value);
			mm_free(value_tmp);
			errno = errsv;
			return -1;
		}

		strcpy(value_print, value);
	} else {
		if (!(value_print = mm_alloc(strlen(value_tmp) + 2))) {
			errsv = errno;
			log_crit("exec_admin_ipc_name_show(): mm_alloc(): %s\n", strerror(errno));
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
	print_admin_category_var_value(USCHED_CATEGORY_EXEC_STR, CONFIG_USCHED_FILE_EXEC_IPC_NAME, value_print);

	/* Free memory */
	mm_free(value);
	mm_free(value_tmp);
	mm_free(value_print);

	/* All good */
	return 0;
}

int exec_admin_ipc_name_change(const char *ipc_name) {
	int errsv = 0;

	if (file_write_line_single(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_EXEC "/." CONFIG_USCHED_FILE_EXEC_IPC_NAME, ipc_name) < 0) {
		errsv = errno;
		log_crit("exec_admin_ipc_name_change(): file_write_line_single(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	if (exec_admin_ipc_name_show() < 0) {
		errsv = errno;
		log_crit("exec_admin_ipc_name_change(): exec_admin_ipc_name_show(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	return 0;
}

int exec_admin_ipc_key_show(void) {
	int errsv = 0;
	char *value = NULL, *value_tmp = NULL, *value_print = NULL;

	if (!(value_tmp = file_read_line_single(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_EXEC "/." CONFIG_USCHED_FILE_EXEC_IPC_KEY))) {
		errsv = errno;
		log_crit("exec_admin_ipc_key_show(): file_read_line_single(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	if (!(value = file_read_line_single(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_EXEC "/" CONFIG_USCHED_FILE_EXEC_IPC_KEY))) {
		errsv = errno;
		log_crit("exec_admin_ipc_key_show(): file_read_line_single(): %s\n", strerror(errno));
		mm_free(value_tmp);
		errno = errsv;
		return -1;
	}

	/* Check which value to print */
	if (!strcmp(value, value_tmp)) {
		if (!(value_print = mm_alloc(strlen(value) + 1))) {
			errsv = errno;
			log_crit("exec_admin_ipc_key_show(): mm_alloc(): %s\n", strerror(errno));
			mm_free(value);
			mm_free(value_tmp);
			errno = errsv;
			return -1;
		}

		strcpy(value_print, value);
	} else {
		if (!(value_print = mm_alloc(strlen(value_tmp) + 2))) {
			errsv = errno;
			log_crit("exec_admin_ipc_key_show(): mm_alloc(): %s\n", strerror(errno));
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
	print_admin_category_var_value(USCHED_CATEGORY_EXEC_STR, CONFIG_USCHED_FILE_EXEC_IPC_KEY, value_print);

	/* Free memory */
	mm_free(value);
	mm_free(value_tmp);
	mm_free(value_print);

	/* All good */
	return 0;
}

int exec_admin_ipc_key_change(const char *ipc_key) {
	int errsv = 0;

	if (file_write_line_single(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_EXEC "/." CONFIG_USCHED_FILE_EXEC_IPC_KEY, ipc_key) < 0) {
		errsv = errno;
		log_crit("exec_admin_ipc_key_change(): file_write_line_single(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	if (exec_admin_ipc_key_show() < 0) {
		errsv = errno;
		log_crit("exec_admin_ipc_key_change(): exec_admin_ipc_key_show(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	return 0;
}

