/**
 * @file parse.c
 * @brief uSched
 *        Parser interface
 *
 * Date: 29-07-2014
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
#include <strings.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>

#include <sys/types.h>

#include "runtime.h"
#include "mm.h"
#include "debug.h"
#include "usched.h"
#include "usage.h"
#include "parse.h"

static struct usched_request *_parse_prep_compound(struct usched_request *req, int argc, char **argv);

static long _parse_req_arg(const struct usched_request *req, const char *arg) {
	char *ctf_str = NULL, *endptr = NULL;
	size_t len = 0;
	struct tm tm;
	long val = strtol(arg, &endptr, 10);

	/* Validate 'val' */
	if ((endptr == arg) || (val < 0) || (errno == ERANGE))
		return -1;

	memset(&tm, 0, sizeof(struct tm));

	if ((req->prep != USCHED_PREP_ON) && (req->prep != USCHED_PREP_TO)) {
		/* Prepositions IN and EVERY contain absolute offsets */
		switch (req->adverb) {
			case USCHED_ADVERB_SECONDS:	return val;
			case USCHED_ADVERB_MINUTES:	return val * 60;
			case USCHED_ADVERB_HOURS:	return val * 3600;
			case USCHED_ADVERB_DAYS:	return val * 86400;
			case USCHED_ADVERB_WEEKS:	return val * 86400 * 7;
			case USCHED_ADVERB_MONTHS:	return val * 86400 * 30;
			case USCHED_ADVERB_YEARS:	return val * 86400 * 365;
			case USCHED_ADVERB_DATE:	/* Invalid in this context */
			case USCHED_ADVERB_DATETIME:	/* Invalid in this context */
			case USCHED_ADVERB_TIMESTAMP:	/* Invalid in this context */
			case USCHED_ADVERB_WEEKDAYS:	/* Invalid in this context */
			case USCHED_ADVERB_TIME:	return -1; /* Invalid in this context */
		}
	} else {
		/* Prepositions ON and TO contain relative offsets */
		localtime_r(&runc.t, &tm);

		switch (req->adverb) {
			case USCHED_ADVERB_SECONDS:
				if (tm.tm_sec >= val) tm.tm_min ++;
				tm.tm_sec = val;
				return mktime(&tm) - runc.t;
			case USCHED_ADVERB_MINUTES:
				if (tm.tm_min >= val) tm.tm_hour ++;
				tm.tm_sec = 0;
				tm.tm_min = val;
				return mktime(&tm) - runc.t;
			case USCHED_ADVERB_HOURS:
				if (tm.tm_hour >= val) tm.tm_mday ++;
				tm.tm_sec = tm.tm_min = 0;
				tm.tm_hour = val;
				return mktime(&tm) - runc.t;
			case USCHED_ADVERB_DAYS:
				if (tm.tm_mday >= val) tm.tm_mon ++;
				tm.tm_sec = tm.tm_min = tm.tm_hour = 0;
				tm.tm_mday = val;
				return mktime(&tm) - runc.t;
			case USCHED_ADVERB_WEEKS:
				if ((tm.tm_yday / 7) >= val) tm.tm_year ++;
				tm.tm_sec = tm.tm_min = tm.tm_hour = 0;
				tm.tm_mon = 0;
				tm.tm_mday = val * 7;
				return mktime(&tm) - runc.t;
			case USCHED_ADVERB_MONTHS:
				if (tm.tm_mon >= val) tm.tm_year ++;
				tm.tm_sec = tm.tm_min = tm.tm_hour = 0;
				tm.tm_mday = 1;
				tm.tm_mon = val;
				return mktime(&tm) - runc.t;
			case USCHED_ADVERB_YEARS:
				if (tm.tm_year >= val) return -1;
				tm.tm_sec = tm.tm_min = tm.tm_hour = tm.tm_mon = 0;
				tm.tm_mday = 1;
				tm.tm_year = val;
				return mktime(&tm) - runc.t;
			case USCHED_ADVERB_DATE:
				if (!strptime(arg, "%Y-%m-%d", &tm)) return -1;
				tm.tm_sec = tm.tm_min = tm.tm_hour = 0;
				return mktime(&tm) - runc.t;
			case USCHED_ADVERB_DATETIME:
				return (strptime(arg, "%Y-%m-%d %H:%M:%S", &tm) ? mktime(&tm) : -1) - runc.t;
			case USCHED_ADVERB_TIMESTAMP:
				return val;
			case USCHED_ADVERB_WEEKDAYS:	/* Special case */
			case USCHED_ADVERB_TIME:	/* Special case */
			default:			break;
		}

		if (req->adverb == USCHED_ADVERB_TIME) {
			time_t t_off = 0;

			len = 4 + 1 + 2 + 1 + 2 + 1 + strlen(arg) + 2;

			if (!(ctf_str = mm_alloc(len)))
				return -1;

			memset(ctf_str, 0, len);

			snprintf(ctf_str, len - 1, "%d-%d-%d %s", 1900 + tm.tm_year, tm.tm_mon + 1, tm.tm_mday, arg);

			if (!strptime(ctf_str, "%Y-%m-%d %H:%M:%S", &tm)) {
				mm_free(ctf_str);
				return -1;
			}

			mm_free(ctf_str);

			if ((t_off = mktime(&tm)) < runc.t)
				t_off += 86400;

			return t_off - runc.t;
		} else if (req->adverb == USCHED_ADVERB_WEEKDAYS) {
			if (!strcasecmp(arg, USCHED_WEEKDAY_SUNDAY_STR) || (val == USCHED_WEEKDAY_SUNDAY)) {
				val = USCHED_WEEKDAY_SUNDAY;
			} else if (!strcasecmp(arg, USCHED_WEEKDAY_MONDAY_STR) || (val == USCHED_WEEKDAY_MONDAY)) {
				val = USCHED_WEEKDAY_MONDAY;
			} else if (!strcasecmp(arg, USCHED_WEEKDAY_TUESDAY_STR) || (val == USCHED_WEEKDAY_TUESDAY)) {
				val = USCHED_WEEKDAY_TUESDAY;
			} else if (!strcasecmp(arg, USCHED_WEEKDAY_WEDNESDAY_STR) || (val == USCHED_WEEKDAY_WEDNESDAY)) {
				val = USCHED_WEEKDAY_WEDNESDAY;
			} else if (!strcasecmp(arg, USCHED_WEEKDAY_THURSDAY_STR) || (val == USCHED_WEEKDAY_THURSDAY)) {
				val = USCHED_WEEKDAY_THURSDAY;
			} else if (!strcasecmp(arg, USCHED_WEEKDAY_FRIDAY_STR) || (val == USCHED_WEEKDAY_FRIDAY)) {
				val = USCHED_WEEKDAY_FRIDAY;
			} else if (!strcasecmp(arg, USCHED_WEEKDAY_SATURDAY_STR) || (val == USCHED_WEEKDAY_SATURDAY)) {
				val = USCHED_WEEKDAY_SATURDAY;
			} else {
				return -1;	/* Invalid weekday */
			}

			tm.tm_mday += (tm.tm_wday < (val - 1)) ? ((val - 1) - tm.tm_wday) : ((7 - tm.tm_wday) + (val - 1));

			tm.tm_sec = tm.tm_min = tm.tm_hour = 0;

			return mktime(&tm) - runc.t;
		}
	}

	return -1;
}

