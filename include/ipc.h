/**
 * @file ipc.h
 * @brief uSched
 *        Inter-Process Communication interface header
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


#ifndef USCHED_IPC_H
#define USCHED_IPC_H

/* Prototypes */
int ipc_daemon_init(void);
int ipc_exec_init(void);
void ipc_daemon_destroy(void);
void ipc_exec_destroy(void);
int ipc_admin_create(void);
int ipc_admin_delete(void);

#endif

