/**
 * @file print.h
 * @brief uSched
 *        Printing interface header
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

#ifndef USCHED_PRINT_H
#define USCHED_PRINT_H

#include <stdint.h>

#include "entry.h"

/* Prototypes */
void print_result_error(void);
void print_result_empty(void);
void print_result_run(uint64_t entry_id);
void print_result_del(uint64_t *entry_list, size_t count);
void print_result_show(struct usched_entry *entry_list, size_t count);

#endif

