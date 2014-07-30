/**
 * @file op.c
 * @brief uSched
 *        Operation Handling interface
 *
 * Date: 24-06-2014
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

#include "usched.h"
#include "runtime.h"
#include "log.h"
#include "logic.h"

int op_client_process(void) {
	int errsv = 0, ret = -1;

	if (!runc.req) {
		errno = EINVAL;
		return -1;
	}

	runc.op = runc.req->op;

	switch (runc.op) {
		case USCHED_OP_RUN:
			ret = logic_process_run();

			if (ret < 0) {
				errsv = errno;
				log_warn("op_client_process(): logic_process_run(): %s\n", strerror(errno));
			}

			break;
		case USCHED_OP_STOP:
			ret = logic_process_stop();

			if (ret < 0) {
				errsv = errno;
				log_warn("op_client_process(): logic_process_run(): %s\n", strerror(errno));
			}

			break;
		case USCHED_OP_SHOW:
			ret = logic_process_show();

			if (ret < 0) {
				errsv = errno;
				log_warn("op_client_process(): logic_process_run(): %s\n", strerror(errno));
			}

			break;
		default: errsv = EINVAL;
	}

	errno = errsv;

	return ret;
}

