/**
 * @file exec.c
 * @brief uSched
 *        Exec configuration and administration interface
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

#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <fsop/path.h>
#include <fsop/file.h>

#include "config.h"
#include "admin.h"
#include "exec.h"
#include "log.h"
#include "mm.h"
#include "usched.h"

int exec_admin_commit(void) {
	int errsv = 0;

	/* Check if services are running and stop processing if they are */
	if (fsop_path_exists(CONFIG_USCHED_DAEMON_PID_FILE)) {
		log_crit("exec_admin_commit(): uSched services are running (usd). You must stop all uSched services before you can commit exec configuration changes.\n");
		errno = EBUSY;
		return -1;
	}

	if (fsop_path_exists(CONFIG_USCHED_EXEC_PID_FILE)) {
		log_crit("exec_admin_commit(): uSched services are running (use). You must stop all uSched services before you can commit exec configuration changes.\n");
		errno = EBUSY;
		return -1;
	}

	if (fsop_path_exists(CONFIG_USCHED_STAT_PID_FILE)) {
		log_crit("exec_admin_commit(): uSched services are running (uss). You must stop all uSched services before you can commit exec configuration changes.\n");
		errno = EBUSY;
		return -1;
	}

	if (fsop_path_exists(CONFIG_USCHED_IPC_PID_FILE)) {
		log_crit("exec_admin_commit(): uSched services are running (usi). You must stop all uSched services before you can commit exec configuration changes.\n");
		errno = EBUSY;
		return -1;
	}

	/* Destroy the current configuration */
	config_admin_destroy();

	/* delta.noexec */
	if (fsop_cp(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_EXEC "/." CONFIG_USCHED_FILE_EXEC_DELTA_NOEXEC, CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_EXEC "/" CONFIG_USCHED_FILE_EXEC_DELTA_NOEXEC, 128) < 0) {
		errsv = errno;
		log_crit("exec_admin_commit(): fsop_cp(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Re-initialize the configuration */
	if (config_admin_init() < 0) {
		errsv = errno;
		log_crit("exec_admin_commit(): config_admin_init(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* All good */
	return 0;
}

int exec_admin_rollback(void) {
	int errsv = 0;

	/* delta.noexec */
	if (fsop_cp(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_EXEC "/" CONFIG_USCHED_FILE_EXEC_DELTA_NOEXEC, CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_EXEC "/." CONFIG_USCHED_FILE_EXEC_DELTA_NOEXEC, 128) < 0) {
		errsv = errno;
		log_crit("exec_admin_rollback(): fsop_cp(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* All good */
	return 0;
}

int exec_admin_show(void) {
	int errsv = 0;

	if (exec_admin_delta_noexec_show() < 0) {
		errsv = errno;
		log_crit("exec_admin_show(): exec_admin_delta_noexec_show(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	return 0;
}

int exec_admin_delta_noexec_show(void) {
	int errsv = 0;

	if (admin_property_show(CONFIG_USCHED_DIR_EXEC, USCHED_CATEGORY_EXEC_STR, CONFIG_USCHED_FILE_EXEC_DELTA_NOEXEC) < 0) {
		errsv = errno;
		log_crit("exec_admin_delta_noexec_show(): admin_property_show(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* All good */
	return 0;
}

int exec_admin_delta_noexec_change(const char *ipc_msgmax) {
	int errsv = 0;

	if (admin_property_change(CONFIG_USCHED_DIR_EXEC, CONFIG_USCHED_FILE_EXEC_DELTA_NOEXEC, delta_noexec) < 0) {
		errsv = errno;
		log_crit("exec_admin_delta_noexec_change(): admin_property_change(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	if (exec_admin_delta_noexec_show() < 0) {
		errsv = errno;
		log_crit("exec_admin_delta_noexec_change(): exec_admin_delta_noexec_show(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	return 0;
}

