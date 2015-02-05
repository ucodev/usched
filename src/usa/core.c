/**
 * @file core.c
 * @brief uSched
 *        Core configuration and administration interface
 *
 * Date: 05-02-2015
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
#include "core.h"
#include "file.h"
#include "log.h"
#include "mm.h"
#include "usched.h"
#include "print.h"


void core_admin_show(void) {
	core_admin_delta_noexec_show();
	core_admin_delta_reload_show();
	core_admin_jail_dir_show();
	core_admin_pmq_msgmax_show();
	core_admin_pmq_msgsize_show();
	core_admin_pmq_name_show();
	core_admin_privdrop_group_show();
	core_admin_privdrop_user_show();
	core_admin_serialize_file_show();
	core_admin_thread_priority_show();
	core_admin_thread_workers_show();
}

int core_admin_delta_noexec_show(void) {
	int errsv = 0;
	char *value = NULL;

	if (!(value = file_read_line_single(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_CORE "/" CONFIG_USCHED_FILE_CORE_DELTA_NOEXEC))) {
		errsv = errno;
		log_crit("core_admin_delta_noexec_show(): file_read_line_single(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	print_admin_category_var_value(USCHED_CATEGORY_CORE_STR, CONFIG_USCHED_FILE_CORE_DELTA_NOEXEC, value);

	mm_free(value);

	return 0;
}

int core_admin_delta_noexec_change(const char *delta_noexec) {
	int errsv = 0;

	if (file_write_line_single(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_CORE "/" CONFIG_USCHED_FILE_CORE_DELTA_NOEXEC, delta_noexec) < 0) {
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
	char *value = NULL;

	if (!(value = file_read_line_single(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_CORE "/" CONFIG_USCHED_FILE_CORE_DELTA_RELOAD))) {
		errsv = errno;
		log_crit("core_admin_delta_reload_show(): file_read_line_single(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	print_admin_category_var_value(USCHED_CATEGORY_CORE_STR, CONFIG_USCHED_FILE_CORE_DELTA_RELOAD, value);

	mm_free(value);

	return 0;
}

int core_admin_delta_reload_change(const char *delta_reload) {
	int errsv = 0;

	if (file_write_line_single(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_CORE "/" CONFIG_USCHED_FILE_CORE_DELTA_RELOAD, delta_reload) < 0) {
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
	char *value = NULL;

	if (!(value = file_read_line_single(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_CORE "/" CONFIG_USCHED_FILE_CORE_JAIL_DIR))) {
		errsv = errno;
		log_crit("core_admin_jail_dir_show(): file_read_line_single(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	print_admin_category_var_value(USCHED_CATEGORY_CORE_STR, CONFIG_USCHED_FILE_CORE_JAIL_DIR, value);

	mm_free(value);

	return 0;
}

int core_admin_jail_dir_change(const char *jail_dir) {
	int errsv = 0;

	if (file_write_line_single(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_CORE "/" CONFIG_USCHED_FILE_CORE_JAIL_DIR, jail_dir) < 0) {
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

int core_admin_pmq_msgmax_show(void) {
	int errsv = 0;
	char *value = NULL;

	if (!(value = file_read_line_single(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_CORE "/" CONFIG_USCHED_FILE_CORE_PMQ_MSGMAX))) {
		errsv = errno;
		log_crit("core_admin_pmq_msgmax_show(): file_read_line_single(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	print_admin_category_var_value(USCHED_CATEGORY_CORE_STR, CONFIG_USCHED_FILE_CORE_PMQ_MSGMAX, value);

	mm_free(value);

	return 0;
}

int core_admin_pmq_msgmax_change(const char *pmq_msgmax) {
	int errsv = 0;

	if (file_write_line_single(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_CORE "/" CONFIG_USCHED_FILE_CORE_PMQ_MSGMAX, pmq_msgmax) < 0) {
		errsv = errno;
		log_crit("core_admin_pmq_msgmax_change(): file_write_line_single(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	if (core_admin_pmq_msgmax_show() < 0) {
		errsv = errno;
		log_crit("core_admin_pmq_msgmax_change(): core_admin_pmq_msgmax_show(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	return 0;
}

int core_admin_pmq_msgsize_show(void) {
	int errsv = 0;
	char *value = NULL;

	if (!(value = file_read_line_single(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_CORE "/" CONFIG_USCHED_FILE_CORE_PMQ_MSGSIZE))) {
		errsv = errno;
		log_crit("core_admin_pmq_msgsize_show(): file_read_line_single(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	print_admin_category_var_value(USCHED_CATEGORY_CORE_STR, CONFIG_USCHED_FILE_CORE_PMQ_MSGSIZE, value);

	mm_free(value);

	return 0;
}

int core_admin_pmq_msgsize_change(const char *pmq_msgsize) {
	int errsv = 0;

	if (file_write_line_single(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_CORE "/" CONFIG_USCHED_FILE_CORE_PMQ_MSGSIZE, pmq_msgsize) < 0) {
		errsv = errno;
		log_crit("core_admin_pmq_msgsize_change(): file_write_line_single(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	if (core_admin_pmq_msgsize_show() < 0) {
		errsv = errno;
		log_crit("core_admin_pmq_msgsize_change(): core_admin_pmq_msgsize_show(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	return 0;
}

int core_admin_pmq_name_show(void) {
	int errsv = 0;
	char *value = NULL;

	if (!(value = file_read_line_single(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_CORE "/" CONFIG_USCHED_FILE_CORE_PMQ_NAME))) {
		errsv = errno;
		log_crit("core_admin_pmq_name_show(): file_read_line_single(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	print_admin_category_var_value(USCHED_CATEGORY_CORE_STR, CONFIG_USCHED_FILE_CORE_PMQ_NAME, value);

	mm_free(value);

	return 0;
}

int core_admin_pmq_name_change(const char *pmq_name) {
	int errsv = 0;

	if (file_write_line_single(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_CORE "/" CONFIG_USCHED_FILE_CORE_PMQ_NAME, pmq_name) < 0) {
		errsv = errno;
		log_crit("core_admin_pmq_name_change(): file_write_line_single(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	if (core_admin_pmq_name_show() < 0) {
		errsv = errno;
		log_crit("core_admin_pmq_name_change(): core_admin_pmq_name_show(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	return 0;
}

int core_admin_privdrop_group_show(void) {
	int errsv = 0;
	char *value = NULL;

	if (!(value = file_read_line_single(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_CORE "/" CONFIG_USCHED_FILE_CORE_PRIVDROP_GROUP))) {
		errsv = errno;
		log_crit("core_admin_privdrop_group_show(): file_read_line_single(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	print_admin_category_var_value(USCHED_CATEGORY_CORE_STR, CONFIG_USCHED_FILE_CORE_PRIVDROP_GROUP, value);

	mm_free(value);

	return 0;
}

int core_admin_privdrop_group_change(const char *privdrop_group) {
	int errsv = 0;

	if (file_write_line_single(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_CORE "/" CONFIG_USCHED_FILE_CORE_PRIVDROP_GROUP, privdrop_group) < 0) {
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
	char *value = NULL;

	if (!(value = file_read_line_single(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_CORE "/" CONFIG_USCHED_FILE_CORE_PRIVDROP_USER))) {
		errsv = errno;
		log_crit("core_admin_privdrop_user_show(): file_read_line_single(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	print_admin_category_var_value(USCHED_CATEGORY_CORE_STR, CONFIG_USCHED_FILE_CORE_PRIVDROP_USER, value);

	mm_free(value);

	return 0;
}

int core_admin_privdrop_user_change(const char *privdrop_user) {
	int errsv = 0;

	if (file_write_line_single(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_CORE "/" CONFIG_USCHED_FILE_CORE_PRIVDROP_USER, privdrop_user) < 0) {
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
	char *value = NULL;

	if (!(value = file_read_line_single(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_CORE "/" CONFIG_USCHED_FILE_CORE_SERIALIZE_FILE))) {
		errsv = errno;
		log_crit("core_admin_serialize_file_show(): file_read_line_single(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	print_admin_category_var_value(USCHED_CATEGORY_CORE_STR, CONFIG_USCHED_FILE_CORE_SERIALIZE_FILE, value);

	mm_free(value);

	return 0;
}

int core_admin_serialize_file_change(const char *serialize_file) {
	int errsv = 0;

	if (file_write_line_single(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_CORE "/" CONFIG_USCHED_FILE_CORE_SERIALIZE_FILE, serialize_file) < 0) {
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
	char *value = NULL;

	if (!(value = file_read_line_single(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_CORE "/" CONFIG_USCHED_FILE_CORE_THREAD_PRIORITY))) {
		errsv = errno;
		log_crit("core_admin_thread_priority_show(): file_read_line_single(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	print_admin_category_var_value(USCHED_CATEGORY_CORE_STR, CONFIG_USCHED_FILE_CORE_THREAD_PRIORITY, value);

	mm_free(value);

	return 0;
}

int core_admin_thread_priority_change(const char *thread_priority) {
	int errsv = 0;

	if (file_write_line_single(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_CORE "/" CONFIG_USCHED_FILE_CORE_THREAD_PRIORITY, thread_priority) < 0) {
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
	char *value = NULL;

	if (!(value = file_read_line_single(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_CORE "/" CONFIG_USCHED_FILE_CORE_THREAD_WORKERS))) {
		errsv = errno;
		log_crit("core_admin_thread_workers_show(): file_read_line_single(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	print_admin_category_var_value(USCHED_CATEGORY_CORE_STR, CONFIG_USCHED_FILE_CORE_THREAD_WORKERS, value);

	mm_free(value);

	return 0;
}

int core_admin_thread_workers_change(const char *thread_workers) {
	int errsv = 0;

	if (file_write_line_single(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_CORE "/" CONFIG_USCHED_FILE_CORE_THREAD_WORKERS, thread_workers) < 0) {
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

