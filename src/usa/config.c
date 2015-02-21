/**
 * @file config.c
 * @brief uSched
 *        Configuration interface - Admin
 *
 * Date: 21-02-2015
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

int config_admin_init(void) {
	struct usched_config *config = &runa.config;

	memset(config, 0, sizeof(struct usched_config));

	return 0;
}

void config_admin_destroy(void) {
	struct usched_config *config = &runa.config;

	memset(config, 0, sizeof(struct usched_config));
}
