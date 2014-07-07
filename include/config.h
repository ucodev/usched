/**
 * @file config.h
 * @brief uSched
 *        Configuration interface header
 *
 * Date: 07-07-2014
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

#define CONFIG_USCHED_DEBUG			1
#define CONFIG_USCHED_PATH_MAX			4088
#define CONFIG_USCHED_CONN_TIMEOUT		5			/* 5 seconds timeout */
#define CONFIG_USCHED_CONN_USER_NAMED_SOCKET	"/tmp/usched.sock"
#define CONFIG_USCHED_FILE_DAEMON_SERIALIZE	"/tmp/usched_daemon.dat"
#define CONFIG_USCHED_CLIENT_PROC_NAME		"usc"
#define CONFIG_USCHED_DAEMON_PROC_NAME		"usd"
#define CONFIG_USCHED_EXEC_PROC_NAME		"use"
#define CONFIG_USCHED_LOG_MSG_MAX_SIZE		1024
#define CONFIG_USCHED_PMQ_DESC_NAME		"/uschedpmq01"
#define CONFIG_USCHED_PMQ_MSG_MAX		128
#define CONFIG_USCHED_PMQ_MSG_SIZE		(8 + CONFIG_USCHED_PATH_MAX)
#define CONFIG_USCHED_HASH_FNV1A		1
/* #define CONFIG_USCHED_HASH_DJB2		1 */

/* #define CONFIG_SYS_LINUX			0 */
/* #define CONFIG_SYS_NETBSD			0 */
/* #define CONFIG_SYS_BSD			0 */
/* #define CONFIG_SYS_SOLARIS			0 */
#define CONFIG_SYS_DEV_ZERO			"/dev/zero"
#define CONFIG_SYS_DEV_NULL			"/dev/null"

#endif