static usched_op_t _parse_get_op(const char *op) {
	if (!strcasecmp(op, USCHED_OP_RUN_STR))
		return USCHED_OP_RUN;

	if (!strcasecmp(op, USCHED_OP_STOP_STR))
		return USCHED_OP_STOP;

	if (!strcasecmp(op, USCHED_OP_SHOW_STR))
		return USCHED_OP_SHOW;

	return -1;
}

static usched_prep_t _parse_get_prep(const char *prep) {
	if (!strcasecmp(prep, USCHED_PREP_IN_STR))
		return USCHED_PREP_IN;

	if (!strcasecmp(prep, USCHED_PREP_ON_STR))
		return USCHED_PREP_ON;

	if (!strcasecmp(prep, USCHED_PREP_EVERY_STR))
		return USCHED_PREP_EVERY;

	if (!strcasecmp(prep, USCHED_PREP_NOW_STR))
		return USCHED_PREP_NOW;

	if (!strcasecmp(prep, USCHED_PREP_TO_STR))
		return USCHED_PREP_TO;

	return -1;
}

static usched_adverb_t _parse_get_adverb(const char *adverb) {
	if (!strcasecmp(adverb, USCHED_ADVERB_SECOND_STR) || !strcasecmp(adverb, USCHED_ADVERB_SECONDS_STR))
		return USCHED_ADVERB_SECONDS;

	if (!strcasecmp(adverb, USCHED_ADVERB_MINUTE_STR) || !strcasecmp(adverb, USCHED_ADVERB_MINUTES_STR))
		return USCHED_ADVERB_MINUTES;

	if (!strcasecmp(adverb, USCHED_ADVERB_HOUR_STR) || !strcasecmp(adverb, USCHED_ADVERB_HOURS_STR))
		return USCHED_ADVERB_HOURS;

	if (!strcasecmp(adverb, USCHED_ADVERB_DAY_STR) || !strcasecmp(adverb, USCHED_ADVERB_DAYS_STR))
		return USCHED_ADVERB_DAYS;

	if (!strcasecmp(adverb, USCHED_ADVERB_WEEK_STR) || !strcasecmp(adverb, USCHED_ADVERB_WEEKS_STR))
		return USCHED_ADVERB_WEEKS;

	if (!strcasecmp(adverb, USCHED_ADVERB_MONTH_STR) || !strcasecmp(adverb, USCHED_ADVERB_MONTHS_STR))
		return USCHED_ADVERB_MONTHS;

	if (!strcasecmp(adverb, USCHED_ADVERB_YEAR_STR) || !strcasecmp(adverb, USCHED_ADVERB_YEARS_STR))
		return USCHED_ADVERB_YEARS;

	if (!strcasecmp(adverb, USCHED_ADVERB_WEEKDAY_STR) || !strcasecmp(adverb, USCHED_ADVERB_WEEKDAYS_STR))
		return USCHED_ADVERB_WEEKDAYS;

	if (!strcasecmp(adverb, USCHED_ADVERB_TIME_STR))
		return USCHED_ADVERB_TIME;

	if (!strcasecmp(adverb, USCHED_ADVERB_DATE_STR))
		return USCHED_ADVERB_DATE;

	if (!strcasecmp(adverb, USCHED_ADVERB_DATETIME_STR))
		return USCHED_ADVERB_DATETIME;

	if (!strcasecmp(adverb, USCHED_ADVERB_TIMESTAMP_STR))
		return USCHED_ADVERB_TIMESTAMP;

	return -1;
}

