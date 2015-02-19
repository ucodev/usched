/**
 * @file core.h
 * @brief uSched
 *        Core configuration and administration interface header
 *
 * Date: 19-02-2015
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

#ifndef USCHED_CORE_H
#define USCHED_CORE_H

/* Prototypes */
int core_admin_commit(void);
int core_admin_rollback(void);
void core_admin_show(void);
int core_admin_delta_noexec_show(void);
int core_admin_delta_noexec_change(const char *delta_noexec);
int core_admin_delta_reload_show(void);
int core_admin_delta_reload_change(const char *delta_reload);
int core_admin_jail_dir_show(void);
int core_admin_jail_dir_change(const char *jail_dir);
int core_admin_pmq_msgmax_show(void);
int core_admin_pmq_msgmax_change(const char *pmq_msgmax);
int core_admin_pmq_msgsize_show(void);
int core_admin_pmq_msgsize_change(const char *pmq_msgsize);
int core_admin_pmq_name_show(void);
int core_admin_pmq_name_change(const char *pmq_name);
int core_admin_privdrop_group_show(void);
int core_admin_privdrop_group_change(const char *privdrop_group);
int core_admin_privdrop_user_show(void);
int core_admin_privdrop_user_change(const char *privdrop_user);
int core_admin_serialize_file_show(void);
int core_admin_serialize_file_change(const char *serialize_file);
int core_admin_thread_priority_show(void);
int core_admin_thread_priority_change(const char *thread_priority);
int core_admin_thread_workers_show(void);
int core_admin_thread_workers_change(const char *thread_workers);

#endif

