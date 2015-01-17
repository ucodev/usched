/**
 * @file lib.c
 * @brief uSched
 *        uSched Client Library interface
 *
 * Date: 17-01-2015
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

#include <errno.h>

#include "config.h"
#include "runtime.h"
#include "conn.h"
#include "usage.h"
#include "op.h"
#include "pool.h"
#include "parse.h"

static int _init(void) {
	if (runtime_client_lib_init() < 0)
		return -1;

	return 0;
}

static int _do(char *req) {
	/* Reset runtime data */
	if (runtime_client_lib_reset() < 0)
		return -1;

	/* Parse and process data
	 *
	 * TODO: Set a unique runtime flag for each type of error in order to be returned by usched_error().
	 *
	 */
	runc.req_str = req;

	if (!(runc.req = parse_client_instruction(runc.req_str)))
		return -1;

	/* Initialize client connection handlers */
	if (conn_client_init() < 0)
		return -1;

	if (pool_client_init() < 0)
		return -1;

	if (op_client_process() < 0)
		return -1;

	if (conn_client_process() < 0)
		return -1;

	return 0;
}

static void _destroy(void) {
	runtime_client_lib_destroy();
}

/* Library Interface */
#ifdef COMPILE_WIN32
DLLIMPORT
#endif
int usched_init(void) {
	return _init();
}

#ifdef COMPILE_WIN32
DLLIMPORT
#endif
int usched_request(char *req) {
	return _do(req);
}

#ifdef COMPILE_WIN32
DLLIMPORT
#endif
int usched_opt_set_remote_hostname(char *hostname) {
	if (strlen(hostname) >= sizeof(runc.opt.remote_hostname)) {
		errno = EINVAL;
		return -1;
	}

	strcpy(runc.opt.remote_hostname, hostname);

	return 0;
}

#ifdef COMPILE_WIN32
DLLIMPORT
#endif
int usched_opt_set_remote_port(char *port) {
	if (strlen(port) >= sizeof(runc.opt.remote_port)) {
		errno = EINVAL;
		return -1;
	}

	strcpy(runc.opt.remote_port, port);

	return 0;
}

#ifdef COMPILE_WIN32
DLLIMPORT
#endif
int usched_opt_set_remote_username(char *username) {
	if (strlen(username) >= sizeof(runc.opt.remote_username)) {
		errno = EINVAL;
		return -1;
	}

	strcpy(runc.opt.remote_username, username);

	return 0;
}

#ifdef COMPILE_WIN32
DLLIMPORT
#endif
int usched_opt_set_remote_password(char *password) {
	if (strlen(password) >= sizeof(runc.opt.remote_password)) {
		errno = EINVAL;
		return -1;
	}

	strcpy(runc.opt.remote_password, password);

	return 0;
}

#ifdef COMPILE_WIN32
DLLIMPORT
#endif
void usched_result_get_run(uint64_t **entry_list, size_t *nmemb) {
	*entry_list = runc.result;
	*nmemb = runc.result_nmemb;
}

#ifdef COMPILE_WIN32
DLLIMPORT
#endif
void usched_result_get_stop(uint64_t **entry_list, size_t *nmemb) {
	*entry_list = runc.result;
	*nmemb = runc.result_nmemb;
}

#ifdef COMPILE_WIN32
DLLIMPORT
#endif
void usched_result_get_show(struct usched_entry **entry_list, size_t *nmemb) {
	*entry_list = runc.result;
	*nmemb = runc.result_nmemb;
}

#ifdef COMPILE_WIN32
DLLIMPORT
#endif
void usched_result_free_run(void) {
	mm_free(runc.result);
	runc.result = NULL;
	runc.result_nmemb = 0;
}

#ifdef COMPILE_WIN32
DLLIMPORT
#endif
void usched_result_free_stop(void) {
	mm_free(runc.result);
	runc.result = NULL;
	runc.result_nmemb = 0;
}

#ifdef COMPILE_WIN32
DLLIMPORT
#endif
void usched_result_free_show(void) {
	int i = 0;
	struct usched_entry *entry_list = runc.result;

	for (i = runc.result_nmemb - 1; i >= 0; i --) {
		if (entry_list[i].subj)
			mm_free(entry_list[i].subj);
	}

	mm_free(entry_list);

	runc.result = NULL;
	runc.result_nmemb = 0;
}

#ifdef COMPILE_WIN32
DLLIMPORT
#endif
usched_usage_client_err_t usched_usage_error(void) {
	return runc.usage_err;
}

#ifdef COMPILE_WIN32
DLLIMPORT
#endif
char *usched_usage_error_str(usched_usage_client_err_t error) {
	switch (error) {
		case USCHED_USAGE_CLIENT_ERR_INVALID_OP: return USCHED_USAGE_CLIENT_ERR_INVALID_OP_STR;
		case USCHED_USAGE_CLIENT_ERR_INVALID_PREP: return USCHED_USAGE_CLIENT_ERR_INVALID_PREP_STR;
		case USCHED_USAGE_CLIENT_ERR_INVALID_ADVERB: return USCHED_USAGE_CLIENT_ERR_INVALID_ADVERB_STR;
		case USCHED_USAGE_CLIENT_ERR_INVALID_CONJ: return USCHED_USAGE_CLIENT_ERR_INVALID_CONJ_STR;
		case USCHED_USAGE_CLIENT_ERR_INVALID_ARG: return USCHED_USAGE_CLIENT_ERR_INVALID_ARG_STR;
		case USCHED_USAGE_CLIENT_ERR_UNEXPECT_PREP: return USCHED_USAGE_CLIENT_ERR_UNEXPECT_PREP_STR;
		case USCHED_USAGE_CLIENT_ERR_UNEXPECT_CONJ: return USCHED_USAGE_CLIENT_ERR_UNEXPECT_CONJ_STR;
		case USCHED_USAGE_CLIENT_ERR_INSUFF_ARGS: return USCHED_USAGE_CLIENT_ERR_INSUFF_ARGS_STR;
		case USCHED_USAGE_CLIENT_ERR_TOOMANY_ARGS: return USCHED_USAGE_CLIENT_ERR_TOOMANY_ARGS_STR;
	}

	return "Success";
}

#ifdef COMPILE_WIN32
DLLIMPORT
#endif
void usched_destroy(void) {
	_destroy();
}

