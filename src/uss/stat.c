/**
 * @file stat.c
 * @brief uSched
 *        Status and Statistics Module Main Component
 *
 * Date: 31-03-2015
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

#include <stdlib.h>
#include <unistd.h>

#include "config.h"
#include "runtime.h"
#include "log.h"
#include "bitops.h"

static void _stat_process(void) {
	/* TODO */
	pause();

	return;
}

static void _init(int argc, char **argv) {
	if (runtime_stat_init(argc, argv) < 0) {
		log_crit("_init(): runtime_stat_init(): %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
}

static void _destroy(void) {
	runtime_stat_destroy();
}

static void _loop(int argc, char **argv) {
	_init(argc, argv);

	for (;;) {
		/* Process status and statistics requests */
		_stat_process();

		/* Check for runtime interruptions */
		if (bit_test(&runs.flags, USCHED_RUNTIME_FLAG_TERMINATE))
			break;

		if (bit_test(&runs.flags, USCHED_RUNTIME_FLAG_RELOAD)) {
			_destroy();
			_init(argc, argv);
		}
	}

	_destroy();
}

int main(int argc, char **argv) {
	_loop(argc, argv);

	return 0;
}