static usched_conj_t _parse_get_conj(const char *conj) {
	if (!strcasecmp(conj, USCHED_CONJ_AND_STR))
		return USCHED_CONJ_AND;

	if (!strcasecmp(conj, USCHED_CONJ_THEN_STR))
		return USCHED_CONJ_THEN;

	if (!strcasecmp(conj, USCHED_CONJ_UNTIL_STR))
		return USCHED_CONJ_UNTIL;

	if (!strcasecmp(conj, USCHED_CONJ_WHILE_STR))
		return USCHED_CONJ_WHILE;

	return -1;
}

static struct usched_request *_parse_conj_compound(struct usched_request *req, int argc, char **argv) {
	int ret = 0;

	/* Process conjuction */
	if ((ret = _parse_get_conj(argv[0])) < 0) {
		usage_client_error_set(USCHED_USAGE_CLIENT_ERR_INVALID_CONJ, argv[0]);
		goto _conj_error;
	}

	req->conj = ret;

	/* Pre-validate the logic of conjunctions:
	 *
	 *	- After an UNTIL conjunction, only the AND conjuction is accepted
	 *	- After a WHILE conjunction, only the AND conjunction is accepted
	 */

	if (req->prev && (req->conj != USCHED_CONJ_AND)) {
		if ((req->prev->conj == USCHED_CONJ_UNTIL) || (req->prev->conj == USCHED_CONJ_WHILE)) {
			usage_client_error_set(USCHED_USAGE_CLIENT_ERR_UNEXPECT_CONJ, argv[0]);
			goto _conj_error;
		}
	}

	debug_printf(DEBUG_INFO, "CONJ: %d\n", req->conj);

	/* We need at least 4 args to consider the request (the [ CONJ ] and the { PREP [ ARG ADVERB | ADVERB ARG ] }) */
	if (argc < 4) {
		usage_client_error_set(USCHED_USAGE_CLIENT_ERR_INSUFF_ARGS, NULL);
		goto _conj_error;
	}

	/* Allocate the next entry */
	if (!(req->next = mm_alloc(sizeof(struct usched_request))))
		goto _conj_error;

	memset(req->next, 0, sizeof(struct usched_request));

	/* Duplicate SUBJECT to the next entry. strdup() shall not be used to take advantage of libfsma */
	if (!(req->next->subj = mm_alloc(strlen(req->subj) + 1)))
		goto _conj_error;

	strcpy(req->next->subj, req->subj);

	req->next->op   = req->op;
	req->next->uid  = req->uid;
	req->next->gid  = req->gid;
	req->next->prev = req;

	return _parse_prep_compound(req->next, argc - 1, &argv[1]);

_conj_error:
	parse_req_destroy(req);

	return NULL;
}

static struct usched_request *_parse_prep_compound(struct usched_request *req, int argc, char **argv) {
	int ret = 0;
	int argc_delta = 1;

