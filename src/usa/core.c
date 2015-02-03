/**
 * @file core.c
 * @brief uSched
 *        Core configuration and administration interface
 *
 * Date: 03-02-2015
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

#include "config.h"
#include "core.h"


int core_admin_delta_noexec_show(void) {
	return -1;
}

int core_admin_delta_noexec_change(const char *delta_noexec) {
	return -1;
}

int core_admin_delta_reload_show(void) {
	return -1;
}

int core_admin_delta_reload_change(const char *delta_reload) {
	return -1;
}

int core_admin_serialize_file_show(void) {
	return -1;
}

int core_admin_serialize_file_change(const char *file_serialize) {
	return -1;
}

int core_admin_jail_dir_show(void) {
	return -1;
}

int core_admin_jail_dir_change(const char *jail_dir) {
	return -1;
}

int core_admin_pmq_msgmax_show(void) {
	return -1;
}

int core_admin_pmq_msgmax_change(const char *pmq_msgmax) {
	return -1;
}

int core_admin_pmq_msgsize_show(void) {
	return -1;
}

int core_admin_pmq_msgsize_change(const char *pmq_msgsize) {
	return -1;
}

int core_admin_pmq_name_show(void) {
	return -1;
}

int core_admin_pmq_name_change(const char *pmq_name) {
	return -1;
}

int core_admin_privdrop_group_show(void) {
	return -1;
}

int core_admin_privdrop_group_change(const char *privdrop_group) {
	return -1;
}

int core_admin_privdrop_user_show(void) {
	return -1;
}

int core_admin_privdrop_user_change(const char *privdrop_user) {
	return -1;
}

int core_admin_thread_priority_show(void) {
	return -1;
}

int core_admin_thread_priority_change(const char *thread_priority) {
	return -1;
}

int core_admin_thread_workers_show(void) {
	return -1;
}

int core_admin_thread_workers_change(const char *thread_workers) {
	return -1;
}

