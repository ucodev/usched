/**
 * @file runtime.c
 * @brief uSched
 *        Runtime handlers interface - Client
 *
 * Date: 21-08-2014
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
#include "mm.h"
#include "conn.h"
#include "usage.h"
#include "parse.h"
#include "op.h"
#include "log.h"
#include "pool.h"
#include "bitops.h"


int runtime_client_init(int argc, char **argv) {
	int errsv = 0, opt_index = 0;

	memset(&runc, 0, sizeof(struct usched_runtime_client));

	runc.argc = argc;
	runc.argv = argv;
	runc.t = time(NULL);

	/* Initialize logging interface */
	if (log_client_init() < 0) {
		errsv = errno;
		debug_printf(DEBUG_CRIT, "runtime_client_init(): log_client_init(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Initialize configuration interface */
	if (config_client_init() < 0) {
		errsv = errno;
		log_crit("runtime_client_init(): config_client_init(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Parse command line options */
	if ((opt_index = opt_client_process(argc, argv, &runc.opt)) < 0) {
		errsv = errno;
		log_crit("runtime_client_init(): opt_client_process(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Check if this is a help request (opt_index is 0) */
	if (!opt_index) {
		config_client_destroy();
		log_destroy();
		exit(EXIT_SUCCESS);
	}

	debug_printf(DEBUG_INFO, "opt_index: %d\n", opt_index);

	/* Parse requested command line instruction */
	if (!(runc.req = parse_client_instruction_array(argc - opt_index, &argv[opt_index]))) {
		errsv = errno;
		usage_client_show();
		errno = errsv;
		return -1;
	}

	/* Initialize pools */
	if (pool_client_init() < 0) {
		errsv = errno;
		log_crit("runtime_client_init(): pool_client_init(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Process requested operations */
	if (op_client_process() < 0) {
		errsv = errno;
		log_crit("runtime_client_init(): op_client_process(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Initialize client connection handlers */
	if (conn_client_init() < 0) {
		errsv = errno;
		log_crit("runtime_client_init(): conn_client_init(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* All good */
	return 0;
}

int runtime_client_lib_init(void) {
	memset(&runc, 0, sizeof(struct usched_runtime_client));

	runc.t = time(NULL);

	/* This is a client library */
	bit_set(&runc.flags, USCHED_RUNTIME_FLAG_LIB);

	/* Initialize client connection handlers */
	if (conn_client_init() < 0)
		return -1;

	/* All good */
	return 0;
}

int runtime_client_interrupted(void) {
	return 0;
}

void runtime_client_destroy(void) {
	/* Destroy connection interface */
	conn_client_destroy();

	/* Destroy pools */
	pool_client_destroy();

	/* Destroy requests */
	parse_client_req_destroy(runc.req);
	runc.req = NULL;

	/* Destroy configuration */
	config_client_destroy();

	/* Destroy usage interface */
	if (runc.usage_err_offending) {
		mm_free(runc.usage_err_offending);
		runc.usage_err_offending = NULL;
	}

	/* Destroy logging interface */
	log_destroy();
}

void runtime_client_lib_destroy(void) {
	/* Destroy connection interface */
	conn_client_destroy();

	/* Destroy pools */
	pool_client_destroy();

	/* Destroy requests */
	parse_client_req_destroy(runc.req);
	runc.req = NULL;

	/* Destroy configuration */
	config_client_destroy();

	/* Destroy usage interface */
	if (runc.usage_err_offending)
		mm_free(runc.usage_err_offending);

	/* Destroy log interface */
	log_destroy();
}

