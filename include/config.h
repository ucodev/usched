/**
 * @file config.h
 * @brief uSched
 *        Configuration interface header
 *
 * Date: 31-07-2014
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


#ifndef USCHED_CONFIG_H
#define USCHED_CONFIG_H

#include <sys/types.h>

#include <pall/cll.h>

#define CONFIG_USCHED_DEBUG			0
#define CONFIG_USCHED_PATH_MAX			4088
#define CONFIG_USCHED_DIR_BASE			"/etc/usched"
#define CONFIG_USCHED_DIR_AUTH			"/etc/usched/auth"
#define CONFIG_USCHED_DIR_CORE			"/etc/usched/core"
#define CONFIG_USCHED_DIR_NETWORK		"/etc/usched/network"
#define CONFIG_USCHED_DIR_USERS			"/etc/usched/users"
#define CONFIG_USCHED_CONN_TIMEOUT		5			/* 5 seconds timeout */
#define CONFIG_USCHED_CONN_USER_NAMED_SOCKET	"/var/run/usched.sock"
#define CONFIG_USCHED_FILE_DAEMON_SERIALIZE	"/var/run/usched_daemon.dat"
#define CONFIG_USCHED_DAEMON_PID_FILE		"/var/run/usched_usd.pid"
#define CONFIG_USCHED_EXEC_PID_FILE		"/var/run/usched_use.pid"
#define CONFIG_USCHED_CLIENT_PROC_NAME		"usc"
#define CONFIG_USCHED_DAEMON_PROC_NAME		"usd"
#define CONFIG_USCHED_EXEC_PROC_NAME		"use"
#define CONFIG_USCHED_MONITOR_PROC_NAME		"usm"
#define CONFIG_USCHED_LOG_MSG_MAX_SIZE		1024
#define CONFIG_USCHED_PMQ_DESC_NAME		"/uschedpmq01"
#define CONFIG_USCHED_PMQ_MSG_MAX		128
#define CONFIG_USCHED_AUTH_USERNAME_MAX		32
#define CONFIG_USCHED_AUTH_PASSWORD_MAX		128
#define CONFIG_USCHED_PMQ_MSG_SIZE		(8 + CONFIG_USCHED_PATH_MAX)
#define CONFIG_USCHED_HASH_FNV1A		1
/* #define CONFIG_USCHED_HASH_DJB2		1 */

/* #define CONFIG_SYS_LINUX			1 */
/* #define CONFIG_SYS_NETBSD			0 */
/* #define CONFIG_SYS_BSD			0 */
/* #define CONFIG_SYS_SOLARIS			0 */
#define CONFIG_SYS_DEV_ZERO			"/dev/zero"
#define CONFIG_SYS_DEV_NULL			"/dev/null"


/* Configuration structures */
struct usched_config_userinfo {
	char *username;
	char *password;
	uid_t uid;
	gid_t gid;
};

struct usched_config_users {
	struct cll_handler *list;
};

struct usched_config_auth {
	struct cll_handler *gid_blacklist;
	struct cll_handler *gid_whitelist;
	struct cll_handler *uid_blacklist;
	struct cll_handler *uid_whitelist;
	unsigned short use_local;
	unsigned short use_pam;
	unsigned short users_remote;
};

struct usched_config_core {
	char *file_serialize;
	unsigned int pmq_msgmax;
	size_t pmq_msgsize;
	char *pmq_name;
	int thread_priority;
	unsigned int thread_workers;
};

struct usched_config_network {
	char *bind_addr;
	char *bind_port;
	unsigned int conn_limit;
	unsigned int conn_timeout;
	char *sock_named;
};

struct usched_config {
	struct usched_config_auth auth;
	struct usched_config_core core;
	struct usched_config_network network;
	struct usched_config_users users;
};


/* Prototypes */
int config_init(struct usched_config *config);
void config_destroy(struct usched_config *config);


#endif

