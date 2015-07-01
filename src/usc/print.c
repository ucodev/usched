/**
 * @file print.c
 * @brief uSched
 *        Printing interface - Client
 *
 * Date: 01-07-2015
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
#include <stdint.h>

#include "entry.h"

void print_client_result_error(void) {
	printf("An error ocurred. Check your syslog entries for more details.\n");
}

void print_client_result_empty(void) {
	printf("No results.\n");
}

void print_client_result_run(uint64_t entry_id) {
	printf("Installed Entry ID: 0x%016llX\n", (unsigned long long) entry_id);
}

void print_client_result_del(const uint64_t *entry_list, size_t count) {
	size_t i = 0;

	for (i = 0; i < count; i ++)
		printf("Deleted Entry ID: 0x%016llX\n", (unsigned long long) entry_list[i]);
}

static void _print_client_result_single_show(const struct usched_entry *entry_list, size_t count) {
	const struct usched_entry *entry = &entry_list[0];

	if (!entry)
		return;

	printf("Entry ID:  0x%016llX\n", (unsigned long long) entry->id);
	/* TODO: Show flags */
	printf("Username:  %s\n", !entry->username[0] ? "-" : entry->username);
	printf("Trigger:   %u\n", (unsigned int) entry->trigger);
	printf("Step:      %u\n", (unsigned int) entry->step);
	printf("Expire:    %u\n", (unsigned int) entry->expire);
	printf("UID:       %u\n", (unsigned int) entry->uid);
	printf("GID:       %u\n", (unsigned int) entry->gid);
	printf("Command:   %s\n", entry->subj);
	printf("Status:    %u\n", entry->status);
	printf("Exec Time: %.3fus\n", entry->exec_time / 1000.0);
	printf("Latency:   %.3fus\n", entry->latency / 1000.0);
	printf("PID:       %u\n", entry->pid);
	printf("Output:    %s\n", entry->outdata);
}

static void _print_client_result_multi_show(const struct usched_entry *entry_list, size_t count) {
	size_t i = 0;

	printf("                 id |    user | status |     trigger |     step |      expire | cmd\n");

	for (i = 0; i < count; i ++) {
		printf(
			"%c0x%016llX | " \
			"%7s | " \
			"%6u | " \
			"%11u | " \
			"%8u | " \
			"%11u | " \
			"%s\n",
			entry_has_flag(&entry_list[i], USCHED_ENTRY_FLAG_INVALID) ? '*' : ' ',
			(unsigned long long) entry_list[i].id,
			!entry_list[i].username[0] ? "-" : entry_list[i].username,
			(unsigned int) entry_list[i].status,
			(unsigned int) entry_list[i].trigger,
			(unsigned int) entry_list[i].step,
			(unsigned int) entry_list[i].expire,
			entry_list[i].subj);
	}
}

void print_client_result_show(const struct usched_entry *entry_list, size_t count) {
	if (count > 1) {
		_print_client_result_multi_show(entry_list, count);
	} else {
		_print_client_result_single_show(entry_list, count);
	}
}

