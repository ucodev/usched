/**
 * @file stat.c
 * @brief uSched
 *        Stat configuration and administration interface
 *
 * Date: 19-05-2015
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
#include "stat.h"
#include "file.h"
#include "log.h"
#include "mm.h"
#include "usched.h"
#include "print.h"

int stat_admin_commit(void) {
	int errsv = 0;

	/* Check if services are running and stop processing if they are */
	if (fsop_path_exists(CONFIG_USCHED_DAEMON_PID_FILE)) {
		log_crit("stat_admin_commit(): uSched services are running (usd). You must stop all uSched services before you can commit stat configuration changes.\n");
		errno = EBUSY;
		return -1;
	}

	if (fsop_path_exists(CONFIG_USCHED_EXEC_PID_FILE)) {
		log_crit("stat_admin_commit(): uSched services are running (use). You must stop all uSched services before you can commit stat configuration changes.\n");
		errno = EBUSY;
		return -1;
	}

	if (fsop_path_exists(CONFIG_USCHED_STAT_PID_FILE)) {
		log_crit("stat_admin_commit(): uSched services are running (uss). You must stop all uSched services before you can commit stat configuration changes.\n");
		errno = EBUSY;
		return -1;
	}

	/* Destroy the current configuration */
	config_admin_destroy();

	/* jail.dir */
	if (fsop_cp(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_STAT "/." CONFIG_USCHED_FILE_STAT_JAIL_DIR, CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_STAT "/" CONFIG_USCHED_FILE_STAT_JAIL_DIR, 128) < 0) {
		errsv = errno;
		log_crit("stat_admin_commit(): fsop_cp(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* privdrop.user */
	if (fsop_cp(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_STAT "/." CONFIG_USCHED_FILE_STAT_PRIVDROP_USER, CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_STAT "/" CONFIG_USCHED_FILE_STAT_PRIVDROP_USER, 128) < 0) {
		errsv = errno;
		log_crit("stat_admin_commit(): fsop_cp(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* privdrop.group */
	if (fsop_cp(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_STAT "/." CONFIG_USCHED_FILE_STAT_PRIVDROP_GROUP, CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_STAT "/" CONFIG_USCHED_FILE_STAT_PRIVDROP_GROUP, 128) < 0) {
		errsv = errno;
		log_crit("stat_admin_commit(): fsop_cp(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* report.file */
	if (fsop_cp(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_STAT "/." CONFIG_USCHED_FILE_STAT_REPORT_FILE, CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_STAT "/" CONFIG_USCHED_FILE_STAT_REPORT_FILE, 128) < 0) {
		errsv = errno;
		log_crit("stat_admin_commit(): fsop_cp(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* report.freq */
	if (fsop_cp(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_STAT "/." CONFIG_USCHED_FILE_STAT_REPORT_FREQ, CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_STAT "/" CONFIG_USCHED_FILE_STAT_REPORT_FREQ, 128) < 0) {
		errsv = errno;
		log_crit("stat_admin_commit(): fsop_cp(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* report.mode */
	if (fsop_cp(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_STAT "/." CONFIG_USCHED_FILE_STAT_REPORT_MODE, CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_STAT "/" CONFIG_USCHED_FILE_STAT_REPORT_MODE, 128) < 0) {
		errsv = errno;
		log_crit("stat_admin_commit(): fsop_cp(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Re-initialize the configuration */
	if (config_admin_init() < 0) {
		errsv = errno;
		log_crit("stat_admin_commit(): config_admin_init(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* All good */
	return 0;
}

int stat_admin_rollback(void) {
	int errsv = 0;

	/* jail.dir */
	if (fsop_cp(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_STAT "/" CONFIG_USCHED_FILE_STAT_JAIL_DIR, CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_STAT "/." CONFIG_USCHED_FILE_STAT_JAIL_DIR, 128) < 0) {
		errsv = errno;
		log_crit("stat_admin_rollback(): fsop_cp(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* privdrop.user */
	if (fsop_cp(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_STAT "/" CONFIG_USCHED_FILE_STAT_PRIVDROP_USER, CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_STAT "/." CONFIG_USCHED_FILE_STAT_PRIVDROP_USER, 128) < 0) {
		errsv = errno;
		log_crit("stat_admin_rollback(): fsop_cp(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* privdrop.group */
	if (fsop_cp(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_STAT "/" CONFIG_USCHED_FILE_STAT_PRIVDROP_GROUP, CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_STAT "/." CONFIG_USCHED_FILE_STAT_PRIVDROP_GROUP, 128) < 0) {
		errsv = errno;
		log_crit("stat_admin_rollback(): fsop_cp(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* report.file */
	if (fsop_cp(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_STAT "/" CONFIG_USCHED_FILE_STAT_REPORT_FILE, CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_STAT "/." CONFIG_USCHED_FILE_STAT_REPORT_FILE, 128) < 0) {
		errsv = errno;
		log_crit("stat_admin_rollback(): fsop_cp(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* report.freq */
	if (fsop_cp(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_STAT "/" CONFIG_USCHED_FILE_STAT_REPORT_FREQ, CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_STAT "/." CONFIG_USCHED_FILE_STAT_REPORT_FREQ, 128) < 0) {
		errsv = errno;
		log_crit("stat_admin_rollback(): fsop_cp(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* report.mode */
	if (fsop_cp(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_STAT "/" CONFIG_USCHED_FILE_STAT_REPORT_MODE, CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_STAT "/." CONFIG_USCHED_FILE_STAT_REPORT_MODE, 128) < 0) {
		errsv = errno;
		log_crit("stat_admin_rollback(): fsop_cp(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* All good */
	return 0;
}

int stat_admin_show(void) {
	int errsv = 0;

	if (stat_admin_jail_dir_show() < 0) {
		errsv = errno;
		log_crit("stat_admin_show(): stat_admin_jail_dir_show(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	if (stat_admin_privdrop_group_show() < 0) {
		errsv = errno;
		log_crit("stat_admin_show(): stat_admin_privdrop_group_show(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	if (stat_admin_privdrop_user_show() < 0) {
		errsv = errno;
		log_crit("stat_admin_show(): stat_admin_privdrop_user_show(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	if (stat_admin_report_file_show() < 0) {
		errsv = errno;
		log_crit("stat_admin_show(): stat_admin_report_file_show(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	if (stat_admin_report_freq_show() < 0) {
		errsv = errno;
		log_crit("stat_admin_show(): stat_admin_report_freq_show(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	if (stat_admin_report_mode_show() < 0) {
		errsv = errno;
		log_crit("stat_admin_show(): stat_admin_report_mode_show(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	return 0;
}

int stat_admin_jail_dir_show(void) {
	int errsv = 0;

	if (admin_property_show(CONFIG_USCHED_DIR_STAT, USCHED_CATEGORY_STAT_STR, CONFIG_USCHED_FILE_STAT_JAIL_DIR) < 0) {
		errsv = errno;
		log_crit("stat_admin_jail_dir_show(): admin_property_show(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* All good */
	return 0;
}

int stat_admin_jail_dir_change(const char *jail_dir) {
	int errsv = 0;

	if (admin_property_change(CONFIG_USCHED_DIR_STAT, CONFIG_USCHED_FILE_STAT_JAIL_DIR, jail_dir) < 0) {
		errsv = errno;
		log_crit("stat_admin_jail_dir_change(): admin_property_change(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	if (stat_admin_jail_dir_show() < 0) {
		errsv = errno;
		log_crit("stat_admin_jail_dir_change(): stat_admin_jail_dir_show(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	return 0;
}

int stat_admin_privdrop_group_show(void) {
	int errsv = 0;

	if (admin_property_show(CONFIG_USCHED_DIR_STAT, USCHED_CATEGORY_STAT_STR, CONFIG_USCHED_FILE_STAT_PRIVDROP_GROUP) < 0) {
		errsv = errno;
		log_crit("stat_admin_privdrop_group_show(): admin_property_show(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* All good */
	return 0;
}

int stat_admin_privdrop_group_change(const char *privdrop_group) {
	int errsv = 0;

	if (admin_property_change(CONFIG_USCHED_DIR_STAT, CONFIG_USCHED_FILE_STAT_PRIVDROP_GROUP, privdrop_group) < 0) {
		errsv = errno;
		log_crit("stat_admin_privdrop_group_change(): admin_property_change(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	if (stat_admin_privdrop_group_show() < 0) {
		errsv = errno;
		log_crit("stat_admin_privdrop_group_change(): stat_admin_privdrop_group_show(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	return 0;
}

int stat_admin_privdrop_user_show(void) {
	int errsv = 0;

	if (admin_property_show(CONFIG_USCHED_DIR_STAT, USCHED_CATEGORY_STAT_STR, CONFIG_USCHED_FILE_STAT_PRIVDROP_USER) < 0) {
		errsv = errno;
		log_crit("stat_admin_privdrop_user_show(): admin_property_show(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* All good */
	return 0;
}

int stat_admin_privdrop_user_change(const char *privdrop_user) {
	int errsv = 0;

	if (admin_property_change(CONFIG_USCHED_DIR_STAT, CONFIG_USCHED_FILE_STAT_PRIVDROP_USER, privdrop_user) < 0) {
		errsv = errno;
		log_crit("stat_admin_privdrop_user_change(): admin_property_change(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	if (stat_admin_privdrop_user_show() < 0) {
		errsv = errno;
		log_crit("stat_admin_privdrop_user_change(): stat_admin_privdrop_user_show(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	return 0;
}

int stat_admin_report_file_show(void) {
	int errsv = 0;

	if (admin_property_show(CONFIG_USCHED_DIR_STAT, USCHED_CATEGORY_STAT_STR, CONFIG_USCHED_FILE_STAT_REPORT_FILE) < 0) {
		errsv = errno;
		log_crit("stat_admin_report_file_show(): admin_property_show(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* All good */
	return 0;
}

int stat_admin_report_file_change(const char *report_file) {
	int errsv = 0;

	if (admin_property_change(CONFIG_USCHED_DIR_STAT, CONFIG_USCHED_FILE_STAT_REPORT_FILE, report_file) < 0) {
		errsv = errno;
		log_crit("stat_admin_report_file_change(): admin_property_change(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	if (stat_admin_report_file_show() < 0) {
		errsv = errno;
		log_crit("stat_admin_report_file_change(): stat_admin_report_file_show(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	return 0;
}

int stat_admin_report_freq_show(void) {
	int errsv = 0;

	if (admin_property_show(CONFIG_USCHED_DIR_STAT, USCHED_CATEGORY_STAT_STR, CONFIG_USCHED_FILE_STAT_REPORT_FREQ) < 0) {
		errsv = errno;
		log_crit("stat_admin_report_freq_show(): admin_property_show(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* All good */
	return 0;
}

int stat_admin_report_freq_change(const char *report_freq) {
	int errsv = 0;

	if (admin_property_change(CONFIG_USCHED_DIR_STAT, CONFIG_USCHED_FILE_STAT_REPORT_FREQ, report_freq) < 0) {
		errsv = errno;
		log_crit("stat_admin_report_freq_change(): admin_property_change(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	if (stat_admin_report_freq_show() < 0) {
		errsv = errno;
		log_crit("stat_admin_report_freq_change(): stat_admin_report_freq_show(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	return 0;
}

int stat_admin_report_mode_show(void) {
	int errsv = 0;

	if (admin_property_show(CONFIG_USCHED_DIR_STAT, USCHED_CATEGORY_STAT_STR, CONFIG_USCHED_FILE_STAT_REPORT_MODE) < 0) {
		errsv = errno;
		log_crit("stat_admin_report_mode_show(): admin_property_show(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* All good */
	return 0;
}

int stat_admin_report_mode_change(const char *report_mode) {
	int errsv = 0;

	if (admin_property_change(CONFIG_USCHED_DIR_STAT, CONFIG_USCHED_FILE_STAT_REPORT_MODE, report_mode) < 0) {
		errsv = errno;
		log_crit("stat_admin_report_mode_change(): admin_property_change(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	if (stat_admin_report_mode_show() < 0) {
		errsv = errno;
		log_crit("stat_admin_report_mode_change(): stat_admin_report_mode_show(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	return 0;
}

