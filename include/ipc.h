/**
 * @file ipc.h
 * @brief uSched
 *        Inter-Process Communication interface header
 *
 * Date: 05-04-2015
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

#include "config.h"

#if CONFIG_USE_IPC_PMQ == 1
 #include <mqueue.h>
 typedef mqd_t ipcd_t;
#elif CONFIG_USE_IPC_UNIX == 1 || CONFIG_USE_IPC_INET == 1
 #include <panet/panet.h>
 typedef sock_t ipcd_t;
#endif

/* Structures */
struct ipc_usd_hdr {
	uint64_t id;
	uint64_t exec_time;	/* In nanoseconds */
	uint64_t latency;	/* In nanoseconds */
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
	struct timespec t_start;
	struct timespec t_end;
	uint32_t status;
	uint32_t pid;
	uint32_t outdata_len;
};

/* Prototypes */
int ipc_timedsend(ipcd_t ipcd, const char *msg, size_t count, const struct timespec *timeout);
int ipc_recv(ipcd_t ipcd, char *msg, size_t count);
size_t ipc_pending(ipcd_t ipcd);
void ipc_close(ipcd_t ipcd);
int ipc_daemon_init(void);
int ipc_exec_init(void);
int ipc_stat_init(void);
void ipc_daemon_destroy(void);
void ipc_exec_destroy(void);
void ipc_stat_destroy(void);
int ipc_admin_create(void);
int ipc_admin_delete(void);

#endif

