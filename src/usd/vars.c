/**
 * @file vars.c
 * @brief uSched
 *        Variable Processing interface - Daemon
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

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

#include <sys/types.h>

#include "config.h"
#include "str.h"
#include "vars.h"
#include "mm.h"


static char *_vars_replace_var(const char *in, const char *var, const char *value) {
	return strreplall(in, var, value);
}

char *vars_replace_id(const char *in, uint64_t id) {
	char buf[20];

	memset(buf, 0, sizeof(buf));

	snprintf(buf, sizeof(buf) - 1, "0x%016llX", (unsigned long long) id);

	return _vars_replace_var(in, USCHED_VAR_NAME_ID, buf);
}

char *vars_replace_username(const char *in, const char *username) {
	char buf[CONFIG_USCHED_AUTH_USERNAME_MAX + 1];

	memset(buf, 0, sizeof(buf));

	snprintf(buf, sizeof(buf) - 1, "%s", username);

	return _vars_replace_var(in, USCHED_VAR_NAME_USERNAME, buf);
}

char *vars_replace_uid(const char *in, uid_t uid) {
	char buf[24];

	memset(buf, 0, sizeof(buf));

	snprintf(buf, sizeof(buf) - 1, "%u", uid);

	return _vars_replace_var(in, USCHED_VAR_NAME_UID, buf);
}

char *vars_replace_gid(const char *in, gid_t gid) {
	char buf[24];

	memset(buf, 0, sizeof(buf));

	snprintf(buf, sizeof(buf) - 1, "%u", gid);

	return _vars_replace_var(in, USCHED_VAR_NAME_GID, buf);
}

char *vars_replace_trigger(const char *in, time_t trigger) {
	char buf[24];

	memset(buf, 0, sizeof(buf));

	snprintf(buf, sizeof(buf) - 1, "%lu", trigger);

	return _vars_replace_var(in, USCHED_VAR_NAME_TRIGGER, buf);
}

char *vars_replace_step(const char *in, time_t step) {
	char buf[24];

	memset(buf, 0, sizeof(buf));

	snprintf(buf, sizeof(buf) - 1, "%lu", step);

	return _vars_replace_var(in, USCHED_VAR_NAME_STEP, buf);
}

char *vars_replace_expire(const char *in, time_t expire) {
	char buf[24];

	memset(buf, 0, sizeof(buf));

	snprintf(buf, sizeof(buf) - 1, "%lu", expire);

	return _vars_replace_var(in, USCHED_VAR_NAME_EXPIRE, buf);
}

char *vars_replace_all(const char *in, const struct usched_vars *vars) {
	char *ret = NULL;
	const char *tmp = in;

	/* Replace variable id */
	ret = vars_replace_id(tmp, vars->id);

	if (ret) tmp = ret;

	/* Replace variable username */
	ret = vars_replace_username(tmp, vars->username);

	if (ret) {
		if (tmp != in) mm_free((void *) tmp); /* Safe! We're sure that it's non-const */
		tmp = ret;
	}

	/* Replace variable uid */
	ret = vars_replace_uid(tmp, vars->uid);

	if (ret) {
		if (tmp != in) mm_free((void *) tmp); /* Safe! We're sure that it's non-const */
		tmp = ret;
	}

	/* Replace variable gid */
	ret = vars_replace_gid(tmp, vars->gid);

	if (ret) {
		if (tmp != in) mm_free((void *) tmp); /* Safe! We're sure that it's non-const */
		tmp = ret;
	}

	/* Replace variable trigger */
	ret = vars_replace_trigger(tmp, vars->trigger);

	if (ret) {
		if (tmp != in) mm_free((void *) tmp); /* Safe! We're sure that it's non-const */
		tmp = ret;
	}

	/* Replace variable step */
	ret = vars_replace_step(tmp, vars->step);

	if (ret) {
		if (tmp != in) mm_free((void *) tmp); /* Safe! We're sure that it's non-const */
		tmp = ret;
	}

	/* Replace variable step */
	ret = vars_replace_expire(tmp, vars->expire);

	if (ret) {
		if (tmp != in) mm_free((void *) tmp); /* Safe! We're sure that it's non-const */
	}

	/* Return result */
	return ret;
}

