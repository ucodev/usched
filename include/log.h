/**
 * @file log.h
 * @brief uSched
 *        Logging interface header
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


#ifndef USCHED_LOG_H
#define USCHED_LOG_H

/* Prototypes */
int log_admin_init(void);
int log_client_init(void);
int log_daemon_init(void);
int log_exec_init(void);
int log_ipc_init(void);
int log_monitor_init(void);
int log_stat_init(void);
void log_info(const char *fmt, ...);
void log_warn(const char *fmt, ...);
void log_crit(const char *fmt, ...);
void log_destroy(void);

#endif

