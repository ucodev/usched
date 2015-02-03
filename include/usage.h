/**
 * @file usage.h
 * @brief uSched
 *        Usage handlers interface header
 *
 * Date: 03-02-2015
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


#ifndef USCHED_USAGE_H
#define USCHED_USAGE_H

/* Usage error type strings */
#define USCHED_USAGE_ADMIN_ERR_INVALID_OP_STR			"Invalid operation"
#define USCHED_USAGE_ADMIN_ERR_INVALID_CATEGORY_STR		"Invalid category"
#define USCHED_USAGE_ADMIN_ERR_INVALID_COMPONENT_STR		"Invalid component"
#define USCHED_USAGE_ADMIN_ERR_INVALID_PROPERTY_STR		"Invalid property"
#define USCHED_USAGE_ADMIN_ERR_INVALID_ARG_STR			"Invalid argument"
#define USCHED_USAGE_ADMIN_ERR_INSUFF_ARGS_STR			"Insufficient arguments"
#define USCHED_USAGE_ADMIN_ERR_TOOMANY_ARGS_STR			"Too many arguments"

#define USCHED_USAGE_CLIENT_ERR_INVALID_OP_STR			"Invalid operation"
#define USCHED_USAGE_CLIENT_ERR_INVALID_PREP_STR		"Invalid preposition"
#define USCHED_USAGE_CLIENT_ERR_INVALID_ADVERB_STR		"Invalid adverbial of time"
#define USCHED_USAGE_CLIENT_ERR_INVALID_CONJ_STR		"Invalid conjunction"
#define USCHED_USAGE_CLIENT_ERR_INVALID_ARG_STR			"Invalid argument"
#define USCHED_USAGE_CLIENT_ERR_UNEXPECT_PREP_STR		"Unexpected preposition"
#define USCHED_USAGE_CLIENT_ERR_UNEXPECT_CONJ_STR		"Unexpected conjunction"
#define USCHED_USAGE_CLIENT_ERR_INSUFF_ARGS_STR			"Insufficient arguments"
#define USCHED_USAGE_CLIENT_ERR_TOOMANY_ARGS_STR		"Too many arguments"

/* Usage error type values */
typedef enum USAGE_ADMIN_ERROR {
	USCHED_USAGE_ADMIN_ERR_INVALID_OP = 1,
	USCHED_USAGE_ADMIN_ERR_INVALID_CATEGORY,
	USCHED_USAGE_ADMIN_ERR_INVALID_COMPONENT,
	USCHED_USAGE_ADMIN_ERR_INVALID_PROPERTY,
	USCHED_USAGE_ADMIN_ERR_INVALID_ARG,
	USCHED_USAGE_ADMIN_ERR_INSUFF_ARGS,
	USCHED_USAGE_ADMIN_ERR_TOOMANY_ARGS
} usched_usage_admin_err_t;


typedef enum USAGE_CLIENT_ERROR {
	USCHED_USAGE_CLIENT_ERR_INVALID_OP = 1,
	USCHED_USAGE_CLIENT_ERR_INVALID_PREP,
	USCHED_USAGE_CLIENT_ERR_INVALID_ADVERB,
	USCHED_USAGE_CLIENT_ERR_INVALID_CONJ,
	USCHED_USAGE_CLIENT_ERR_INVALID_ARG,
	USCHED_USAGE_CLIENT_ERR_UNEXPECT_PREP,
	USCHED_USAGE_CLIENT_ERR_UNEXPECT_CONJ,
	USCHED_USAGE_CLIENT_ERR_INSUFF_ARGS,
	USCHED_USAGE_CLIENT_ERR_TOOMANY_ARGS
} usched_usage_client_err_t;


/* Prototypes */
void usage_admin_show(void);
void usage_admin_error_set(usched_usage_admin_err_t err, char *offending);
void usage_client_show(void);
void usage_client_error_set(usched_usage_client_err_t err, char *offending);

#endif
