/**
 * @file stat.h
 * @brief uSched
 *        Stat configuration and administration interface header
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

#ifndef USCHED_STAT_H
#define USCHED_STAT_H

/* Prototypes */
int stat_admin_commit(void);
int stat_admin_rollback(void);
int stat_admin_show(void);
int stat_admin_jail_dir_show(void);
int stat_admin_jail_dir_change(const char *jail_dir);
int stat_admin_ipc_msgmax_show(void);
int stat_admin_ipc_msgmax_change(const char *ipc_msgmax);
int stat_admin_ipc_msgsize_show(void);
int stat_admin_ipc_msgsize_change(const char *ipc_msgsize);
int stat_admin_ipc_name_show(void);
int stat_admin_ipc_name_change(const char *ipc_name);
int stat_admin_ipc_key_show(void);
int stat_admin_ipc_key_change(const char *ipc_name);
int stat_admin_privdrop_group_show(void);
int stat_admin_privdrop_group_change(const char *privdrop_group);
int stat_admin_privdrop_user_show(void);
int stat_admin_privdrop_user_change(const char *privdrop_user);

#endif

