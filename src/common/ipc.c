/**
 * @file ipc.c
 * @brief uSched
 *        Inter-Process Communication interface - Common
 *
 * Date: 12-05-2015
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

#include <pipc/pipc.h>

#include "config.h"
#include "ipc.h"
#include "log.h"

ssize_t ipc_send(pipcd_t pipcd, long src_id, long dst_id, const char *msg, size_t count) {
	return pipc_send(pipcd, src_id, dst_id, msg, count);
}

ssize_t ipc_send_nowait(pipcd_t pipcd, long src_id, long dst_id, const char *msg, size_t count) {
	return pipc_send_nowait(pipcd, src_id, dst_id, msg, count);
}

ssize_t ipc_recv(pipcd_t pipcd, long *src_id, long *dst_id, char *msg, size_t count) {
	return pipc_recv(pipcd, src_id, dst_id, msg, count);
}

ssize_t ipc_recv_nowait(pipcd_t pipcd, long *src_id, long *dst_id, char *msg, size_t count) {
	return pipc_recv_nowait(pipcd, src_id, dst_id, msg, count);
}

int ipc_pending(pipcd_t pipcd) {
	return pipc_pending(pipcd);
}

void ipc_close(pipcd_t pipcd) {
	/* TODO */
	return ;
}

