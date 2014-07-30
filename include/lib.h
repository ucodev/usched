/**
 * @file lib.h
 * @brief uSched
 *        Client Library interface header
 *
 * Date: 25-06-2014
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


#ifndef USCHED_LIB_H
#define USCHED_LIB_H

#include <stdint.h>

#include "entry.h"
#include "usage.h"

/* Library Interface Prototypes */
int usched_init(void);
int usched_request(char *req);
void usched_result_get_run(uint64_t **entry_list, size_t *nmemb);
void usched_result_get_stop(uint64_t **entry_list, size_t *nmemb);
void usched_result_get_show(struct usched_entry **entry_list, size_t *nmemb);
void usched_result_free_run(void);
void usched_result_free_stop(void);
void usched_result_free_show(void);
usched_usage_client_err_t usched_usage_error(void);
char *usched_usage_error_str(usched_usage_client_err_t error);
void usched_destroy(void);

#endif

