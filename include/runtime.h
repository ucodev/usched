/**
 * @file runtime.h
 * @brief uSched
 *        Runtime handlers interface header
 *
 * Date: 16-02-2015
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


#ifndef USCHED_RUNTIME_H
#define USCHED_RUNTIME_H

#include <time.h>

#include "config.h"

#ifndef COMPILE_WIN32
#include <pthread.h>
#include <mqueue.h>
#endif
#include <signal.h>

#include <pall/fifo.h>
#include <panet/panet.h>
#if CONFIG_CLIENT_ONLY == 0
#include <psched/sched.h>
#endif

#include <sys/types.h>

#include "usched.h"
#include "usage.h"
#include "opt.h"

/* Flags */
typedef enum USCHED_RUNTIME_FLAGS {
	USCHED_RUNTIME_FLAG_TERMINATE = 1,
	USCHED_RUNTIME_FLAG_FATAL,
	USCHED_RUNTIME_FLAG_RELOAD,
	USCHED_RUNTIME_FLAG_FLUSH,
	USCHED_RUNTIME_FLAG_INTERRUPT, /* Set atomically */
	USCHED_RUNTIME_FLAG_LIB
} usched_runtime_flag_t;

/* Structures */
#if CONFIG_CLIENT_SPECIFIC == 1 || CONFIG_COMMON == 1
struct usched_runtime_client {
	int argc;
	char **argv;
	char *req_str;
	time_t t;
	usched_op_t op;
	usched_usage_client_err_t usage_err;
	char *usage_err_offending;
	struct usched_client_request *req;
	struct fifo_handler *epool;	/* Entries pool */
	void *result;
	size_t result_nmemb;

	sock_t fd;

	volatile usched_runtime_flag_t flags;
	struct sigaction sa_save;

	struct usched_config config;

	/* Command line options */
	struct usched_opt_client opt;
};
#endif /* CONFIG_CLIENT_SPECIFIC */

#if CONFIG_CLIENT_ONLY == 0
#if CONFIG_ADMIN_SPECIFIC == 1 || CONFIG_COMMON == 1
struct usched_runtime_admin {
	int argc;
	char **argv;

	usched_op_t op;
	usched_usage_admin_err_t usage_err;
	char *usage_err_offending;
	struct usched_admin_request *req;

	volatile usched_runtime_flag_t flags;
	struct sigaction sa_save;

	struct usched_config config;
};
#endif /* CONFIG_ADMIN_SPECIFIC */

#if CONFIG_DAEMON_SPECIFIC == 1 || CONFIG_COMMON == 1
struct usched_runtime_daemon {
	int argc;
	char **argv;

	pid_t pid;

	sock_t fd_unix;
	sock_t fd_remote;
	volatile usched_runtime_flag_t flags;
	struct sigaction sa_save;

	struct cll_handler *rpool;	/* Receiving pool */
	struct cll_handler *apool;	/* Active pool */

	pthread_mutex_t mutex_interrupt;
	pthread_mutex_t mutex_rpool;
	pthread_mutex_t mutex_apool;
#if CONFIG_USCHED_SERIALIZE_ON_REQ == 1
	pthread_mutex_t mutex_marshal;
#endif

	psched_t *psched;

	mqd_t pmqd;

	pall_fd_t ser_fd;

	size_t conn_cur;

	struct usched_config config;

	pthread_t t_runtime;
	pthread_t t_unix, t_remote;
	pthread_t t_delta;

	time_t time_last;
	int64_t delta_last;
};
#endif /* CONFIG_DAEMON_SPECIFIC */

#if CONFIG_EXEC_SPECIFIC == 1 || CONFIG_COMMON == 1
struct usched_runtime_exec {
	int argc;
	char **argv;

	volatile usched_runtime_flag_t flags;
	struct sigaction sa_save;

	mqd_t pmqd;

	struct usched_config config;
};
#endif /* CONFIG_EXEC_SPECIFIC */
#endif /* CONFIG_CLIENT_ONLY == 0 */

/* External */
#if CONFIG_CLIENT_SPECIFIC == 1
extern struct usched_runtime_client runc;
#endif
#if CONFIG_CLIENT_ONLY == 0
 #if CONFIG_ADMIN_SPECIFIC == 1 || CONFIG_COMMON == 1
 extern struct usched_runtime_admin runa;
 #endif /* CONFIG_ADMIN_SPECIFIC */
 #if CONFIG_DAEMON_SPECIFIC == 1 || CONFIG_COMMON == 1
 extern struct usched_runtime_daemon rund;
 #endif /* CONFIG_DAEMON_SPECIFIC */
 #if CONFIG_EXEC_SPECIFIC == 1 || CONFIG_COMMON == 1
 extern struct usched_runtime_exec rune;
 #endif /* CONFIG_EXEC_SPECIFIC */
#endif /* CONFIG_CLIENT_ONLY == 0 */

/* Prototypes */
int runtime_client_init(int argc, char **argv);
int runtime_client_lib_init(void);
int runtime_client_lib_reset(void);
int runtime_client_interrupted(void);
#if CONFIG_CLIENT_ONLY == 0
int runtime_admin_init(int argc, char **argv);
int runtime_daemon_init(int argc, char **argv);
int runtime_exec_init(int argc, char **argv);
int runtime_admin_interrupted(void);
void runtime_daemon_fatal(void);
void runtime_daemon_interrupt(void);
int runtime_daemon_interrupted(void);
int runtime_exec_interrupted(void);
#endif /* CONFIG_CLIENT_ONLY == 0 */
void runtime_client_destroy(void);
void runtime_client_lib_destroy(void);
#if CONFIG_CLIENT_ONLY == 0
void runtime_admin_destroy(void);
void runtime_daemon_destroy(void);
void runtime_exec_destroy(void);
void runtime_exec_quiet_destroy(void);
#endif /* CONFIG_CLIENT_ONLY == 0 */

#endif

