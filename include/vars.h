/**
 * @file vars.h
 * @brief uSched
 *        Variable Processing interface header - Daemon
 *
 * Date: 06-02-2015
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

#ifndef USCHED_VARS_H
#define USCHED_VARS_H

#include <stdint.h>
#include <time.h>

#include <sys/types.h>

/* Variable names */
#define USCHED_VAR_NAME_ID		"@@id@@"
#define USCHED_VAR_NAME_USERNAME	"@@username@@"
#define USCHED_VAR_NAME_UID		"@@uid@@"
#define USCHED_VAR_NAME_GID		"@@gid@@"
#define USCHED_VAR_NAME_TRIGGER		"@@trigger@@"
#define USCHED_VAR_NAME_STEP		"@@step@@"
#define USCHED_VAR_NAME_EXPIRE		"@@expire@@"

/* Structures */
struct usched_vars {
	uint64_t id;
	char *username;
	uid_t uid;
	gid_t gid;
	time_t trigger;
	time_t step;
	time_t expire;
};

/* Prototypes */
char *vars_replace_id(const char *in, uint64_t id);
char *vars_replace_username(const char *in, const char *username);
char *vars_replace_uid(const char *in, uid_t uid);
char *vars_replace_gid(const char *in, gid_t gid);
char *vars_replace_trigger(const char *in, time_t trigger);
char *vars_replace_step(const char *in, time_t step);
char *vars_replace_expire(const char *in, time_t expire);
char *vars_replace_all(const char *in, const struct usched_vars *vars);

#endif

