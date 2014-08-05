/**
 * @file usched.h
 * @brief uSched
 *        uSched Common interface header
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


#ifndef USCHED_H
#define USCHED_H

#include <stdint.h>

#include <sys/types.h>

#include "entry.h"

/* Categorty - Human */
#define USCHED_CATEGORY_USERS_STR	"users"

/* Operations - Human */
#define USCHED_OP_RUN_STR		"run"
#define USCHED_OP_STOP_STR		"stop"
#define USCHED_OP_SHOW_STR		"show"
#define USCHED_OP_ADD_STR		"add"
#define USCHED_OP_DELETE_STR		"delete"
#define USCHED_OP_CHANGE_STR		"change"

/* Prepositions - Human */
#define USCHED_PREP_IN_STR		"in"
#define USCHED_PREP_ON_STR		"on"
#define USCHED_PREP_EVERY_STR		"every"
#define USCHED_PREP_NOW_STR		"now"
#define USCHED_PREP_TO_STR		"to"

/* Adverbials of time - Human */
#define USCHED_ADVERB_SECOND_STR	"second"
#define USCHED_ADVERB_SECONDS_STR	"seconds"
#define USCHED_ADVERB_MINUTE_STR	"minute"
#define USCHED_ADVERB_MINUTES_STR	"minutes"
#define USCHED_ADVERB_HOUR_STR		"hour"
#define USCHED_ADVERB_HOURS_STR		"hours"
#define USCHED_ADVERB_DAY_STR		"day"
#define USCHED_ADVERB_DAYS_STR		"days"
#define USCHED_ADVERB_WEEK_STR		"week"
#define USCHED_ADVERB_WEEKS_STR		"weeks"
#define USCHED_ADVERB_MONTH_STR		"month"
#define USCHED_ADVERB_MONTHS_STR	"months"
#define USCHED_ADVERB_YEAR_STR		"year"
#define USCHED_ADVERB_YEARS_STR		"years"
#define USCHED_ADVERB_WEEKDAY_STR	"weekday"
#define USCHED_ADVERB_WEEKDAYS_STR	"weekdays"
#define USCHED_ADVERB_TIME_STR		"time"
#define USCHED_ADVERB_DATE_STR		"date"
#define USCHED_ADVERB_DATETIME_STR	"datetime"
#define USCHED_ADVERB_TIMESTAMP_STR	"timestamp"

/* Conjuctions - Human */
#define USCHED_CONJ_AND_STR		"and"
#define USCHED_CONJ_THEN_STR		"then"
#define USCHED_CONJ_UNTIL_STR		"until"
#define USCHED_CONJ_WHILE_STR		"while"

/* Weekdays - Human */
#define USCHED_WEEKDAY_MONDAY_STR	"monday"
#define USCHED_WEEKDAY_TUESDAY_STR	"tuesday"
#define USCHED_WEEKDAY_WEDNESDAY_STR	"wednesday"
#define USCHED_WEEKDAY_THURSDAY_STR	"thursday"
#define USCHED_WEEKDAY_FRIDAY_STR	"friday"
#define USCHED_WEEKDAY_SATURDAY_STR	"saturday"
#define USCHED_WEEKDAY_SUNDAY_STR	"sunday"

/* Categories - Machine */
typedef enum CATEGORY {
	USCHED_CATEGORY_USERS = 1
} usched_category_t;

/* Operations - Machine */
typedef enum OP {
	USCHED_OP_RUN = 1,
	USCHED_OP_STOP,
	USCHED_OP_SHOW,
	USCHED_OP_ADD,		/* Used by administration tools only */
	USCHED_OP_DELETE,	/* Used by administration tools only */
	USCHED_OP_CHANGE	/* Used by administration tools only */
} usched_op_t;

/* Prepositions - Machine */
typedef enum PREP {
	USCHED_PREP_IN = 1,
	USCHED_PREP_ON,
	USCHED_PREP_EVERY,
	USCHED_PREP_NOW,
	USCHED_PREP_TO
} usched_prep_t;

/* Adverbials of time - Machine */
typedef enum ADVERB {
	USCHED_ADVERB_SECONDS = 1,
	USCHED_ADVERB_MINUTES,
	USCHED_ADVERB_HOURS,
	USCHED_ADVERB_DAYS,
	USCHED_ADVERB_WEEKS,
	USCHED_ADVERB_MONTHS,
	USCHED_ADVERB_YEARS,
	USCHED_ADVERB_WEEKDAYS,
	USCHED_ADVERB_TIME,
	USCHED_ADVERB_DATE,
	USCHED_ADVERB_DATETIME,
	USCHED_ADVERB_TIMESTAMP
} usched_adverb_t;

/* Conjuctions - Machine */
typedef enum CONJ {
	USCHED_CONJ_AND = 1,
	USCHED_CONJ_THEN,
	USCHED_CONJ_UNTIL,
	USCHED_CONJ_WHILE
} usched_conj_t;

/* Weekdays - Machine */
typedef enum WEEKDAY {
	USCHED_WEEKDAY_SUNDAY = 1,
	USCHED_WEEKDAY_MONDAY,
	USCHED_WEEKDAY_TUESDAY,
	USCHED_WEEKDAY_WEDNESDAY,
	USCHED_WEEKDAY_THURSDAY,
	USCHED_WEEKDAY_FRIDAY,
	USCHED_WEEKDAY_SATURDAY
} usched_weekday_t;

/* uSched Admin Request Structure */
struct usched_admin_request {
	usched_op_t op;
	usched_category_t category;
	char **args;
	size_t argc;
};

/* uSched Client Request Structure */
struct usched_client_request {
	usched_op_t op;
	char *subj;
	usched_prep_t prep;
	usched_adverb_t adverb;
	usched_conj_t conj;
	long arg;
	uid_t uid;
	gid_t gid;
	struct usched_client_request *next;
	struct usched_client_request *prev;
};

#endif
