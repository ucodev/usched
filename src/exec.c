/**
 * @file exec.c
 * @brief uSched
 *        Execution Module Main Component
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


#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <pthread.h>
#include <mqueue.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "config.h"
#include "debug.h"
#include "runtime.h"
#include "log.h"
#include "daemonize.h"

static void *_exec_cmd(void *arg) {
	char *buf = arg;	/* | uid (32 bits) | gid (32 bits) | cmd (dynnamic) ... | */
	uint32_t uid = 0, gid = 0;
	char *cmd = &buf[8];
	pid_t pid = 0;
	int status = 0;

	memcpy(&uid, buf, 4);
	memcpy(&gid, buf + 4, 4);

	/* Create a new process, drop privileges to UID and GID and execute CMD */
	if ((pid = fork()) < 0) {
		/* Failure */
		log_warn("_exec_cmd(): fork(): %s\n", strerror(errno));
	} else if (!pid) {
		/* Child */
		pid = getpid();

		debug_printf(DEBUG_INFO, "Executing: %s\nUID: %u\nGID: %u\n", cmd, uid, gid);

		/* Create a new session */
		if (setsid() == (pid_t) - 1)
			log_warn("PID[%u]: _exec_cmd(): setsid(): %s\n", pid, strerror(errno));

		/* Redirect standard files */
		if (!freopen(CONFIG_SYS_DEV_ZERO, "r", stdin)) {
			log_warn("PID[%u]: _exec_cmd(): freopen(\"%s\", \"r\", stdin): %s\n",
				CONFIG_SYS_DEV_ZERO, pid, strerror(errno));
		}

		if (!freopen(CONFIG_SYS_DEV_NULL, "a", stdout)) {
			log_warn("PID[%u]: _exec_cmd(): freopen(\"%s\", \"a\", stdout): %s\n",
				CONFIG_SYS_DEV_NULL, pid, strerror(errno));
		}

		if (!freopen(CONFIG_SYS_DEV_NULL, "a", stderr)) {
			log_warn("PID[%u]: _exec_cmd(): freopen(\"%s\", \"a\", stderr): %s\n",
				CONFIG_SYS_DEV_NULL, pid, strerror(errno));
		}

		/* Drop privileges, if required */
		if (setuid(uid) < 0) {
			log_crit("PID[%u]: _exec_cmd(): setuid(%u): %s\n", pid, uid, strerror(errno));
			exit(EXIT_FAILURE);
		}

		if (setgid(gid) < 0) {
			log_crit("PID[%u]: _exec_cmd(): setgid(%u): %s\n", pid, gid, strerror(errno));
			exit(EXIT_FAILURE);
		}

		/* Execute command. TODO: This should be done by execve() with '/bin/sh -c' as prefix args */
		if ((status = system(cmd)) < 0) {
			log_crit("PID[%u]: _exec_cmd(): system(\"%s\"): %s\n", pid, cmd, strerror(errno));
			exit(EXIT_FAILURE);
		}

		log_info("PID[%u]: Executed '%s'. Exit Status: %d\n", pid, cmd, WEXITSTATUS(status));

		/* Exit child process with the exit status returned by system() */
		exit(WEXITSTATUS(status));
	}

	/* Parent */

	mm_free(arg);

	pthread_exit(NULL);

	return NULL;
}

static void _exec_process(void) {
	pthread_t ptid;
	char *tbuf = NULL;

	for (;;) {
		if (!(tbuf = mm_alloc(CONFIG_USCHED_PMQ_MSG_SIZE))) {
			log_warn("_exec_process(): tbuf = mm_alloc(): %s\n", strerror(errno));
			continue;
		}

		memset(tbuf, 0, CONFIG_USCHED_PMQ_MSG_SIZE);

		/* Read message from queue */
		if (mq_receive(rune.pmqd, tbuf, CONFIG_USCHED_PMQ_MSG_SIZE, 0) < 0) {
			log_warn("_exec_process(): mq_receive(): %s\n", strerror(errno));
			mm_free(tbuf);
			continue;
		}

		/* Create a new thread for command execution */
		if (pthread_create(&ptid, NULL, _exec_cmd, tbuf)) {
			log_warn("_exec_process(): pthread_create(): %s\n", strerror(errno));
			mm_free(tbuf);
			continue;
		}
	}
}

static void _init(int argc, char **argv) {
	if (runtime_exec_init(argc, argv) < 0) {
		log_crit("_init(): runtime_exec_init(): %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	if (daemonize() < 0) {
		log_crit("_init(): daemonize(): %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
}

static void _destroy(void) {
	runtime_exec_destroy();
}

static void _loop(void) {
	_exec_process();
}

int main(int argc, char *argv[]) {
	_init(argc, argv);

	_loop();

	_destroy();

	return 0;
}

