/**
 * @file schedule.h
 * @brief uSched
 *        Scheduling handlers interface header
 *
 * Date: 24-06-2014
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



#ifndef USCHED_SCHEDULE_H
#define USCHED_SCHEDULE_H

#include <stdint.h>

int schedule_daemon_init(void);
void schedule_daemon_destroy(void);
int schedule_entry_create(struct usched_entry *entry);
int schedule_entry_delete(uint32_t id);

#endif

