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


#include "usched.h"
#include "runtime.h"
#include "logic.h"

int op_client_process(void) {
	if (!runc.req)
		return -1;

	runc.op = runc.req->op;

	switch (runc.op) {
		case USCHED_OP_RUN:	return logic_process_run();
		case USCHED_OP_STOP:	return logic_process_stop();
		case USCHED_OP_SHOW:	return logic_process_show();
	}

	return -1;
}

