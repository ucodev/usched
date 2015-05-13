/**
 * @file ipc.h
 * @brief uSched
 *        Inter-Process Communication interface header
 *
 * Date: 13-05-2015
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

#include <stdio.h>
#include <stdint.h>
#include <time.h>

#include <pipc/pipc.h>

#include "config.h"

/* IPC module IDs */
enum IPC_MODULE_ID {
	IPC_USI_ID = 1,
	IPC_USD_ID,
	IPC_USE_ID,
	IPC_USS_ID
};

/* Structures */
struct ipc_usd_hdr {
	uint64_t id;
	uint64_t exec_time;	/* In nanoseconds */
	uint64_t latency;	/* In nanoseconds */
	uint32_t pid;
	uint32_t status;
	uint32_t outdata_len;
};

struct ipc_use_hdr {
	uint64_t id;		/* Entry ID */
	uint32_t uid;		/* Entry UID */
	uint32_t gid;		/* Entry GID */
	uint32_t trigger;	/* Entry Trigger */
	uint32_t cmd_len;	/* Command length */
};

struct ipc_uss_hdr {
	uint64_t id;
	uint32_t uid;
	uint32_t gid;
	uint32_t pid;
	uint32_t status;
	struct timespec t_trigger;
	struct timespec t_start;
	struct timespec t_end;
	uint32_t outdata_len;
};

/* Prototypes */
ssize_t ipc_send(pipcd_t *pipcd, long src_id, long dst_id, const char *msg, size_t count);
ssize_t ipc_send_nowait(pipcd_t *pipcd, long src_id, long dst_id, const char *msg, size_t count);
ssize_t ipc_recv(pipcd_t *pipcd, long *src_id, long *dst_id, char *msg, size_t count);
ssize_t ipc_recv_nowait(pipcd_t *pipcd, long *src_id, long *dst_id, char *msg, size_t count);
int ipc_pending(pipcd_t *pipcd);
void ipc_close(pipcd_t *pipcd);
int ipc_daemon_init(void);
int ipc_exec_init(void);
int ipc_stat_init(void);
int ipc_ipc_init(void);
void ipc_daemon_destroy(void);
void ipc_exec_destroy(void);
void ipc_stat_destroy(void);
void ipc_ipc_destroy(void);
int ipc_admin_commit(void);
int ipc_admin_rollback(void);
int ipc_admin_show(void);
int ipc_admin_auth_key_show(void);
int ipc_admin_auth_key_change(const char *auth_key);
int ipc_admin_id_key_change(const char *id_key);
int ipc_admin_id_key_show(void);
int ipc_admin_id_name_change(const char *id_key);
int ipc_admin_id_name_show(void);
int ipc_admin_msg_max_show(void);
int ipc_admin_msg_max_change(const char *msg_max);
int ipc_admin_msg_size_show(void);
int ipc_admin_msg_size_change(const char *msg_size);
int ipc_admin_jail_dir_show(void);
int ipc_admin_jail_dir_change(const char *jail_dir);
int ipc_admin_privdrop_user_show(void);
int ipc_admin_privdrop_user_change(const char *privdrop_user);
int ipc_admin_privdrop_group_show(void);
int ipc_admin_privdrop_group_change(const char *privdrop_group);

#endif