	/* Process preposition */
	if ((ret = _parse_get_prep(argv[0])) < 0) {
		usage_client_error_set(USCHED_USAGE_CLIENT_ERR_INVALID_PREP, argv[0]);
		goto _prep_error;
	}

	req->prep = ret;

	if (req->prep == USCHED_PREP_TO) {
		/* The TO preposition is only expected when preceeded by an UNTIL conjuntion */
		if (!req->prev) {
			usage_client_error_set(USCHED_USAGE_CLIENT_ERR_UNEXPECT_PREP, argv[0]);
			goto _prep_error;
		}

		if (req->prev->conj != USCHED_CONJ_UNTIL) {
			usage_client_error_set(USCHED_USAGE_CLIENT_ERR_UNEXPECT_PREP, argv[0]);
			goto _prep_error;
		}
	} else {
		/* Any preposition other than TO after an UNTIL conjuntion is invalid */
		if (req->prev && (req->prev->conj == USCHED_CONJ_UNTIL)) {
			usage_client_error_set(USCHED_USAGE_CLIENT_ERR_UNEXPECT_PREP, argv[0]);
			goto _prep_error;
		}
	}

	if (req->prep != USCHED_PREP_IN) {
		/* After a WHILE conjunction, only the IN preposition is accepted */
		if (req->prev && (req->prev->conj == USCHED_CONJ_WHILE)) {
			usage_client_error_set(USCHED_USAGE_CLIENT_ERR_UNEXPECT_PREP, argv[0]);
			goto _prep_error;
		}
	}

	debug_printf(DEBUG_INFO, "PREP: %d\n", req->prep);

	if (req->prep != USCHED_PREP_NOW) {
		/* Prepositions other than NOW, always require 3 arguments */
		argc_delta = 3;

		/* We need at least 3 args */
		if (argc < 3) {
			usage_client_error_set(USCHED_USAGE_CLIENT_ERR_INSUFF_ARGS, NULL);
			goto _prep_error;
		}

		/* Process adverbial of time
		 *
		 * For the ON and TO prepositions, the adverbial of time comes before the argument.
		 * For the IN and EVERY prepositions, the argument comes before the adverbial of time.
		 *
		 */
		if ((ret = _parse_get_adverb(((req->prep == USCHED_PREP_ON) || (req->prep == USCHED_PREP_TO)) ? argv[1] : argv[2])) < 0) {
			usage_client_error_set(USCHED_USAGE_CLIENT_ERR_INVALID_ADVERB, ((req->prep == USCHED_PREP_ON) || (req->prep == USCHED_PREP_TO)) ? argv[1] : argv[2]);
			goto _prep_error;
		}

		req->adverb = ret;

		debug_printf(DEBUG_INFO, "ADVERB: %d\n", req->adverb);

		/* Process arg */
		if ((req->prep == USCHED_PREP_ON) || (req->prep == USCHED_PREP_TO)) {
			req->arg = _parse_req_arg(req, argv[2]);
		} else {
			req->arg = _parse_req_arg(req, argv[1]);
		}

		/* Validate argument value */
		if (req->arg < 0) {
			usage_client_error_set(USCHED_USAGE_CLIENT_ERR_INVALID_ARG, argv[2]);
			goto _prep_error;
		}
	} else {
		/* The NOW preposition isn't allowed after a conjunction */
		if (req->prev) {
			usage_client_error_set(USCHED_USAGE_CLIENT_ERR_UNEXPECT_PREP, argv[0]);
			goto _prep_error;
		}

		req->arg = 0;
	}

	debug_printf(DEBUG_INFO, "ARG: %lu\n", req->arg);

	return (argc - argc_delta) ? _parse_conj_compound(req, argc - argc_delta, &argv[argc_delta]) : req;

_prep_error:
	parse_req_destroy(req);

	return NULL;
}

static struct usched_request *_parse_subj_compound(struct usched_request *req, int argc, char **argv) {
	int errsv = errno;

	/* Process command */
	if (!(req->subj = mm_alloc(strlen(argv[0]) + 1))) {
		errsv = errno;
		goto _cmd_error;
	}

	strcpy(req->subj, argv[0]);

	debug_printf(DEBUG_INFO, "SUBJ: %s\n", req->subj);

	switch (req->op) {
		case USCHED_OP_RUN:  return (argc - 1) ? _parse_prep_compound(req, argc - 1, &argv[1]) : req;
		case USCHED_OP_STOP: if (!(argc - 1)) { return req; } else break;
		case USCHED_OP_SHOW: if (!(argc - 1)) { return req; } else break;
	}

_cmd_error:
	parse_req_destroy(req);

