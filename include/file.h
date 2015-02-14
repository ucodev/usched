/**
 * @file file.h
 * @brief uSched
 *        File contents management interface header
 *
 * Date: 14-02-2015
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

#ifndef USCHED_FILE_H
#define USCHED_FILE_H

#include <pall/cll.h>


/* Prototypes */
char *file_read_line_single(const char *file);
struct cll_handler *file_read_line_all_ordered(const char *file);
int file_write_line_single(const char *file, const char *line);
int file_write_line_all_ordered(const char *file, struct cll_handler *lines);

#endif
