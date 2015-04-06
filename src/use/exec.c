/**
 * @file exec.c
 * @brief uSched
 *        Execution Module Main Component
 *
 * Date: 06-04-2015
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
#include "ipc.h"

extern char **environ;

static int _uss_dispatch(
	uint64_t id,
	uint32_t pid,
	const struct timespec *t_start,
	const struct timespec *t_end,
	uint32_t status,
	const char *outdata)
{
	int errsv = 0;
	char *buf = NULL;
	struct ipc_uss_hdr *hdr = NULL;
	struct timespec ipc_timeout = { CONFIG_USCHED_IPC_TIMEOUT, 0 };

	/* Allocate IPC buffer */
	if (!(buf = mm_alloc((size_t) rune.config.exec.ipc_msgsize))) {
		errsv = errno;
		log_warn("_uss_dispatch(): mm_alloc(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Reset IPC buffer */
	memset(buf, 0, (size_t) rune.config.exec.ipc_msgsize);

	/* Craft IPC message header */
	hdr 		 = (struct ipc_uss_hdr *) buf;
	hdr->id		 = id;
	hdr->status 	 = status;
	hdr->pid	 = pid;
	hdr->outdata_len = strlen(outdata);
	hdr->outdata_len = (hdr->outdata_len >= (rune.config.exec.ipc_msgsize - sizeof(struct ipc_uss_hdr))) ? (rune.config.exec.ipc_msgsize - sizeof(struct ipc_uss_hdr) - 1) : hdr->outdata_len;
	memcpy(&hdr->t_start, t_start, sizeof(struct timespec));
	memcpy(&hdr->t_end, t_end, sizeof(struct timespec));

	/* Validate message size */
	if ((hdr->outdata_len + sizeof(struct timespec) + 1) > (size_t) rune.config.exec.ipc_msgsize) {
		log_warn("_uss_dispatch(): IPC message size too long (Entry ID: 0x%016llX)\n", id);
		mm_free(buf);
		errno = EINVAL;
		return -1;
	}

	/* Append output data to IPC message */
	memcpy(buf + sizeof(struct ipc_uss_hdr), outdata, hdr->outdata_len);

	/* Dispatch IPC message to uss module */
	if (ipc_timedsend(rune.ipcd_uss_wo, buf, (size_t) rune.config.exec.ipc_msgsize, &ipc_timeout) < 0) {
		errsv = errno;
		log_warn("_uss_dispatch(): ipc_timedsend(): %s\n", strerror(errno));
		mm_free(buf);
		errno = errsv;
		return -1;
	}

	/* Free IPC message memory */
	mm_free(buf);

	/* All good */
	return 0;
}

static void *_exec_cmd(void *arg) {
	char *buf = arg;	/* | id (64 bits) | uid (32 bits) | gid (32 bits) | trigger (32 bits) | cmd (...) ... | */
	char child_outdata[CONFIG_USCHED_EXEC_OUTPUT_MAX];
	ssize_t child_outlen = 0;
	pid_t pid = 0;
	int status = 0, opipe[2] = { 0, 0 };
	char *cmd = buf + sizeof(struct ipc_use_hdr);
	struct ipc_use_hdr *hdr = (struct ipc_use_hdr *) buf;
	struct timespec t_start, t_end;

	/* Validate cmd length */
	if (hdr->cmd_len >= (rune.config.core.ipc_msgsize - sizeof(struct ipc_use_hdr))) {
		log_crit("_exec_cmd(): hdr->cmd_len is too long (%u bytes). Entry ID: 0x%016llX\n", hdr->cmd_len, hdr->id);
		goto _exec_finish;
	}

	/* Grant NULL termination */
	cmd[hdr->cmd_len] = 0;

	/* Get the start time of execution */
	clock_gettime(CLOCK_REALTIME, &t_start);

	/* Create a pipe to read child output */
	if (pipe(opipe) < 0)
		log_warn("Entry[0x%016llX]: _exec_cmd(): pipe(): %s\n", hdr->id, strerror(errno));

	/* Check delta time before executing event (Absolute value is a safe check. Negative values
	 * won't occur here... hopefully).
	 */
	if ((unsigned int) labs((long) (time(NULL) - hdr->trigger)) >= rune.config.core.delta_noexec) {
		log_warn("Entry[0x%016llX]: _exec_cmd(): Entry delta T (%u seconds) is >= than the configured delta T for noexec (%d seconds). Ignoring execution...\n", hdr->id, time(NULL) - hdr->trigger, rune.config.core.delta_noexec);
	} else if ((pid = fork()) == (pid_t) -1) {	/* Create a new process, drop privileges to
							 * UID and GID and execute CMD
							 */
		/* Failure */
		log_warn("Entry[0x%016llX]: _exec_cmd(): fork(): %s\n", hdr->id, strerror(errno));

		/* Close pipe */
		close(opipe[0]);
		close(opipe[1]);
	} else if (!pid) {
		/* Child */

		/* Get child pid */
		pid = getpid();

		debug_printf(DEBUG_INFO, "Entry[0x%016llX]: PID[%u]: Executing: %s\nUID: %u\nGID: %u\n", hdr->id, pid, cmd, hdr->uid, hdr->gid);

		/* Close IPC descriptor */
		ipc_close(rune.ipcd_usd_ro);

		/* Close the read end of the output pipe */
		close(opipe[0]);

		/* Duplicate standard output */
		if (dup2(opipe[1], STDOUT_FILENO) < 0)
			debug_printf(DEBUG_INFO, "Entry[0x%016llX]: PID[%u]: dup2(opipe[1], STDOUT_FILENO): %s\n", hdr->id, pid, strerror(errno));

		/* Duplicate standard error */
		if (dup2(opipe[1], STDERR_FILENO) < 0)
			debug_printf(DEBUG_INFO, "Entry[0x%016llX]: PID[%u]: dup2(opipe[1], STDERR_FILENO): %s\n", hdr->id, pid, strerror(errno));

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

#if CONFIG_USCHED_MULTIUSER == 1
		/* Drop privileges, if required */
		if (setregid(hdr->gid, hdr->gid) < 0) {
			/* Free argument resources */
			mm_free(arg);

			exit(CONFIG_SYS_EXIT_CODE_CUSTOM_BASE + CHILD_EXIT_STATUS_FAILED_SETREGID);
		}

		if (setreuid(hdr->uid, hdr->uid) < 0) {
			/* Free argument resources */
			mm_free(arg);

			exit(CONFIG_SYS_EXIT_CODE_CUSTOM_BASE + CHILD_EXIT_STATUS_FAILED_SETREUID);
		}
#endif

		/* Paranoid mode */
		if ((getuid() != (uid_t) hdr->uid) || (geteuid() != (uid_t) hdr->uid)) {
			/* Free argument resources */
			mm_free(arg);

			exit(CONFIG_SYS_EXIT_CODE_CUSTOM_BASE + CHILD_EXIT_STATUS_FAILED_UID);
		}

		if ((getgid() != (gid_t) hdr->gid) || (getegid() != (gid_t) hdr->gid)) {
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
		log_info("Entry[0x%016llX]: PID[%u]: Executing '%s' [uid: %u, gid: %u]. Waiting for child to exit...\n", hdr->id, pid, cmd, hdr->uid, hdr->gid);

		/* Close the write end of the output pipe */
		close(opipe[1]);

		/* Wait for child to return */
		if (waitpid(pid, &status, 0) < 0)
			log_crit("Entry[0x%016llX]: _exec_cmd(): waitpid(): %s\n", hdr->id, strerror(errno));

		/* Read the output from the child process */
		memset(child_outdata, 0, sizeof(child_outdata));
		child_outlen = read(opipe[0], child_outdata, sizeof(child_outdata) - 1);

		/* Process the output buffer */
		if (child_outlen < 0) {
			/* Set the error as the output */
			snprintf(child_outdata, sizeof(child_outdata) - 1, "Entry[0x%016llX]: read(): %s\n", (unsigned long long) hdr->id, strerror(errno));
		} else {
			/* Grant NULL terination on output data buffer */
			child_outdata[child_outlen] = 0;
		}

		/* Close the read end of the pipe */
		close(opipe[0]);

		debug_printf(DEBUG_INFO, "Entry[0x%016llX]: Output: %s\n", hdr->id, child_outdata);

		/* Log errors (if any) based on exit status */
		switch (WEXITSTATUS(status)) {
			case CONFIG_SYS_EXIT_CODE_CUSTOM_BASE + CHILD_EXIT_STATUS_FAILED_SETSID: {
				log_warn("Entry[0x%016llX]: PID[%u]: setsid() failed.\n", hdr->id, pid);
			} break;
			case CONFIG_SYS_EXIT_CODE_CUSTOM_BASE + CHILD_EXIT_STATUS_FAILED_FREOPEN_STDIN: {
				log_warn("Entry[0x%016llX]: PID[%u]: freopen(%s, \"r\", stdout) failed.\n", hdr->id, pid, CONFIG_SYS_DEV_ZERO);
			} break;
			case CONFIG_SYS_EXIT_CODE_CUSTOM_BASE + CHILD_EXIT_STATUS_FAILED_SETREGID: {
				log_warn("Entry[0x%016llX]: PID[%u]: setregid() failed.\n", hdr->id, pid);
			} break;
			case CONFIG_SYS_EXIT_CODE_CUSTOM_BASE + CHILD_EXIT_STATUS_FAILED_SETREUID: {
				log_warn("Entry[0x%016llX]: PID[%u]: setreuid() failed.\n", hdr->id, pid);
			} break;
			case CONFIG_SYS_EXIT_CODE_CUSTOM_BASE + CHILD_EXIT_STATUS_FAILED_UID: {
				log_warn("Entry[0x%016llX]: PID[%u]: Failed to drop process privileges (UID).", hdr->id, pid);
			} break;
			case CONFIG_SYS_EXIT_CODE_CUSTOM_BASE + CHILD_EXIT_STATUS_FAILED_GID: {
				log_warn("Entry[0x%016llX]: PID[%u]: Failed to drop process privileges (GID).", hdr->id, pid);
			} break;
			case CONFIG_SYS_EXIT_CODE_CUSTOM_BASE + CHILD_EXIT_STATUS_FAILED_EXECLP: {
				log_warn("Entry[0x%016llX]: PID[%u]: execlp() failed.", hdr->id, pid);
			} break;
			case EXIT_SUCCESS:
			default: break;
		}

		log_info("Entry[0x%016llX]: PID[%u]: Child exited. Exit Status: %d\n", hdr->id, pid, WEXITSTATUS(status));
	}

	/* Get the end time of the execution */
	clock_gettime(CLOCK_REALTIME, &t_end);

	/* Send status and statistical data to uSched Status and Statistics */
	if (_uss_dispatch(hdr->id, pid, &t_start, &t_end, status, child_outdata) < 0)
		log_warn("_exec_cmd(): _uss_dispatch(): %s\n", strerror(errno));

_exec_finish:
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

	for (;;) {
		/* Check for rutime interruptions */
		if (runtime_exec_interrupted()) {
			/* Interrupt execution only when there are no messages in the queue */
			if (!ipc_pending(rune.ipcd_usd_ro))
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

		/* Wait for IPC message */
		if (ipc_recv(rune.ipcd_usd_ro, tbuf, (size_t) rune.config.core.ipc_msgsize) < 0) {
			log_warn("_exec_process(): ipc_recv(): %s\n", strerror(errno));
			mm_free(tbuf);
			continue;
		}

		/* Create a new thread for command execution */
		if ((errno = pthread_create(&ptid, NULL, _exec_cmd, tbuf))) {
			log_warn("_exec_process(): pthread_create(): %s\n", strerror(errno));
			mm_free(tbuf);
			continue;
		}

		/* Detach the newly created thread as the resources should be automatically free'd
		 * upon thread termination.
		 */
		if ((errno = pthread_detach(ptid)))
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

