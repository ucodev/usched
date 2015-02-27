/**
 * @file parse.c
 * @brief uSched
 *        Parser interface - Admin
 *
 * Date: 27-02-2015
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
#include <strings.h>
#include <stdlib.h>
#include <errno.h>

#include "debug.h"
#include "runtime.h"
#include "mm.h"
#include "log.h"
#include "parse.h"
#include "usched.h"

static usched_op_t _parse_get_op(const char *op) {
	if (!strcasecmp(op, USCHED_OP_ADD_STR))
		return USCHED_OP_ADD;

	if (!strcasecmp(op, USCHED_OP_DELETE_STR))
		return USCHED_OP_DELETE;

	if (!strcasecmp(op, USCHED_OP_CHANGE_STR))
		return USCHED_OP_CHANGE;

	if (!strcasecmp(op, USCHED_OP_SHOW_STR))
		return USCHED_OP_SHOW;

	if (!strcasecmp(op, USCHED_OP_COMMIT_STR))
		return USCHED_OP_COMMIT;

	if (!strcasecmp(op, USCHED_OP_ROLLBACK_STR))
		return USCHED_OP_ROLLBACK;

	return -1;
}

static usched_category_t _parse_get_category(const char *category) {
	if (!strcasecmp(category, USCHED_CATEGORY_ALL_STR))
		return USCHED_CATEGORY_ALL;

	if (!strcasecmp(category, USCHED_CATEGORY_AUTH_STR))
		return USCHED_CATEGORY_AUTH;

	if (!strcasecmp(category, USCHED_CATEGORY_CORE_STR))
		return USCHED_CATEGORY_CORE;

	if (!strcasecmp(category, USCHED_CATEGORY_NETWORK_STR))
		return USCHED_CATEGORY_NETWORK;

	if (!strcasecmp(category, USCHED_CATEGORY_USERS_STR))
		return USCHED_CATEGORY_USERS;

	return -1;
}

static struct usched_admin_request *_parse_category_compound(struct usched_admin_request *req, int argc, char **argv) {
	int ret = 0;

	if ((ret = _parse_get_category(argv[0])) < 0) {
		usage_admin_error_set(USCHED_USAGE_ADMIN_ERR_INVALID_CATEGORY, argv[0]);
		goto _category_error;
	} else {
		req->category = ret;
	}

	debug_printf(DEBUG_INFO, "CATEGORY: %d\n", req->category);

	/* Set the argument vector and argument counter of the request */
	req->args = &argv[1];
	req->argc = (size_t) argc - 1;

	return req;

_category_error:
	parse_admin_req_destroy(req);

	return NULL;
}

static struct usched_admin_request *_parse_op_compound(struct usched_admin_request *req, int argc, char **argv) {
	int ret = 0;

	if ((ret = _parse_get_op(argv[0])) < 0) {
		usage_admin_error_set(USCHED_USAGE_ADMIN_ERR_INVALID_OP, argv[0]);
		goto _op_error;
	} else {
		req->op = ret;
	}

	debug_printf(DEBUG_INFO, "OP: %d\n", req->op);

	/* Evaluate argument counter validity for each operation */
	switch (req->op) {
		case USCHED_OP_DELETE:

		case USCHED_OP_ADD: {
			if (argc < 3) {
				usage_admin_error_set(USCHED_USAGE_ADMIN_ERR_INSUFF_ARGS, NULL);
				goto _op_error;
			}
		} break;

		case USCHED_OP_CHANGE: {
			if (argc < 4) {
				usage_admin_error_set(USCHED_USAGE_ADMIN_ERR_INSUFF_ARGS, NULL);
				goto _op_error;
			}
		} break;

		case USCHED_OP_SHOW: {
			if (argc < 2) {
				usage_admin_error_set(USCHED_USAGE_ADMIN_ERR_INSUFF_ARGS, NULL);
				goto _op_error;
			}
		} break;

		case USCHED_OP_COMMIT: {
			if (argc < 2) {
				usage_admin_error_set(USCHED_USAGE_ADMIN_ERR_INSUFF_ARGS, NULL);
				goto _op_error;
			}
		} break;

		case USCHED_OP_ROLLBACK: {
			if (argc < 1) {
				usage_admin_error_set(USCHED_USAGE_ADMIN_ERR_INSUFF_ARGS, NULL);
				goto _op_error;
			}
		} break;

		default: {
			usage_admin_error_set(USCHED_USAGE_ADMIN_ERR_INVALID_OP, argv[0]);
			goto _op_error;
		}
	}

	return _parse_category_compound(req, argc - 1, &argv[1]);

_op_error:
	parse_admin_req_destroy(req);

	return NULL;
}

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

	if (!_parse_op_compound(req, argc, argv))
		return NULL;

	/* All good */
	return req;
}

void parse_admin_req_destroy(struct usched_admin_request *req) {
	if (req)
		mm_free(req);
}

