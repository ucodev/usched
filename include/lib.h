/**
 * @file lib.h
 * @brief uSched
 *        Client Library interface header
 *
 * Date: 17-01-2015
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


#ifndef USCHED_LIB_H
#define USCHED_LIB_H

#include <stdint.h>

#include "config.h"
#include "entry.h"
#include "usage.h"

/* Library Interface Prototypes */
#ifdef COMPILE_WIN32
DLLIMPORT
#endif
int usched_init(void);
#ifdef COMPILE_WIN32
DLLIMPORT
#endif
int usched_request(char *req);
#ifdef COMPILE_WIN32
DLLIMPORT
#endif
int usched_opt_set_remote_hostname(char *hostname);
#ifdef COMPILE_WIN32
DLLIMPORT
#endif
int usched_opt_set_remote_port(char *port);
#ifdef COMPILE_WIN32
DLLIMPORT
#endif
int usched_opt_set_remote_username(char *username);
#ifdef COMPILE_WIN32
DLLIMPORT
#endif
int usched_opt_set_remote_password(char *password);
#ifdef COMPILE_WIN32
DLLIMPORT
#endif
void usched_result_get_run(uint64_t **entry_list, size_t *nmemb);
#ifdef COMPILE_WIN32
DLLIMPORT
#endif
void usched_result_get_stop(uint64_t **entry_list, size_t *nmemb);
#ifdef COMPILE_WIN32
DLLIMPORT
#endif
void usched_result_get_show(struct usched_entry **entry_list, size_t *nmemb);
#ifdef COMPILE_WIN32
DLLIMPORT
#endif
void usched_result_free_run(void);
#ifdef COMPILE_WIN32
DLLIMPORT
#endif
void usched_result_free_stop(void);
#ifdef COMPILE_WIN32
DLLIMPORT
#endif
void usched_result_free_show(void);
usched_usage_client_err_t usched_usage_error(void);
#ifdef COMPILE_WIN32
DLLIMPORT
#endif
char *usched_usage_error_str(usched_usage_client_err_t error);
#ifdef COMPILE_WIN32
DLLIMPORT
#endif
void usched_destroy(void);

#endif

