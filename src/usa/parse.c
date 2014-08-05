/**
 * @file parse.c
 * @brief uSched
 *        Parser interface - Admin
 *
 * Date: 05-08-2014
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "runtime.h"
#include "mm.h"
#include "log.h"
#include "usched.h"


struct usched_admin_request *parse_admin_request_array(int argc, char **argv) {
	int errsv = 0;
	struct usched_admin_request *req = NULL;

	if (argc < 1) {
		errno = EINVAL;
		return NULL;
	}

	if (!(req = mm_alloc(sizeof(struct usched_admin_request)))) {
		errsv = errno;
		log_warn("parse_admin_request_array(): mm_alloc(): %s\n", strerror(errno));
		errno = errsv;
		return NULL;
	}

	memset(req, 0, sizeof(struct usched_admin_request));

	/* TODO */
	return req;
}

