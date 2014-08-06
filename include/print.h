/**
 * @file print.h
 * @brief uSched
 *        Printing interface header
 *
 * Date: 06-08-2014
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

#ifndef USCHED_PRINT_H
#define USCHED_PRINT_H

#include <stdint.h>

#include "config.h"
#include "entry.h"

/* Prototypes */
void print_admin_error(void);
void print_admin_no_priv(void);
void print_admin_config_user_added(const char *username);
void print_admin_config_user_deleted(const char *username);
void print_admin_config_user_changed(const char *username);
void print_admin_config_users(const struct usched_config_users *users);
void print_client_result_error(void);
void print_client_result_empty(void);
void print_client_result_run(uint64_t entry_id);
void print_client_result_del(const uint64_t *entry_list, size_t count);
void print_client_result_show(const struct usched_entry *entry_list, size_t count);

#endif

