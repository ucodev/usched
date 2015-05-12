/**
 * @file all.c
 * @brief uSched
 *       ALL Category administration interface
 *
 * Date: 12-05-2015
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

#include "config.h"
#include "log.h"
#include "all.h"
#include "auth.h"
#include "core.h"
#include "exec.h"
#include "network.h"
#include "stat.h"
#include "users.h"

int all_admin_show(void) {
	int errsv = 0;

	if (auth_admin_show() < 0) {
		errsv = errno;
		log_warn("all_admin_show(): auth_admin_show(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	if (core_admin_show() < 0) {
		errsv = errno;
		log_warn("all_admin_show(): core_admin_show(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	if (exec_admin_show() < 0) {
		errsv = errno;
		log_warn("all_admin_show(): exec_admin_show(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	if (ipc_admin_show() < 0) {
		errsv = errno;
		log_warn("all_admin_show(): ipc_admin_show(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	if (network_admin_show() < 0) {
		errsv = errno;
		log_warn("all_admin_show(): network_admin_show(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	if (stat_admin_show() < 0) {
		errsv = errno;
		log_warn("all_admin_show(): stat_admin_show(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	if (users_admin_show() < 0) {
		errsv = errno;
		log_warn("all_admin_show(): users_admin_show(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	return 0;
}

int all_admin_commit(void) {
	int errsv = 0;

	/* Commit auth changes */
	if (auth_admin_commit() < 0) {
		errsv = errno;
		log_warn("all_admin_commit(): auth_admin_commit(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Commit core changes */
	if (core_admin_commit() < 0) {
		errsv = errno;
		log_warn("all_admin_commit(): core_admin_commit(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Commit exec changes */
	if (exec_admin_commit() < 0) {
		errsv = errno;
		log_warn("all_admin_commit(): exec_admin_commit(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Commit ipc changes */
	if (ipc_admin_commit() < 0) {
		errsv = errno;
		log_warn("all_admin_commit(): ipc_admin_commit(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Commit network changes */
	if (network_admin_commit() < 0) {
		errsv = errno;
		log_warn("all_admin_commit(): network_admin_commit(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Commit stat changes */
	if (stat_admin_commit() < 0) {
		errsv = errno;
		log_warn("all_admin_commit(): stat_admin_commit(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Commit auth changes */
	if (users_admin_commit() < 0) {
		errsv = errno;
		log_warn("all_admin_commit(): users_admin_commit(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	return 0;
}

int all_admin_rollback(void) {
	int errsv = 0;

	/* Rollback auth changes */
	if (auth_admin_rollback() < 0) {
		errsv = errno;
		log_warn("all_admin_rollback(): auth_admin_rollback(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Rollback core changes */
	if (core_admin_rollback() < 0) {
		errsv = errno;
		log_warn("all_admin_rollback(): core_admin_rollback(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Rollback exec changes */
	if (exec_admin_rollback() < 0) {
		errsv = errno;
		log_warn("all_admin_rollback(): exec_admin_rollback(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Rollback ipc changes */
	if (ipc_admin_rollback() < 0) {
		errsv = errno;
		log_warn("all_admin_rollback(): ipc_admin_rollback(9: %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Rollback network changes */
	if (network_admin_rollback() < 0) {
		errsv = errno;
		log_warn("category_all_commit(): network_admin_rollback(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Rollback stat changes */
	if (stat_admin_rollback() < 0) {
		errsv = errno;
		log_warn("category_all_commit(): stat_admin_rollback(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Rollback auth changes */
	if (users_admin_rollback() < 0) {
		errsv = errno;
		log_warn("all_admin_rollback(): users_admin_rollback(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	return 0;
}

