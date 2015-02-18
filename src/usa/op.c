/**
 * @file op.c
 * @brief uSched
 *        Operation Handling interface - Admin
 *
 * Date: 18-02-2015
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

#include "usched.h"
#include "runtime.h"
#include "log.h"
#include "logic.h"

int op_admin_process(void) {
	int errsv = 0, ret = -1;

	if (!runa.req) {
		log_warn("op_admin_process(): Unusable request.\n");
		errno = EINVAL;
		return -1;
	}

	runa.op = runa.req->op;

	switch (runa.op) {
		case USCHED_OP_ADD:
			ret = logic_admin_process_add();

			if (ret < 0) {
				errsv = errno;
				log_warn("op_admin_process(): logic_admin_process_add(): %s\n", strerror(errno));
			}

			break;
		case USCHED_OP_DELETE:
			ret = logic_admin_process_delete();

			if (ret < 0) {
				errsv = errno;
				log_warn("op_admin_process(): logic_admin_process_delete(): %s\n", strerror(errno));
			}

			break;
		case USCHED_OP_CHANGE:
			ret = logic_admin_process_change();

			if (ret < 0) {
				errsv = errno;
				log_warn("op_admin_process(): logic_admin_process_change(): %s\n", strerror(errno));
			}

			break;
		case USCHED_OP_COMMIT:
			ret = logic_admin_process_commit();

			if (ret < 0) {
				errsv = errno;
				log_warn("op_admin_process(): logic_admin_process_commit(): %s\n", strerror(errno));
			}

			break;
		case USCHED_OP_ROLLBACK:
			ret = logic_admin_process_rollback();

			if (ret < 0) {
				errsv = errno;
				log_warn("op_admin_process(): logic_admin_process_rollback(): %s\n", strerror(errno));
			}

			break;
		case USCHED_OP_SHOW:
			ret = logic_admin_process_show();

			if (ret < 0) {
				errsv = errno;
				log_warn("op_admin_process(): logic_admin_process_show(): %s\n", strerror(errno));
			}

			break;
		default: errsv = EINVAL;
	}

	errno = errsv;

	return ret;
}

