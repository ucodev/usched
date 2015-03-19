/**
 * @file exec.c
 * @brief uSched
 *        Execution Module Main Component
 *
 * Date: 19-03-2015
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
#include <time.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "config.h"
#include "debug.h"
#include "runtime.h"
#include "log.h"
#include "bitops.h"
#include "local.h"

#if CONFIG_USE_IPC_PMQ == 1
 #include <mqueue.h>
#elif CONFIG_USE_IPC_SOCK == 1
 #include <sys/socket.h>
 #include <panet/panet.h>

 static sock_t usd_fd = (sock_t) -1;
#endif

extern char **environ;

static void *_exec_cmd(void *arg) {
	char *buf = arg;	/* | id (64 bits) | uid (32 bits) | gid (32 bits) | trigger (32 bits) | cmd (...) ... | */
	uint64_t id = 0;
	uint32_t uid = 0, gid = 0, trigger = 0;
	char *cmd = &buf[20];
	pid_t pid = 0;
	int status = 0;

	memcpy(&id, buf, 8);
	memcpy(&uid, buf + 8, 4);
	memcpy(&gid, buf + 12, 4);
	memcpy(&trigger, buf + 16, 4);

	/* Check delta time before executing event (Absolute value is a safe check. Negative values
	 * won't occur here... hopefully).
	 */
	if ((unsigned int) labs((long) (time(NULL) - trigger)) >= rune.config.core.delta_noexec) {
		log_warn("Entry[0x%016llX]: _exec_cmd(): Entry delta T (%u seconds) is >= than the configured delta T for noexec (%d seconds). Ignoring execution...\n", id, time(NULL) - trigger, rune.config.core.delta_noexec);
	} else if ((pid = fork()) == (pid_t) -1) {	/* Create a new process, drop privileges to
							 * UID and GID and execute CMD
							 */
		/* Failure */
		log_warn("Entry[0x%016llX]: _exec_cmd(): fork(): %s\n", id, strerror(errno));
	} else if (!pid) {
		/* Child */

#if CONFIG_USE_IPC_PMQ == 1
		/* Close message queue descriptor */
		mq_close(rune.ipcd);
#elif CONFIG_USE_IPC_SOCK == 1
		/* Close unix socket descriptor */
		panet_safe_close(rune.ipcd);

		/* Close current uSched daemon descriptor */
		panet_safe_close(usd_fd);
#endif

		/* Get child pid */
		pid = getpid();

		debug_printf(DEBUG_INFO, "Entry[0x%016llX]: PID[%u]: Executing: %s\nUID: %u\nGID: %u\n", id, pid, cmd, uid, gid);

		/* Create a new session */
		if (setsid() == (pid_t) -1) {
			/* Free argument resources */
			mm_free(arg);

			exit(CONFIG_SYS_EXIT_CODE_CUSTOM_BASE + CHILD_EXIT_STATUS_FAILED_SETSID);
		}

		/* Redirect standard files */
		if (!freopen(CONFIG_SYS_DEV_ZERO, "r", stdin)) {
			/* Free argument resources */
			mm_free(arg);

			exit(CONFIG_SYS_EXIT_CODE_CUSTOM_BASE + CHILD_EXIT_STATUS_FAILED_FREOPEN_STDIN);
		}

		if (!freopen(CONFIG_SYS_DEV_NULL, "a", stdout)) {
			/* Free argument resources */
			mm_free(arg);

			exit(CONFIG_SYS_EXIT_CODE_CUSTOM_BASE + CHILD_EXIT_STATUS_FAILED_FREOPEN_STDOUT);
		}

		if (!freopen(CONFIG_SYS_DEV_NULL, "a", stderr)) {
			/* Free argument resources */
			mm_free(arg);

			exit(CONFIG_SYS_EXIT_CODE_CUSTOM_BASE + CHILD_EXIT_STATUS_FAILED_FREOPEN_STDERR);
		}

#if CONFIG_USCHED_MULTIUSER == 1
		/* Drop privileges, if required */
		if (setregid(gid, gid) < 0) {
			/* Free argument resources */
			mm_free(arg);

			exit(CONFIG_SYS_EXIT_CODE_CUSTOM_BASE + CHILD_EXIT_STATUS_FAILED_SETREGID);
		}

		if (setreuid(uid, uid) < 0) {
			/* Free argument resources */
			mm_free(arg);

			exit(CONFIG_SYS_EXIT_CODE_CUSTOM_BASE + CHILD_EXIT_STATUS_FAILED_SETREUID);
		}
#endif

		/* Paranoid mode */
		if ((getuid() != (uid_t) uid) || (geteuid() != (uid_t) uid)) {
			/* Free argument resources */
			mm_free(arg);

			exit(CONFIG_SYS_EXIT_CODE_CUSTOM_BASE + CHILD_EXIT_STATUS_FAILED_UID);
		}

		if ((getgid() != (gid_t) gid) || (getegid() != (gid_t) gid)) {
			/* Free argument resources */
			mm_free(arg);

			exit(CONFIG_SYS_EXIT_CODE_CUSTOM_BASE + CHILD_EXIT_STATUS_FAILED_GID);
		}

		/* Cleanup child */
		runtime_exec_quiet_destroy();

		/* Execute command */
		execlp(CONFIG_USCHED_SHELL_BIN_PATH, CONFIG_USCHED_SHELL_BIN_PATH, "-c", cmd, (char *) NULL);

		/* If execlp() returns, it's safe to assume that it has failed */

		/* Free argument resources */
		mm_free(arg);

		/* If reached, execlp() have failed */
		exit(CONFIG_SYS_EXIT_CODE_CUSTOM_BASE + CHILD_EXIT_STATUS_FAILED_EXECLP);
	} else {
		/* Parent */
		log_info("Entry[0x%016llX]: PID[%u]: Executing '%s' [uid: %u, gid: %u]. Waiting for child to exit...\n", id, pid, cmd, uid, gid);

		/* Wait for child to return */
		if (waitpid(pid, &status, 0) < 0)
			log_crit("Entry[0x%016llX]: _exec_cmd(): waitpid(): %s\n", id, strerror(errno));

		/* Log errors (if any) based on exit status */
		switch (WEXITSTATUS(status)) {
			case CONFIG_SYS_EXIT_CODE_CUSTOM_BASE + CHILD_EXIT_STATUS_FAILED_SETSID: {
				log_warn("Entry[0x%016llX]: PID[%u]: setsid() failed.\n", id, pid);
			} break;
			case CONFIG_SYS_EXIT_CODE_CUSTOM_BASE + CHILD_EXIT_STATUS_FAILED_FREOPEN_STDIN: {
				log_warn("Entry[0x%016llX]: PID[%u]: freopen(%s, \"r\", stdout) failed.\n", id, pid, CONFIG_SYS_DEV_ZERO);
			} break;
			case CONFIG_SYS_EXIT_CODE_CUSTOM_BASE + CHILD_EXIT_STATUS_FAILED_FREOPEN_STDOUT: {
				log_warn("Entry[0x%016llX]: PID[%u]: freopen(%s, \"a\", stdout) failed.\n", id, pid, CONFIG_SYS_DEV_NULL);
			} break;
			case CONFIG_SYS_EXIT_CODE_CUSTOM_BASE + CHILD_EXIT_STATUS_FAILED_FREOPEN_STDERR: {
				log_warn("Entry[0x%016llX]: PID[%u]: freopen(%s, \"a\", stderr) failed.\n", id, pid, CONFIG_SYS_DEV_NULL);
			} break;
			case CONFIG_SYS_EXIT_CODE_CUSTOM_BASE + CHILD_EXIT_STATUS_FAILED_SETREGID: {
				log_warn("Entry[0x%016llX]: PID[%u]: setregid() failed.\n", id, pid);
			} break;
			case CONFIG_SYS_EXIT_CODE_CUSTOM_BASE + CHILD_EXIT_STATUS_FAILED_SETREUID: {
				log_warn("Entry[0x%016llX]: PID[%u]: setreuid() failed.\n", id, pid);
			} break;
			case CONFIG_SYS_EXIT_CODE_CUSTOM_BASE + CHILD_EXIT_STATUS_FAILED_UID: {
				log_warn("Entry[0x%016llX]: PID[%u]: Failed to drop process privileges (UID).", id, pid);
			} break;
			case CONFIG_SYS_EXIT_CODE_CUSTOM_BASE + CHILD_EXIT_STATUS_FAILED_GID: {
				log_warn("Entry[0x%016llX]: PID[%u]: Failed to drop process privileges (GID).", id, pid);
			} break;
			case CONFIG_SYS_EXIT_CODE_CUSTOM_BASE + CHILD_EXIT_STATUS_FAILED_EXECLP: {
				log_warn("Entry[0x%016llX]: PID[%u]: execlp() failed.", id, pid);
			} break;
			case EXIT_SUCCESS:
			default: break;
		}

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
#if CONFIG_USE_IPC_PMQ == 1
	struct mq_attr mqattr;
#elif CONFIG_USE_IPC_SOCK == 1
	uid_t fd_uid = (uid_t) -1;
	gid_t fd_gid = (gid_t) -1;

	/* Wait for the first event */
	if ((usd_fd = (sock_t) accept(rune.ipcd, NULL, NULL)) < 0) {
		log_warn("_exec_process(): accept(): %s\n", strerror(errno));
		continue;
	}
#endif

	for (;;) {
		/* Check for rutime interruptions */
		if (runtime_exec_interrupted()) {
#if CONFIG_USE_IPC_PMQ == 1
			/* Get posix queue attributes */
			if (mq_getattr(rune.ipcd, &mqattr) < 0) {
				log_warn("_exec_process(): mq_getattr(): %s\n", strerror(errno));
				break;
			}

			/* Interrupt execution only when there are no messages in the queue */
			if (!mqattr.mq_curmsgs)
#endif
				break;
		}

		/* Allocate temporary buffer size, plus one byte that won't be written to safe guard
		 * the subject NULL termination
		 */
		if (!(tbuf = mm_alloc((size_t) rune.config.core.ipc_msgsize + 1))) {
			log_warn("_exec_process(): tbuf = mm_alloc(): %s\n", strerror(errno));
			continue;
		}

		/* Reset all the memory to 0, granting the extra byte to be 0, so subject will
		 * will always be NULL terminated regardless of the data received from the queue
		 */
		memset(tbuf, 0, (size_t) rune.config.core.ipc_msgsize + 1);

#if CONFIG_USE_IPC_PMQ == 1
		/* Read message from queue */
		if (mq_receive(rune.ipcd, tbuf, (size_t) rune.config.core.ipc_msgsize, 0) < 0) {
			log_warn("_exec_process(): mq_receive(): %s\n", strerror(errno));
			mm_free(tbuf);
			continue;
		}
#elif CONFIG_USE_IPC_SOCK == 1
		/* Get peer credentials */
		if (local_fd_peer_cred(usd_fd, &fd_uid, &fd_gid) < 0) {
			log_warn("_exec_process(): local_fd_peer_cred(): %s\n", strerror(errno));
			continue;
		}

		/* Validate peer UID */
		if (fd_uid != getuid()) {
			log_warn("_exec_process(): fd_uid[%u] != getuid()[%u]\n", (unsigned) fd_uid, (unsigned) getuid());
			continue;
		}

		/* Validate peer GID */
		if (fd_gid != getgid) {
			log_warn("_exec_process(): fd_gid[%u] != getgid[%u]\n", (unsigned) fd_gid, (unsigned) getgid());
			continue;
		}

		/* Read message from unix socket */
		if (panet_read(usd_fd, tbuf, (size_t) rune.config.core.ipc_msgsize) != (ssize_t) rune.config.core.ipc_msgsize) {
			log_warn("_exec_process(): panet_read(): %s\n", strerror(errno));
			mm_free(tbuf);
			continue;
		}
#else
 #error "No IPC mechanism defined."
#endif

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

