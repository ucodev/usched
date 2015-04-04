/**
 * @file exec.h
 * @brief uSched
 *        Exec configuration and administration interface header
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

#ifndef USCHED_EXEC_H
#define USCHED_EXEC_H

/* Prototypes */
int exec_admin_commit(void);
int exec_admin_rollback(void);
int exec_admin_show(void);
int exec_admin_ipc_msgmax_show(void);
int exec_admin_ipc_msgmax_change(const char *ipc_msgmax);
int exec_admin_ipc_msgsize_show(void);
int exec_admin_ipc_msgsize_change(const char *ipc_msgsize);
int exec_admin_ipc_name_show(void);
int exec_admin_ipc_name_change(const char *ipc_name);
int exec_admin_ipc_key_show(void);
int exec_admin_ipc_key_change(const char *ipc_name);

#endif

