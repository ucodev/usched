/**
 * @file ipc.c
 * @brief uSched
 *        IPC configuration and administration interface
 *
 * Date: 13-05-2015
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
#include "usched.h"
#include "admin.h"
#include "log.h"
#include "mm.h"
#include "usched.h"
#include "ipc.h"

int ipc_admin_commit(void) {
	int errsv = 0;

	/* Check if services are running and stop processing if they are */
	if (fsop_path_exists(CONFIG_USCHED_DAEMON_PID_FILE)) {
		log_crit("ipc_admin_commit(): uSched services are running (usd). You must stop all uSched services before you can commit core configuration changes.\n");
		errno = EBUSY;
		return -1;
	}

	if (fsop_path_exists(CONFIG_USCHED_EXEC_PID_FILE)) {
		log_crit("ipc_admin_commit(): uSched services are running (use). You must stop all uSched services before you can commit exec configuration changes.\n");
		errno = EBUSY;
		return -1;
	}

	if (fsop_path_exists(CONFIG_USCHED_STAT_PID_FILE)) {
		log_crit("ipc_admin_commit(): uSched services are running (uss). You must stop all uSched services before you can commit stat configuration changes.\n");
		errno = EBUSY;
		return -1;
	}

	if (fsop_path_exists(CONFIG_USCHED_IPC_PID_FILE)) {
		log_crit("ipc_admin_commit(): uSched services are running (usi). You must stop all uSched services before you can commit ipc configuration changes.\n");
		errno = EBUSY;
		return -1;
	}

	/* Destroy the current configuration */
	config_admin_destroy();

	/* auth.key */
	if (fsop_cp(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_IPC "/." CONFIG_USCHED_FILE_IPC_AUTH_KEY, CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_IPC "/" CONFIG_USCHED_FILE_IPC_AUTH_KEY, 128) < 0) {
		errsv = errno;
		log_crit("ipc_admin_commit(): fsop_cp(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* id.key */
	if (fsop_cp(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_IPC "/." CONFIG_USCHED_FILE_IPC_ID_KEY, CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_IPC "/" CONFIG_USCHED_FILE_IPC_ID_KEY, 128) < 0) {
		errsv = errno;
		log_crit("ipc_admin_commit(): fsop_cp(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* id.name */
	if (fsop_cp(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_IPC "/." CONFIG_USCHED_FILE_IPC_ID_NAME, CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_IPC "/" CONFIG_USCHED_FILE_IPC_ID_NAME, 128) < 0) {
		errsv = errno;
		log_crit("ipc_admin_commit(): fsop_cp(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* msg.max */
	if (fsop_cp(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_IPC "/." CONFIG_USCHED_FILE_IPC_MSG_MAX, CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_IPC "/" CONFIG_USCHED_FILE_IPC_MSG_MAX, 128) < 0) {
		errsv = errno;
		log_crit("ipc_admin_commit(): fsop_cp(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* msg.size */
	if (fsop_cp(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_IPC "/." CONFIG_USCHED_FILE_IPC_MSG_SIZE, CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_IPC "/" CONFIG_USCHED_FILE_IPC_MSG_SIZE, 128) < 0) {
		errsv = errno;
		log_crit("ipc_admin_commit(): fsop_cp(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* jail.dir */
	if (fsop_cp(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_IPC "/." CONFIG_USCHED_FILE_IPC_JAIL_DIR, CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_IPC "/" CONFIG_USCHED_FILE_IPC_JAIL_DIR, 128) < 0) {
		errsv = errno;
		log_crit("ipc_admin_commit(): fsop_cp(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* privdrop.user */
	if (fsop_cp(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_IPC "/." CONFIG_USCHED_FILE_IPC_PRIVDROP_USER, CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_IPC "/" CONFIG_USCHED_FILE_IPC_PRIVDROP_USER, 128) < 0) {
		errsv = errno;
		log_crit("ipc_admin_commit(): fsop_cp(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* privdrop.group */
	if (fsop_cp(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_IPC "/." CONFIG_USCHED_FILE_IPC_PRIVDROP_GROUP, CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_IPC "/" CONFIG_USCHED_FILE_IPC_PRIVDROP_GROUP, 128) < 0) {
		errsv = errno;
		log_crit("ipc_admin_commit(): fsop_cp(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Re-initialize the configuration */
	if (config_admin_init() < 0) {
		errsv = errno;
		log_crit("ipc_admin_commit(): config_admin_init(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* All good */
	return 0;
}

int ipc_admin_rollback(void) {
	int errsv = 0;

	/* auth.key */
	if (fsop_cp(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_IPC "/" CONFIG_USCHED_FILE_IPC_AUTH_KEY, CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_IPC "/." CONFIG_USCHED_FILE_IPC_AUTH_KEY, 128) < 0) {
		errsv = errno;
		log_crit("ipc_admin_rollback(): fsop_cp(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* id.key */
	if (fsop_cp(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_IPC "/" CONFIG_USCHED_FILE_IPC_ID_KEY, CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_IPC "/." CONFIG_USCHED_FILE_IPC_ID_KEY, 128) < 0) {
		errsv = errno;
		log_crit("ipc_admin_rollback(): fsop_cp(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* id.name */
	if (fsop_cp(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_IPC "/" CONFIG_USCHED_FILE_IPC_ID_NAME, CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_IPC "/." CONFIG_USCHED_FILE_IPC_ID_NAME, 128) < 0) {
		errsv = errno;
		log_crit("ipc_admin_rollback(): fsop_cp(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* msg.max */
	if (fsop_cp(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_IPC "/" CONFIG_USCHED_FILE_IPC_MSG_MAX, CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_IPC "/." CONFIG_USCHED_FILE_IPC_MSG_MAX, 128) < 0) {
		errsv = errno;
		log_crit("ipc_admin_rollback(): fsop_cp(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* msg.size */
	if (fsop_cp(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_IPC "/" CONFIG_USCHED_FILE_IPC_MSG_SIZE, CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_IPC "/." CONFIG_USCHED_FILE_IPC_MSG_SIZE, 128) < 0) {
		errsv = errno;
		log_crit("ipc_admin_rollback(): fsop_cp(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* jail.dir */
	if (fsop_cp(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_IPC "/" CONFIG_USCHED_FILE_IPC_JAIL_DIR, CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_IPC "/." CONFIG_USCHED_FILE_IPC_JAIL_DIR, 128) < 0) {
		errsv = errno;
		log_crit("ipc_admin_rollback(): fsop_cp(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* privdrop.user */
	if (fsop_cp(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_IPC "/" CONFIG_USCHED_FILE_IPC_PRIVDROP_USER, CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_IPC "/." CONFIG_USCHED_FILE_IPC_PRIVDROP_USER, 128) < 0) {
		errsv = errno;
		log_crit("ipc_admin_rollback(): fsop_cp(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* privdrop.group */
	if (fsop_cp(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_IPC "/" CONFIG_USCHED_FILE_IPC_PRIVDROP_GROUP, CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_IPC "/." CONFIG_USCHED_FILE_IPC_PRIVDROP_GROUP, 128) < 0) {
		errsv = errno;
		log_crit("ipc_admin_rollback(): fsop_cp(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* All good */
	return 0;
}

int ipc_admin_show(void) {
	int errsv = 0;

	if (ipc_admin_auth_key_show() < 0) {
		errsv = errno;
		log_crit("ipc_admin_show(): ipc_admin_auth_key_show(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	if (ipc_admin_id_key_show() < 0) {
		errsv = errno;
		log_crit("ipc_admin_show(): ipc_admin_id_key_show(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	if (ipc_admin_id_name_show() < 0) {
		errsv = errno;
		log_crit("ipc_admin_show(): ipc_admin_id_name_show(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	if (ipc_admin_msg_max_show() < 0) {
		errsv = errno;
		log_crit("ipc_admin_show(): ipc_admin_msg_max_show(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	if (ipc_admin_msg_size_show() < 0) {
		errsv = errno;
		log_crit("ipc_admin_show(): ipc_admin_msg_size_show(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	if (ipc_admin_jail_dir_show() < 0) {
		errsv = errno;
		log_crit("ipc_admin_show(): ipc_admin_jail_dir_show(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	if (ipc_admin_privdrop_user_show() < 0) {
		errsv = errno;
		log_crit("ipc_admin_show(): ipc_admin_privdrop_user_show(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	if (ipc_admin_privdrop_group_show() < 0) {
		errsv = errno;
		log_crit("ipc_admin_show(): ipc_admin_privdrop_group_show(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	return 0;
}

int ipc_admin_auth_key_show(void) {
	int errsv = 0;

	if (admin_property_show(CONFIG_USCHED_DIR_IPC, USCHED_CATEGORY_IPC_STR, CONFIG_USCHED_FILE_IPC_AUTH_KEY) < 0) {
		errsv = errno;
		log_crit("ipc_admin_auth_key_show(): admin_property_show(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* All good */
	return 0;
}

int ipc_admin_auth_key_change(const char *auth_key) {
	int errsv = 0;

	if (admin_property_change(CONFIG_USCHED_DIR_IPC, CONFIG_USCHED_FILE_IPC_AUTH_KEY, auth_key) < 0) {
		errsv = errno;
		log_crit("ipc_admin_auth_key_change(): admin_property_change(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	if (ipc_admin_auth_key_show() < 0) {
		errsv = errno;
		log_crit("ipc_admin_auth_key_change(): ipc_admin_auth_key_show(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	return 0;
}

int ipc_admin_id_key_show(void) {
	int errsv = 0;

	if (admin_property_show(CONFIG_USCHED_DIR_IPC, USCHED_CATEGORY_IPC_STR, CONFIG_USCHED_FILE_IPC_ID_KEY) < 0) {
		errsv = errno;
		log_crit("ipc_admin_id_key_show(): admin_property_show(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* All good */
	return 0;
}

int ipc_admin_id_key_change(const char *id_key) {
	int errsv = 0;

	if (admin_property_change(CONFIG_USCHED_DIR_IPC, CONFIG_USCHED_FILE_IPC_ID_KEY, id_key) < 0) {
		errsv = errno;
		log_crit("ipc_admin_id_key_change(): admin_property_change(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	if (ipc_admin_id_key_show() < 0) {
		errsv = errno;
		log_crit("ipc_admin_id_key_change(): ipc_admin_id_key_show(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	return 0;
}

int ipc_admin_id_name_show(void) {
	int errsv = 0;

	if (admin_property_show(CONFIG_USCHED_DIR_IPC, USCHED_CATEGORY_IPC_STR, CONFIG_USCHED_FILE_IPC_ID_NAME) < 0) {
		errsv = errno;
		log_crit("ipc_admin_id_name_show(): admin_property_show(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* All good */
	return 0;
}

int ipc_admin_id_name_change(const char *id_name) {
	int errsv = 0;

	if (admin_property_change(CONFIG_USCHED_DIR_IPC, CONFIG_USCHED_FILE_IPC_ID_NAME, id_name) < 0) {
		errsv = errno;
		log_crit("ipc_admin_id_name_change(): admin_property_change(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	if (ipc_admin_id_name_show() < 0) {
		errsv = errno;
		log_crit("ipc_admin_id_name_change(): ipc_admin_id_name_show(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	return 0;
}


int ipc_admin_msg_max_show(void) {
	int errsv = 0;

	if (admin_property_show(CONFIG_USCHED_DIR_IPC, USCHED_CATEGORY_IPC_STR, CONFIG_USCHED_FILE_IPC_MSG_MAX) < 0) {
		errsv = errno;
		log_crit("ipc_admin_msg_max_show(): admin_property_show(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* All good */
	return 0;
}

int ipc_admin_msg_max_change(const char *msg_max) {
	int errsv = 0;

	if (admin_property_change(CONFIG_USCHED_DIR_IPC, CONFIG_USCHED_FILE_IPC_MSG_MAX, msg_max) < 0) {
		errsv = errno;
		log_crit("ipc_admin_msg_max_change(): admin_property_change(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	if (ipc_admin_msg_max_show() < 0) {
		errsv = errno;
		log_crit("ipc_admin_msg_max_change(): ipc_admin_msg_max_show(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	return 0;
}

int ipc_admin_msg_size_show(void) {
	int errsv = 0;

	if (admin_property_show(CONFIG_USCHED_DIR_IPC, USCHED_CATEGORY_IPC_STR, CONFIG_USCHED_FILE_IPC_MSG_SIZE) < 0) {
		errsv = errno;
		log_crit("ipc_admin_msg_size_show(): admin_property_show(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* All good */
	return 0;
}

int ipc_admin_msg_size_change(const char *msg_size) {
	int errsv = 0;

	if (admin_property_change(CONFIG_USCHED_DIR_IPC, CONFIG_USCHED_FILE_IPC_MSG_SIZE, msg_size) < 0) {
		errsv = errno;
		log_crit("ipc_admin_msg_size_change(): admin_property_change(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	if (ipc_admin_msg_size_show() < 0) {
		errsv = errno;
		log_crit("ipc_admin_msg_size_change(): ipc_admin_msg_size_show(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	return 0;
}

int ipc_admin_jail_dir_show(void) {
	int errsv = 0;

	if (admin_property_show(CONFIG_USCHED_DIR_IPC, USCHED_CATEGORY_IPC_STR, CONFIG_USCHED_FILE_IPC_JAIL_DIR) < 0) {
		errsv = errno;
		log_crit("ipc_admin_jail_dir_show(): admin_property_show(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* All good */
	return 0;
}

int ipc_admin_jail_dir_change(const char *jail_dir) {
	int errsv = 0;

	if (admin_property_change(CONFIG_USCHED_DIR_IPC, CONFIG_USCHED_FILE_IPC_JAIL_DIR, jail_dir) < 0) {
		errsv = errno;
		log_crit("ipc_admin_jail_dir_change(): admin_property_change(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	if (ipc_admin_jail_dir_show() < 0) {
		errsv = errno;
		log_crit("ipc_admin_jail_dir_change(): ipc_admin_jail_dir_show(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	return 0;
}

int ipc_admin_privdrop_user_show(void) {
	int errsv = 0;

	if (admin_property_show(CONFIG_USCHED_DIR_IPC, USCHED_CATEGORY_IPC_STR, CONFIG_USCHED_FILE_IPC_PRIVDROP_USER) < 0) {
		errsv = errno;
		log_crit("ipc_admin_privdrop_user_show(): admin_property_show(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* All good */
	return 0;
}

int ipc_admin_privdrop_user_change(const char *privdrop_user) {
	int errsv = 0;

	if (admin_property_change(CONFIG_USCHED_DIR_IPC, CONFIG_USCHED_FILE_IPC_PRIVDROP_USER, privdrop_user) < 0) {
		errsv = errno;
		log_crit("ipc_admin_privdrop_user_change(): admin_property_change(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	if (ipc_admin_privdrop_user_show() < 0) {
		errsv = errno;
		log_crit("ipc_admin_privdrop_user_change(): ipc_admin_privdrop_user_show(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	return 0;
}

int ipc_admin_privdrop_group_show(void) {
	int errsv = 0;

	if (admin_property_show(CONFIG_USCHED_DIR_IPC, USCHED_CATEGORY_IPC_STR, CONFIG_USCHED_FILE_IPC_PRIVDROP_GROUP) < 0) {
		errsv = errno;
		log_crit("ipc_admin_privdrop_group_show(): admin_property_show(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* All good */
	return 0;
}

int ipc_admin_privdrop_group_change(const char *privdrop_group) {
	int errsv = 0;

	if (admin_property_change(CONFIG_USCHED_DIR_IPC, CONFIG_USCHED_FILE_IPC_PRIVDROP_GROUP, privdrop_group) < 0) {
		errsv = errno;
		log_crit("ipc_admin_privdrop_group_change(): admin_property_change(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	if (ipc_admin_privdrop_group_show() < 0) {
		errsv = errno;
		log_crit("ipc_admin_privdrop_group_change(): ipc_admin_privdrop_group_show(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	return 0;
}

