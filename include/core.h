/**
 * @file core.h
 * @brief uSched
 *        Core configuration and administration interface header
 *
 * Date: 03-02-2015
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

#ifndef USCHED_CORE_H
#define USCHED_CORE_H

/* Prototypes */
char *core_admin_delta_noexec_get(void);
int core_admin_delta_noexec_set(const char *delta_noexec);
char *core_admin_delta_reload_get(void);
int core_admin_delta_reload_set(const char *delta_reload);
char *core_admin_file_serialize_get(void);
int core_admin_file_serialize_set(const char *file_serialize);
char *core_admin_jail_dir_get(void);
int core_admin_jail_dir_set(const char *jail_dir);
char *core_admin_pmq_msgmax_get(void);
int core_admin_pmq_msgmax_set(const char *pmq_msgmax);
char *core_admin_pmq_msgsize_get(void);
int core_admin_pmq_msgsize_set(const char *pmq_msgsize);

#endif
