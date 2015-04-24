/**
 * @file sysvmq.h
 * @brief uSched
 *        System V Message Queueing interface header
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


#ifndef USCHED_SYSVMQ_H
#define USCHED_SYSVMQ_H

#include <sys/types.h>

#include "config.h"

#if CONFIG_USE_IPC_SYSVMQ == 1
 #include <sys/ipc.h>
 #include <sys/msg.h>
#endif


/* Prototypes */
#if CONFIG_USE_IPC_SYSVMQ == 1
int sysvmq_init(const char *key, int oflags, mode_t mode, long maxmsg, long msgsize);
void sysvmq_destroy(int mqid);
int sysvmq_unlink(int mqid);
#endif

#endif

