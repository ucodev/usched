/**
 * @file str.h
 * @brief uSched
 *        String helper interface header
 *
 * Date: 15-02-2015
 * 
 * Copyright 2014-2015 Pedro A. Hortas (pah@ucodev.org)
 *
 * This file is part of usched.
 *
 * usched is mm_free software: you can redistribute it and/or modify
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

#ifndef USCHED_STR_H
#define USCHED_STR_H

/* Prototypes */
char *strrepl(const char *haystack, const char *needle, const char *rcontent);
char *strreplall(const char *haystack, const char *needle, const char *rcontent);
int strisnum(const char *s);

#endif

