/**
 * @file parse.h
 * @brief uSched
 *        Parser interface header
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


#ifndef USCHED_PARSE_H
#define USCHED_PARSE_H

#include "usched.h"

/* Prototypes */
struct usched_admin_request *parse_admin_request_array(int argc, char **argv);
void parse_admin_req_destroy(struct usched_admin_request *req);
struct usched_client_request *parse_instruction_array(int argc, char **argv);
struct usched_client_request *parse_instruction(const char *cmd);
void parse_client_req_destroy(struct usched_client_request *req);

#endif
