/**
 * @file logic.c
 * @brief uSched
 *        Logic Analyzer interface - Admin
 *
 * Date: 15-02-2015
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

#include <string.h>
#include <errno.h>

#include "runtime.h"
#include "log.h"
#include "logic.h"
#include "category.h"

int logic_admin_process_add(void) {
	int errsv = 0;

	if (runa.req->category == USCHED_CATEGORY_USERS) {
		/* Add user */
		if (category_users_add(runa.req->argc, runa.req->args) < 0) {
			errsv = errno;
			log_warn("logic_admin_process_add(): category_users_add(): %s\n", strerror(errno));
			errno = errsv;
			return -1;
		}
	} else if (runa.req->category == USCHED_CATEGORY_AUTH) {
		/* Add auth */
		if (category_auth_add(runa.req->argc, runa.req->args) < 0) {
			errsv = errno;
			log_warn("logic_admin_process_add(): category_auth_add(): %s\n", strerror(errno));
			errno = errsv;
			return -1;
		}
	} else {
		log_warn("logic_admin_process_add(): Invalid category.\n");
		errno = EINVAL;
		return -1;
	}

	return 0;
}

int logic_admin_process_delete(void) {
	int errsv = 0;

	if (runa.req->category == USCHED_CATEGORY_USERS) {
		if (category_users_delete(runa.req->argc, runa.req->args) < 0) {
			errsv = errno;
			log_warn("logic_admin_process_delete(): category_users_delete(): %s\n", strerror(errno));
			errno = errsv;
			return -1;
		}
	} else if (runa.req->category == USCHED_CATEGORY_AUTH) {
		/* Delete auth */
		if (category_auth_delete(runa.req->argc, runa.req->args) < 0) {
			errsv = errno;
			log_warn("logic_admin_process_delete(): category_auth_delete(): %s\n", strerror(errno));
			errno = errsv;
			return -1;
		}
	} else {
		log_warn("logic_admin_process_delete(): Invalid category.\n");
		errno = EINVAL;
		return -1;
	}

	return 0;
}

int logic_admin_process_change(void) {
	int errsv = 0;

	if (runa.req->category == USCHED_CATEGORY_AUTH) {
		if (category_auth_change(runa.req->argc, runa.req->args) < 0) {
			errsv = errno;
			log_warn("logic_admin_process_change(): category_auth_change(): %s\n", strerror(errno));
			errno = errsv;
			return -1;
		}
	} else if (runa.req->category == USCHED_CATEGORY_CORE) {
		if (category_core_change(runa.req->argc, runa.req->args) < 0) {
			errsv = errno;
			log_warn("logic_admin_process_change(): category_core_change(): %s\n", strerror(errno));
			errno = errsv;
			return -1;
		}
	} else if (runa.req->category == USCHED_CATEGORY_NETWORK) {
		if (category_network_change(runa.req->argc, runa.req->args) < 0) {
			errsv = errno;
			log_warn("logic_admin_process_change(): category_network_change(): %s\n", strerror(errno));
			errno = errsv;
			return -1;
		}
	} else if (runa.req->category == USCHED_CATEGORY_USERS) {
		if (category_users_change(runa.req->argc, runa.req->args) < 0) {
			errsv = errno;
			log_warn("logic_admin_process_change(): category_users_change(): %s\n", strerror(errno));
			errno = errsv;
			return -1;
		}
	} else {
		log_warn("logic_admin_process_change(): Invalid category.\n");
		errno = EINVAL;
		return -1;
	}

	return 0;
}

int logic_admin_process_show(void) {
	int errsv = 0;

	if (runa.req->category == USCHED_CATEGORY_AUTH) {
		if (category_auth_show(runa.req->argc, runa.req->args) < 0) {
			errsv = errno;
			log_warn("logic_admin_process_show(): category_auth_show(): %s\n", strerror(errno));
			errno = errsv;
			return -1;
		}
	} else if (runa.req->category == USCHED_CATEGORY_CORE) {
		if (category_core_show(runa.req->argc, runa.req->args) < 0) {
			errsv = errno;
			log_warn("logic_admin_process_show(): category_core_show(): %s\n", strerror(errno));
			errno = errsv;
			return -1;
		}
	} else if (runa.req->category == USCHED_CATEGORY_NETWORK) {
		if (category_network_show(runa.req->argc, runa.req->args) < 0) {
			errsv = errno;
			log_warn("logic_admin_process_show(): category_network_show(): %s\n", strerror(errno));
			errno = errsv;
			return -1;
		}
	} else if (runa.req->category == USCHED_CATEGORY_USERS) {
		if (category_users_show(runa.req->argc, runa.req->args) < 0) {
			errsv = errno;
			log_warn("logic_admin_process_show(): category_users_show(): %s\n", strerror(errno));
			errno = errsv;
			return -1;
		}
	} else {
		log_warn("logic_admin_process_show(): Invalid category.\n");
		errno = EINVAL;
		return -1;
	}

	return 0;
}

