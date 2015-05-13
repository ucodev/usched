/**
 * @file log.c
 * @brief uSched
 *        Logging interface
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


#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "config.h"

#ifndef COMPILE_WIN32
#include <syslog.h>
#endif

#include "debug.h"
#include "log.h"


int log_client_init(void) {
#ifndef COMPILE_WIN32
	openlog(CONFIG_USCHED_CLIENT_PROC_NAME, LOG_NOWAIT | LOG_PID, LOG_LOCAL0);
#endif

	return 0;
}
	
#ifndef COMPILE_WIN32
int log_admin_init(void) {
	openlog(CONFIG_USCHED_ADMIN_PROC_NAME, LOG_NOWAIT | LOG_PID, LOG_LOCAL0);

	return 0;
}
	
int log_daemon_init(void) {
	openlog(CONFIG_USCHED_DAEMON_PROC_NAME, LOG_NOWAIT | LOG_PID, LOG_DAEMON);

	return 0;
}

int log_exec_init(void) {
	openlog(CONFIG_USCHED_EXEC_PROC_NAME, LOG_NOWAIT | LOG_PID, LOG_DAEMON);

	return 0;
}

int log_ipc_init(void) {
	openlog(CONFIG_USCHED_IPC_PROC_NAME, LOG_NOWAIT | LOG_PID, LOG_DAEMON);

	return 0;
}

int log_monitor_init(void) {
	openlog(CONFIG_USCHED_MONITOR_PROC_NAME, LOG_NOWAIT | LOG_PID, LOG_LOCAL0);

	return 0;
}

int log_stat_init(void) {
	openlog(CONFIG_USCHED_STAT_PROC_NAME, LOG_NOWAIT | LOG_PID, LOG_DAEMON);

	return 0;
}
#endif

static void _log_msg(int priority, const char *msg) {
#ifdef COMPILE_WIN32
	fprintf(stderr, "%s", msg);
#else
	syslog(priority, "%s", msg);
#endif
}

void log_info(const char *fmt, ...) {
	va_list ap;
	char msg[CONFIG_USCHED_LOG_MSG_MAX_SIZE + 1];

	memset(msg, 0, sizeof(msg));

	va_start(ap, fmt);

	vsnprintf(msg, CONFIG_USCHED_LOG_MSG_MAX_SIZE, fmt, ap);

	va_end(ap);

	debug_printf(DEBUG_INFO, "== DEBUG INFO == %s", msg);

	_log_msg(LOG_INFO, msg);
}

void log_warn(const char *fmt, ...) {
	va_list ap;
	char msg[CONFIG_USCHED_LOG_MSG_MAX_SIZE + 1];

	memset(msg, 0, sizeof(msg));

	va_start(ap, fmt);

	vsnprintf(msg, CONFIG_USCHED_LOG_MSG_MAX_SIZE, fmt, ap);

	va_end(ap);

	debug_printf(DEBUG_WARN, "== DEBUG WARN == %s", msg);

	_log_msg(LOG_WARNING, msg);
}

void log_crit(const char *fmt, ...) {
	va_list ap;
	char msg[CONFIG_USCHED_LOG_MSG_MAX_SIZE + 1];

	memset(msg, 0, sizeof(msg));

	va_start(ap, fmt);

	vsnprintf(msg, CONFIG_USCHED_LOG_MSG_MAX_SIZE, fmt, ap);

	va_end(ap);

	debug_printf(DEBUG_CRIT, "== DEBUG CRIT == %s", msg);

	_log_msg(LOG_CRIT, msg);
}

void log_destroy(void) {
#ifndef COMPILE_WIN32
	closelog();
#endif
}

