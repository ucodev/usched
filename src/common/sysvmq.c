/**
 * @file sysvmq.c
 * @brief uSched
 *        System V Message Queueing interface
 *
 * Date: 24-04-2015
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


#include <string.h>
#include <errno.h>

#include <sys/types.h>

#include "config.h"
#include "runtime.h"
#include "log.h"
#include "sysvmq.h"

#if CONFIG_USE_IPC_SYSVMQ == 1
 #include <sys/ipc.h>
 #include <sys/msg.h>
#endif

#if CONFIG_USE_IPC_SYSVMQ == 1
int sysvmq_init(const char *key, int oflags, mode_t mode, long maxmsg, long msgsize) {
	errno = ENOSYS;
	return -1;
}
#endif

#if CONFIG_USE_IPC_SYSVMQ == 1
void sysvmq_destroy(int mqid) {
	return ;
}
#endif

#if CONFIG_USE_IPC_SYSVMQ == 1
int sysvmq_unlink(int mqid) {
	errno = ENOSYS;
	return -1;
}
#endif

