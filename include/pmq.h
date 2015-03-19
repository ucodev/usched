/**
 * @file pmq.h
 * @brief uSched
 *        POSIX Message Queueing interface header
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


#ifndef USCHED_PMQ_H
#define USCHED_PMQ_H

#include <fcntl.h>
#include <sys/stat.h>

#if CONFIG_USE_IPC_PMQ == 1
 #include <mqueue.h>
#else
 typedef unsigned int mqd_t;
#endif

#include <sys/types.h>


/* Prototypes */
mqd_t pmq_init(const char *name, int oflags, mode_t mode, long maxmsg, long msgsize);
void pmq_destroy(mqd_t pmqd);
int pmq_unlink(const char *pmqname);
int pmq_daemon_init(void);
int pmq_exec_init(void);
void pmq_daemon_destroy(void);
void pmq_exec_destroy(void);
int pmq_admin_create(void);
int pmq_admin_delete(void);

#endif

