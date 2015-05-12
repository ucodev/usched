/**
 * @file config.c
 * @brief uSched
 *        Configuration interface - Daemon
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
#include "runtime.h"
#include "log.h"


int config_daemon_init(void) {
	int errsv = 0;
	struct usched_config *config = &rund.config;

	memset(config, 0, sizeof(struct usched_config));

	if (config_init_auth(&config->auth) < 0) {
		errsv = errno;
		log_warn("config_daemon_init(): config_init_auth(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	if (config_init_core(&config->core) < 0) {
		errsv = errno;
		log_warn("config_daemon_init(): config_init_core(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	if (config_init_exec(&config->exec) < 0) {
		errsv = errno;
		log_warn("config_daemon_init(): config_init_exec(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	if (config_init_ipc(&config->ipc) < 0) {
		errsv = errno;
		log_warn("config_daemon_init(): config_init_ipc(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	if (config_init_network(&config->network) < 0) {
		errsv = errno;
		log_warn("config_daemon_init(): config_init_network(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	if (config_init_stat(&config->stat) < 0) {
		errsv = errno;
		log_warn("config_daemon_init(): config_init_stat(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	if (config_init_users(&config->users) < 0) {
		errsv = errno;
		log_warn("config_daemon_init(): config_init_users(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	return 0;
}

void config_daemon_destroy(void) {
	struct usched_config *config = &rund.config;

	config_destroy_auth(&config->auth);
	config_destroy_core(&config->core);
	config_destroy_exec(&config->exec);
	config_destroy_ipc(&config->ipc);
	config_destroy_network(&config->network);
	config_destroy_stat(&config->stat);
	config_destroy_users(&config->users);

	memset(config, 0, sizeof(struct usched_config));
}