	errno = errsv;

	return NULL;
}

static struct usched_request *_parse_op_compound(struct usched_request *req, int argc, char **argv) {
	int ret = 0;

	/* Process operation */
	if ((ret = _parse_get_op(argv[0])) < 0) {
		usage_client_error_set(USCHED_USAGE_CLIENT_ERR_INVALID_OP, argv[0]);
		goto _op_error;
	} else {
		req->op = ret;
	}

	debug_printf(DEBUG_INFO, "OP: %d\n", req->op);

	/* Evaluate argument counter validity for each operation */
	switch (req->op) {
		case USCHED_OP_RUN:  if (argc < 3)  { usage_client_error_set(USCHED_USAGE_CLIENT_ERR_INSUFF_ARGS, NULL); goto _op_error; } break;
		case USCHED_OP_STOP: if (argc != 2) { usage_client_error_set(USCHED_USAGE_CLIENT_ERR_TOOMANY_ARGS, NULL); goto _op_error; } break;
		case USCHED_OP_SHOW: if (argc != 2) { usage_client_error_set(USCHED_USAGE_CLIENT_ERR_TOOMANY_ARGS, NULL); goto _op_error; } break;
	}

	return _parse_subj_compound(req, argc - 1, &argv[1]);

_op_error:
	parse_req_destroy(req);

	return NULL;
}

struct usched_request *parse_instruction_array(int argc, char **argv) {
	struct usched_request *req = NULL;

	/* We need at least 1 arg to consider the request */
	if (argc < 1)
		return NULL;

	if (!(req = mm_alloc(sizeof(struct usched_request))))
		return NULL;

	memset(req, 0, sizeof(struct usched_request));

	/* Get UID */
	req->uid = getuid();

	debug_printf(DEBUG_INFO, "UID: %u\n", req->uid);

	/* Get GID */
	req->gid = getgid();

	debug_printf(DEBUG_INFO, "GID: %u\n", req->gid);

	if (!_parse_op_compound(req, argc, argv))
		return NULL;

	return req;
}

struct usched_request *parse_instruction(const char *cmd) {
	int counter = 0;
	char *ptr = NULL, *saveptr = NULL, *qarg = NULL, *cmd_s = NULL;
	char **args = NULL;
	struct usched_request *req = NULL;

	/* Duplicate the cmd string to allow safe const and take advantage of libfsma by avoiding strdup() */
	if (!(cmd_s = mm_alloc(strlen(cmd) + 1)))
		return NULL;

	strcpy(cmd_s, cmd);

	/* Split by space, tab and newline */
	for (counter = 0, ptr = cmd_s; (ptr = strtok_r(ptr, " \t\n", &saveptr)); counter ++, ptr = NULL, qarg = NULL) {
		if (!(args = mm_realloc(args, sizeof(char **) * (counter + 2))))
			goto _finish;

		args[counter] = args[counter + 1] = NULL;

		if ((ptr[0] == '\'') || (ptr[0] == '\"')) {
			size_t len = strlen(ptr);
			char qchr = ptr[0];
			int done = (len > 1) && (ptr[len - 1] == qchr) && (ptr[len - 2] != '\\');

			if (!(qarg = mm_alloc(len - done)))
				goto _finish;

			memcpy(memset(qarg, 0, len - done), ptr + 1, strlen(ptr) - 1 - done);

			while (!done && (ptr = strtok_r(NULL, " \t\n", &saveptr))) {
				len = strlen(ptr);

				done = (ptr[len - 1] == qchr) && ((len > 1) ? (ptr[len - 2] != '\\') : 1);

				if (!(qarg = mm_realloc(qarg, strlen(qarg) + 1 + len + !done)))
					goto _finish;

				if (qarg[0])
					strcat(qarg, " ");

				strncat(qarg, ptr, len - done);
			}

			if (!done)
				goto _finish;

			args[counter] = qarg;
		} else {
			if (!(args[counter] = mm_alloc(strlen(ptr) + 1)))
				goto _finish;

			strcpy(args[counter], ptr);
		}
	}

	req = parse_instruction_array(counter, args);

_finish:
	if (cmd_s)
		mm_free(cmd_s);

	if (qarg)
		mm_free(qarg);

	if (args) {
		for (counter = 0; args[counter]; counter ++)
			mm_free(args[counter]);

		mm_free(args);
	}

	return req;
}

void parse_req_destroy(struct usched_request *req) {
	if (req) {
		if (req->subj)
			mm_free(req->subj);

		if (req->next)
			parse_req_destroy(req->next);

		mm_free(req);
	}
}

