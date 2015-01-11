/**
 * @file request_basic.c
 * @brief uSched
 *        uSched Basic Library Request Example
 *
 * Date: 11-01-2015
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

#include <usched/lib.h>

int main(void) {
	int i = 0;
	uint64_t *v = NULL;
	size_t nmemb = 0;

	/* Initialize uSched engine */
	usched_init();

	/* Perform the uSched request */
	usched_request("run 'ls -lah /' in 10 seconds then every 5 seconds");

	/* Fetch the results */
	usched_result_get_run(&v, &nmemb);

	/* Show the results */
	puts("Installed entries:");

	for (i = 0; i < nmemb; i ++)
		printf("  0x%016llX\n", (unsigned long long) v[i]);

	/* Free result resources */
	usched_result_free_run();

	/* Destroy the uSched engine */
	usched_destroy();

	/* All good */
	return 0;
}

