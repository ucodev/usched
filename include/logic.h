/**
 * @file logic.h
 * @brief uSched
 *        Logic Analyzer interface header
 *
 * Date: 18-02-2015
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

#ifndef USCHED_LOGIC_H
#define USCHED_LOGIC_H

/* Prototypes */
int logic_admin_process_add(void);
int logic_admin_process_delete(void);
int logic_admin_process_change(void);
int logic_admin_process_show(void);
int logic_admin_process_commit(void);
int logic_admin_process_rollback(void);
int logic_client_process_run(void);
int logic_client_process_stop(void);
int logic_client_process_show(void);

#endif

