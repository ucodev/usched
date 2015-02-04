/**
 * @file sig.h
 * @brief uSched
 *        Signals interface header
 *
 * Date: 04-02-2015
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


#ifndef USCHED_SIG_H
#define USCHED_SIG_H

/* Prototypes */
int sig_client_init(void);
int sig_daemon_init(void);
int sig_exec_init(void);
void sig_client_destroy(void);
void sig_daemon_destroy(void);
void sig_exec_destroy(void);

#endif

