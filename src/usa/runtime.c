/**
 * @file runtime.c
 * @brief uSched
 *        Runtime handlers interface - Admin
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


#include <string.h>
#include <stdlib.h>
#include <time.h>

#include <pall/cll.h>

#include "config.h"
#include "debug.h"
#include "runtime.h"
#include "usage.h"
#include "parse.h"
#include "print.h"
#include "log.h"


int runtime_admin_init(int argc, char **argv) {
	int errsv = 0;

	/* Pre-check */
	if (getuid()) {
		print_admin_no_priv();
		return -1;
	}

	memset(&runa, 0, sizeof(struct usched_runtime_client));

	runa.argc = argc;
	runa.argv = argv;

	/* Initialize logging interface */
	if (log_admin_init() < 0) {
		errsv = errno;
		debug_printf(DEBUG_CRIT, "runtime_admin_init(): log_admin_init(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Initialize configuration interface */
	if (config_admin_init() < 0) {
		errsv = errno;
		log_crit("runtime_admin_init(): config_admin_init(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Parse requested command line operation */
	if (!(runa.req = parse_admin_request_array(argc - 1, &argv[1]))) {
		errsv = errno;
		usage_admin_show();
		errno = errsv;
		return -1;
	}

	/* All good */
	return 0;
}

int runtime_admin_interrupted(void) {
	return 0;
}

void runtime_admin_destroy(void) {
	/* Destroy request */
	parse_admin_req_destroy(runa.req);

	/* Destroy configuration interface */
	config_admin_destroy();

	/* Destroy usage interface */
	if (runa.usage_err_offending)
		mm_free(runa.usage_err_offending);

	/* Destroy logging interface */
	log_destroy();
}

