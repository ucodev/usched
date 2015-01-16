/**
 * @file exec.c
 * @brief uSched
 *        Execution Module Main Component
 *
 * Date: 16-01-2015
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
#include "bitops.h"

extern char **environ;

static void *_exec_cmd(void *arg) {
	char *buf = arg;	/* | id (64 bits) | uid (32 bits) | gid (32 bits) | cmd (...) ... | */
	uint64_t id = 0;
	uint32_t uid = 0, gid = 0;
	char *cmd = &buf[16];
	pid_t pid = 0;
	int status = 0;

	memcpy(&id, buf, 8);
	memcpy(&uid, buf + 8, 4);
	memcpy(&gid, buf + 12, 4);

	/* Create child */
	pid = fork();

	/* Create a new process, drop privileges to UID and GID and execute CMD */
	if (pid == (pid_t) -1) {
		/* Failure */
		log_warn("Entry[0x%016llX]: _exec_cmd(): fork(): %s\n", id, strerror(errno));
	} else if (!pid) {
		/* Child */

		/* Get child pid */
		pid = getpid();

		debug_printf(DEBUG_INFO, "Entry[0x%016llX]: PID[%u]: Executing: %s\nUID: %u\nGID: %u\n", id, pid, cmd, uid, gid);

		/* Create a new session */
		if (setsid() == (pid_t) -1)
			log_warn("Entry[0x%016llX]: PID[%u]: _exec_cmd(): setsid(): %s\n", id, pid, strerror(errno));

		/* Redirect standard files */
		if (!freopen(CONFIG_SYS_DEV_ZERO, "r", stdin)) {
			log_warn("Entry[0x%016llX]: PID[%u]: _exec_cmd(): freopen(\"%s\", \"r\", stdin): %s\n", id, pid, CONFIG_SYS_DEV_ZERO, strerror(errno));
		}

		if (!freopen(CONFIG_SYS_DEV_NULL, "a", stdout)) {
			log_warn("Entry[0x%016llX]: PID[%u]: _exec_cmd(): freopen(\"%s\", \"a\", stdout): %s\n", id, pid, CONFIG_SYS_DEV_NULL, strerror(errno));
		}

		if (!freopen(CONFIG_SYS_DEV_NULL, "a", stderr)) {
			log_warn("Entry[0x%016llX]: PID[%u]: _exec_cmd(): freopen(\"%s\", \"a\", stderr): %s\n", id, pid, CONFIG_SYS_DEV_NULL, strerror(errno));
		}

		/* Drop privileges, if required */
		if (setregid(gid, gid) < 0) {
			log_crit("Entry[0x%016llX]: PID[%u]: _exec_cmd(): setregid(%u): %s\n", id, pid, gid, strerror(errno));

			/* Free argument resources */
			mm_free(arg);

			exit(EXIT_FAILURE);
		}

		if (setreuid(uid, uid) < 0) {
			log_crit("Entry[0x%016llX]: PID[%u]: _exec_cmd(): setreuid(%u): %s\n", id, pid, uid, strerror(errno));

			/* Free argument resources */
			mm_free(arg);

			exit(EXIT_FAILURE);
		}

		/* Paranoid mode */
		if ((getuid() != uid) || (geteuid() != uid)) {
			log_crit("Entry[0x%016llX]: PID[%u]: _exec_cmd(): Unexpected UID[%u] or EUID[%u] value. Expecting: %u\n", id, pid, getuid(), geteuid(), uid);

			/* Free argument resources */
			mm_free(arg);

			exit(EXIT_FAILURE);
		}

		if ((getgid() != gid) || (getegid() != gid)) {
			log_crit("Entry[0x%016llX]: PID[%u]: _exec_cmd(): Unexpected GID[%u] or EGID[%u] value. Expecting: %u\n", id, pid, getgid(), getegid(), gid);

			/* Free argument resources */
			mm_free(arg);

			exit(EXIT_FAILURE);
		}

		/* Cleanup child */
		runtime_exec_quiet_destroy();

		/* Execute command */
		if (execlp(CONFIG_USCHED_SHELL_BIN_PATH, CONFIG_USCHED_SHELL_BIN_PATH, "-c", cmd, (char *) NULL) < 0)
			log_crit("Entry[0x%016llX]: PID[%u]: _exec_cmd(): execlp(\"%s\", \"-c\", \"%s\"): %s\n", id, pid, CONFIG_USCHED_SHELL_BIN_PATH, cmd, strerror(errno));

		/* Free argument resources */
		mm_free(arg);

		/* If reached, execlp() have failed */
		exit(EXIT_FAILURE);
	} else {
		/* Parent */
		log_info("Entry[0x%016llX]: PID[%u]: Executing '%s' [uid: %u, gid: %u]. Waiting for child to exit...", id, pid, cmd, uid, gid);

		/* Wait for child to return */
		if (waitpid(pid, &status, 0) < 0)
			log_crit("Entry[0x%016llX]: _exec_cmd(): waitpid(): %s\n", id, strerror(errno));

		log_info("Entry[0x%016llX]: PID[%u]: Child exited. Exit Status: %d\n", id, pid, WEXITSTATUS(status));
	}

	/* Free argument resources */
	mm_free(arg);

	/* Terminate this thread */
	pthread_exit(NULL);

	/* All good */
	return NULL;
}

static void _exec_process(void) {
	pthread_t ptid;
	char *tbuf = NULL;
	struct mq_attr mqattr;

	for (;;) {
		/* Check for rutime interruptions */
		if (runtime_exec_interrupted()) {
			/* Get posix queue attributes */
			if (mq_getattr(rune.pmqd, &mqattr) < 0) {
				log_warn("_exec_process(): mq_getattr(): %s\n", strerror(errno));
				break;
			}

			/* Interrupt execution only when there are no messages in the queue */
			if (!mqattr.mq_curmsgs)
				break;
		}

		if (!(tbuf = mm_alloc(rune.config.core.pmq_msgsize))) {
			log_warn("_exec_process(): tbuf = mm_alloc(): %s\n", strerror(errno));
			continue;
		}

		memset(tbuf, 0, rune.config.core.pmq_msgsize);

		/* Read message from queue */
		if (mq_receive(rune.pmqd, tbuf, rune.config.core.pmq_msgsize, 0) < 0) {
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

		/* Detach the newly created thread as the resources should be automatically free'd
		 * upon thread termination.
		 */
		if (pthread_detach(ptid))
			log_crit("_exec_process(): pthread_detach(): %s. (Possible memory leak)\n", strerror(errno));
	}
}

static void _init(int argc, char **argv) {
	if (runtime_exec_init(argc, argv) < 0) {
		log_crit("_init(): runtime_exec_init(): %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
}

static void _destroy(void) {
	runtime_exec_destroy();
}

static void _loop(int argc, char **argv) {
	_init(argc, argv);

	for (;;) {
		/* Process exec requests */
		_exec_process();

		/* Check for runtime interruptions */
		if (bit_test(&rune.flags, USCHED_RUNTIME_FLAG_TERMINATE))
			break;

		if (bit_test(&rune.flags, USCHED_RUNTIME_FLAG_RELOAD)) {
			_destroy();
			_init(argc, argv);
		}
	}

	_destroy();
}

int main(int argc, char *argv[]) {
	_loop(argc, argv);

	return 0;
}

