/**
 * @file config.c
 * @brief uSched
 *        Configuration interface
 *
 * Date: 31-07-2014
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

#include <string.h>
#include <errno.h>

#include "config.h"
#include "log.h"


static int _config_init_auth(struct usched_config_auth *auth) {
	errno = ENOSYS;
	return -1;
}

static int _config_init_core(struct usched_config_core *core) {
	errno = ENOSYS;
	return -1;
}

static int _config_init_network(struct usched_config_network *network) {
	errno = ENOSYS;
	return -1;
}

static int _config_init_users(struct usched_config_users *users) {
	errno = ENOSYS;
	return -1;
}

static void _config_destroy_auth(struct usched_config_auth *auth) {
	return ;
}

static void _config_destroy_core(struct usched_config_core *core) {
	return ;
}

static void _config_destroy_network(struct usched_config_network *network) {
	return ;
}

static void _config_destroy_users(struct usched_config_users *users) {
	return ;
}

int config_init(struct usched_config *config) {
	int errsv = 0;

	if (_config_init_auth(&config->auth) < 0) {
		errsv = errno;
		log_warn("config_init(): _config_read_auth(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	if (_config_init_core(&config->core) < 0) {
		errsv = errno;
		log_warn("config_init(): _config_read_core(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	if (_config_init_network(&config->network) < 0) {
		errsv = errno;
		log_warn("config_init(): _config_read_network(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	if (_config_init_users(&config->users) < 0) {
		errsv = errno;
		log_warn("config_init(): _config_read_users(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	return 0;
}

void config_destroy(struct usched_config *config) {
	_config_destroy_auth(&config->auth);
	_config_destroy_core(&config->core);
	_config_destroy_network(&config->network);
	_config_destroy_users(&config->users);
}

