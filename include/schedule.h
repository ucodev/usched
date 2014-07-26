/**
 * @file schedule.h
 * @brief uSched
 *        Scheduling handlers interface header
 *
 * Date: 26-07-2014
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

#include <sys/types.h>

/* Prototypes */
int schedule_daemon_init(void);
void schedule_daemon_destroy(void);
int schedule_entry_create(struct usched_entry *entry);
struct usched_entry *schedule_entry_get_copy(uint64_t entry_id);
int schedule_entry_get_by_uid(uid_t uid, uint64_t **entry_list, uint32_t *count);
struct usched_entry *schedule_entry_disable(struct usched_entry *entry);
int schedule_entry_delete(struct usched_entry *entry);
int schedule_entry_ownership_delete_by_id(uint64_t id, uid_t uid);
int schedule_entry_update(struct usched_entry *entry);

#endif

