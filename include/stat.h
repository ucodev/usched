/**
 * @file stat.h
 * @brief uSched
 *        Stat configuration and administration interface header
 *
 * Date: 21-05-2015
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

#ifndef USCHED_STAT_H
#define USCHED_STAT_H

/* Structures */
struct usched_stat_exec {
	uid_t uid;			/* The Entry UID */
	gid_t gid;			/* The Entry GID */
	pid_t pid;			/* The Entry PID */
	int status;			/* The Entry exit status */
	struct timespec trigger;	/* Timestamp of the Entry trigger */
	struct timespec start;		/* Timestamp of when the Entry processing was started */
	struct timespec end;		/* Timestamp of when the Entry finished processing */
	size_t outdata_len;		/* Length of the Entry output (may be truncated) */
	char outdata[CONFIG_USCHED_EXEC_OUTPUT_MAX + 1]; /* Entry output (may be truncated) */
};

struct usched_stat_entry {
	uint64_t id;			/* Entry ID */
	struct usched_stat_exec current;/* Stats of the last execution */
	struct usched_stat_exec error;	/* Stats of the last error */
	unsigned int nr_exec;		/* Number of times executed */
	unsigned int nr_ok;		/* Number of executions with zero status */
	unsigned int nr_fail;		/* Number of executions with non-zero status */
};

/* Prototypes */
int stat_admin_commit(void);
int stat_admin_rollback(void);
int stat_admin_show(void);
int stat_admin_jail_dir_show(void);
int stat_admin_jail_dir_change(const char *jail_dir);
int stat_admin_privdrop_group_show(void);
int stat_admin_privdrop_group_change(const char *privdrop_group);
int stat_admin_privdrop_user_show(void);
int stat_admin_privdrop_user_change(const char *privdrop_user);
int stat_admin_report_file_show(void);
int stat_admin_report_file_change(const char *report_file);
int stat_admin_report_freq_show(void);
int stat_admin_report_freq_change(const char *report_freq);
int stat_admin_report_mode_show(void);
int stat_admin_report_mode_change(const char *report_mode);
int stat_daemon_init(void);
void stat_daemon_destroy(void);
int stat_compare(const void *s1, const void *s2);
struct usched_stat_entry *stat_dup(const struct usched_stat_entry *s);
void stat_zero(struct usched_stat_entry *s);
void stat_destroy(void *elem);

#endif

