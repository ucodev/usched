/**
 * @file runtime.h
 * @brief uSched
 *        Runtime handlers interface header
 *
 * Date: 30-07-2014
 * 
 * Copyright 2014 Pedro A. Hortas (pah@ucodev.org)
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


#ifndef USCHED_RUNTIME_H
#define USCHED_RUNTIME_H

#include <time.h>
#include <pthread.h>
#include <mqueue.h>
#include <signal.h>

#include <pall/fifo.h>
#include <panet/panet.h>
#include <psched/sched.h>

#include "usched.h"
#include "usage.h"

/* Flags */
typedef enum USCHED_RUNTIME_FLAGS {
	USCHED_RUNTIME_FLAG_TERMINATE = 1,
	USCHED_RUNTIME_FLAG_RELOAD,
	USCHED_RUNTIME_FLAG_FLUSH,
	USCHED_RUNTIME_FLAG_LIB
} usched_runtime_flag_t;

/* Structures */
struct usched_runtime_client {
	int argc;
	char **argv;
	char *req_str;
	time_t t;
	usched_op_t op;
	usched_usage_client_err_t usage_err;
	char *usage_err_offending;
	struct usched_request *req;
	struct fifo_handler *epool;	/* Entries pool */

	sock_t fd;
	usched_runtime_flag_t flags;
	struct sigaction sa_save;
};

struct usched_runtime_daemon {
	int argc;
	char **argv;

	sock_t fd;
	usched_runtime_flag_t flags;
	struct sigaction sa_save;

	struct cll_handler *rpool;	/* Receiving pool */
	struct cll_handler *apool;	/* Active pool */

	pthread_mutex_t mutex_rpool;
	pthread_mutex_t mutex_apool;

	psched_t *psched;

	mqd_t pmqd;

	pall_fd_t ser_fd;
};

struct usched_runtime_exec {
	int argc;
	char **argv;

	usched_runtime_flag_t flags;
	struct sigaction sa_save;

	mqd_t pmqd;
};

/* External */
extern struct usched_runtime_client runc;
extern struct usched_runtime_daemon rund;
extern struct usched_runtime_exec rune;

/* Prototypes */
int runtime_client_init(int argc, char **argv);
int runtime_client_lib_init(void);
int runtime_daemon_init(int argc, char **argv);
int runtime_exec_init(int argc, char **argv);
int runtime_client_interrupted(void);
int runtime_daemon_interrupted(void);
int runtime_exec_interrupted(void);
void runtime_client_destroy(void);
void runtime_client_lib_destroy(void);
void runtime_daemon_destroy(void);
void runtime_exec_destroy(void);
void runtime_exec_quiet_destroy(void);

#endif

