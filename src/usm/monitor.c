/**
 * @file monitor.c
 * @brief uSched
 *        Monitoring and Daemonizer interface
 *
 * Date: 02-02-2015
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
 *
 * NOTES:
 *
 *  - This monitor is based on the uCodev daemonizer utility.
 *  - Although uCodev daemonizer is licensed under the 3-Clause BSD License, this particular file
 *    (monitor.c) is licensed unser the GNU General Public License version 3.
 *
 */


#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <ctype.h>

#include <sys/types.h>
#include <sys/wait.h>

#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

#include "config.h"
#include "log.h"
#include "mm.h"

extern char *optarg;
extern int optind, optopt;


/* Configuration flags */
#define CONFIG_FL_PIDF_CREATE	0x01
#define CONFIG_FL_PIDF_FORCE	0x02
#define CONFIG_FL_REDIR_IN	0x04
#define CONFIG_FL_REDIR_OUT	0x08
#define CONFIG_FL_REDIR_ERR	0x10
#define CONFIG_FL_PROC_RESTART	0x20
#define CONFIG_FL_PROC_RSTIGN	0x40
#define CONFIG_FL_PROC_RSTSEGV	0x80

/* Declarations */
struct cmdline_params {
	uid_t uid;
	gid_t gid;
	pid_t cpid;
	char *inf_name;
	char *outf_name;
	char *errf_name;
	char *pidf_name;
	int fd_pidf;
	int flags;
	const char *binary;
	char *const *args;
} config;


/* Implementation */
static void _sigah(int sig, siginfo_t *si, void *ucontext) {
	kill(config.cpid, sig);
}

static void _close_safe(int fd) {
#if 0
	while (close(fd) < 0) {
		if (errno != EINTR)
			break;
	}
#endif
	/* SA_RESTART set */
	close(fd);
}

static int _strisdigit(const char *str) {
	while (*str) {
		if (!isdigit(*str ++))
			return 0;
	}

	return 1;
}

static void _log_init(void) {
	log_monitor_init();
}

static void _log_destroy(void) {
	log_destroy();
}

static void _config_default_init(void) {
	memset(&config, 0, sizeof(struct cmdline_params));

	config.uid = getuid();
	config.gid = getgid();
}

static inline void _config_set_req(const char *binary) {
	config.binary = binary;
}

static inline void _config_set_opt(char *const *argv, int optind) {
	config.args = &argv[optind];
}

static void _config_destroy(void) {
	if (config.flags & CONFIG_FL_PIDF_CREATE)
		mm_free(config.pidf_name);

	if (config.flags & CONFIG_FL_REDIR_ERR)
		mm_free(config.errf_name);

	if (config.flags & CONFIG_FL_REDIR_OUT) 
		mm_free(config.outf_name);

	if (config.flags & CONFIG_FL_REDIR_IN)
		mm_free(config.inf_name);

	_close_safe(config.fd_pidf);

	memset(&config, 0, sizeof(struct cmdline_params));
}

static void _failure(const char *caller) {
	fprintf(stderr, "[%s] %s() error: %s\n", CONFIG_USCHED_MONITOR_PROC_NAME, caller, strerror(errno));
	log_crit("_failure(): %s(): %s\n", caller, strerror(errno));

	_config_destroy();

	exit(EXIT_FAILURE);
}

static void _file_pid_create(const char *file) {
	config.fd_pidf = open(file, O_RDONLY);

	if ((config.fd_pidf >= 0) && (config.flags & CONFIG_FL_PIDF_FORCE)) {
		if (unlink(file) < 0)
			_failure("unlink");
	} else if (config.fd_pidf >= 0) {
		log_crit("_file_pid_create(): PID file %s already exists\n", file);
		fprintf(stderr, "PID file %s already exists.\n", file);
		_config_destroy();
		exit(EXIT_FAILURE);
	}

	_close_safe(config.fd_pidf);

	if ((config.fd_pidf = open(file, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR)) < 0) {
		log_crit("_file_pid_create(): open(): Cannot create PID file %s: %s.\n", file, strerror(errno));
		fprintf(stderr, "Cannot create PID file %s.\n", file);
		_failure("open");
	}
}

static int _file_pid_write(pid_t pid) {
	int ret = 0;
	char spid[12];

	memset(spid, 0, sizeof(spid));

	snprintf(spid, 11, "%u", pid);

	ret = write(config.fd_pidf, spid, strlen(spid)) > 0 ? 0 : -1;

	_close_safe(config.fd_pidf);

	return ret;
}

static int _daemonize(void) {
	pid_t pid = 0;

	if ((pid = fork()) > 0) {
		_config_destroy();
		exit(EXIT_SUCCESS);
	} else if (pid == (pid_t) -1) {
		return -1;
	}

	if (!(freopen(config.flags & CONFIG_FL_REDIR_IN ? config.inf_name : "/dev/zero", "r", stdin)))
		return -1;

	if (!(freopen(config.flags & CONFIG_FL_REDIR_OUT ? config.outf_name : "/dev/null", "a", stdout)))
		return -1;

	if (!(freopen(config.flags & CONFIG_FL_REDIR_ERR ? config.errf_name : "/dev/null", "a", stderr)))
		return -1;

	if (setsid() == (pid_t) -1)
		return -1;

	return 0;
}

static int _bexec(
		const char *file,
		char *const *args,
		char *const *envp)
{
	int status = 0;

	if ((config.cpid = fork()) > 0) {
		if ((config.flags & CONFIG_FL_PIDF_CREATE) && (_file_pid_write(config.cpid) < 0)) {
			log_crit("_bexec(): _file_pid_write(): %s\n", strerror(errno));
			return -1;
		}

		/* SA_RESTART set */
		if ((config.cpid = wait(&status)) == (pid_t) -1) {
			log_crit("_bexec(): wait(): %s\n", strerror(errno));
			return -1;
		}

		if (WEXITSTATUS(status) != EXIT_SUCCESS)
			log_crit("_bexec(): Execution of '%s' terminated with error status code %d.\n", file, WEXITSTATUS(status));
	} else if (!config.cpid) {
		_config_destroy();

		if (setsid() == (pid_t) -1)
			_failure("setsid");

		if (execve(file, args, envp) < 0)
			_failure("execve");

		return -1;
	}

	return status;
}

static void _usage(char *const *argv) {
	fprintf(stderr, "Usage: %s [OPTIONS] COMMAND [ARG]...\n" \
			"\n" \
			"  COMMAND    \tFull path to command line executable\n"\
			"  ARG...     \tCommand line arguments to the " \
			"executable\n" \
			"  OPTIONS\n" \
			"    -u <uid>   \tSet UID\n" \
			"    -g <gid>   \tSet GID\n" \
			"    -i <stdin> \tSource file for stdin redirection\n" \
			"    -o <stdout>\tTarget file for stdout redirection\n"\
			"    -e <stderr>\tTarget file for stderr redirection\n"\
			"    -r         \tRestart process if the child exit " \
			"code is different than zero.\n" \
			"    -R         \tRestart process regardless the " \
			"child exit code.\n" \
			"    -S         \tRestart process on child SIGSEGV\n" \
			"    -f         \tForce PID file rewrite\n" \
			"    -p <file>  \tPID file name\n" \
			"\n", argv[0]);

	exit(EXIT_FAILURE);
}

static void _usage_invalid_opt(int opt, char *const *argv) {
	fprintf(stderr, "Unrecognized option: -%c\n", (char) opt);
	_usage(argv);
}

static void _usage_invalid_opt_arg(
		int opt,
		const char *arg,
		char *const *argv)
{
	fprintf(stderr, "Invalid value for option -%c: %s\n", (char) opt, arg);
	_usage(argv);
}

static void _cmdline_process(int argc, char *const *argv) {
	int opt = 0;

	while ((opt = getopt(argc, argv, "frRSu:g:i:o:e:p:")) != -1) {
		if (opt == 'u') {
			if (!_strisdigit(optarg))
				_usage_invalid_opt_arg(opt, optarg, argv);

			config.uid = strtoul(optarg, NULL, 10);
		} else if (opt == 'g') {
			if (!_strisdigit(optarg))
				_usage_invalid_opt_arg(opt, optarg, argv);

			config.gid = strtoul(optarg, NULL, 10);
		} else if (opt == 'i') {
			config.flags |= CONFIG_FL_REDIR_IN;
			if (!(config.inf_name = mm_alloc(strlen(optarg) + 1)))
				_failure("malloc");

			strcpy(config.inf_name, optarg);
		} else if (opt == 'o') {
			config.flags |= CONFIG_FL_REDIR_OUT;
			if (!(config.outf_name = mm_alloc(strlen(optarg) + 1)))
				_failure("malloc");

			strcpy(config.outf_name, optarg);
		} else if (opt == 'e') {
			config.flags |= CONFIG_FL_REDIR_ERR;
			if (!(config.errf_name = mm_alloc(strlen(optarg) + 1)))
				_failure("malloc");

			strcpy(config.errf_name, optarg);
		} else if (opt == 'p') {
			config.flags |= CONFIG_FL_PIDF_CREATE;
			if (!(config.pidf_name = mm_alloc(strlen(optarg) + 1)))
				_failure("malloc");

			strcpy(config.pidf_name, optarg);
		} else if (opt == 'f') {
			config.flags |= CONFIG_FL_PIDF_FORCE;
		} else if (opt == 'r') {
			config.flags |= CONFIG_FL_PROC_RESTART;
		} else if (opt == 'R') {
			config.flags |= CONFIG_FL_PROC_RESTART;
			config.flags |= CONFIG_FL_PROC_RSTIGN;
		} else if (opt == 'S') {
			config.flags |= CONFIG_FL_PROC_RSTSEGV;
		} else {
			_usage_invalid_opt(optopt, argv);
		}
	}

	if ((argc - optind) < 1)
		_usage(argv);

	_config_set_req(argv[optind]);
	_config_set_opt(argv, optind);
}

static int _signal_init(void) {
	struct sigaction sa;

	memset(&sa, 0, sizeof(struct sigaction));

	sa.sa_flags = SA_SIGINFO | SA_RESTART;
	sigemptyset(&sa.sa_mask);
	sa.sa_sigaction = &_sigah;

	if (sigaction(SIGHUP, &sa, NULL) < 0)
		return -1;

	if (sigaction(SIGINT, &sa, NULL) < 0)
		return -1;

	if (sigaction(SIGQUIT, &sa, NULL) < 0)
		return -1;

	if (sigaction(SIGABRT, &sa, NULL) < 0)
		return -1;

	if (sigaction(SIGTERM, &sa, NULL) < 0)
		return -1;

	if (sigaction(SIGUSR1, &sa, NULL) < 0)
		return -1;

	if (sigaction(SIGUSR2, &sa, NULL) < 0)
		return -1;

	if (sigaction(SIGCONT, &sa, NULL) < 0)
		return -1;

	return 0;
}

void _main_loop_restart_prepare(void) {
	if (config.flags & CONFIG_FL_PIDF_CREATE) {
		if (unlink(config.pidf_name) < 0)
			_failure("unlink");
	}

	log_info("_main_loop_restart_prepare(): Restarting '%s'...\n", config.binary);
}

int main(int argc, char *argv[], char *envp[]) {
	int status = 0;

	_log_init();

	_config_default_init();

	_cmdline_process(argc, argv);

	if (setregid(config.gid, config.gid) < 0)
		_failure("setregid");

	if (setreuid(config.uid, config.uid) < 0)
		_failure("setreuid");

	if (_signal_init() < 0)
		_failure("_signal_init");

	if (_daemonize() < 0)
		_failure("_daemonize");

	for (;;) {
		if (config.flags & CONFIG_FL_PIDF_CREATE)
			_file_pid_create(config.pidf_name);

		/* Make sure that we receive a valid status value... otherwise jump out of here. */
		if ((status = _bexec(config.binary, config.args, envp)) == -1)
			break;

		log_info("main(): _bexec(): Execution of '%s' terminated (Exit status: %d).\n", config.binary, WEXITSTATUS(status));

		/* Check if the exit status refers to a bad runtime state */
		if (WEXITSTATUS(status) == PROCESS_EXIT_STATUS_CUSTOM_BAD_RUNTIME_OR_CONFIG) {
			log_info("main(): Child process \'%s\' won't be restarted due to runtime or configuration errors.", config.binary);

			/* If so, do not attempt to restart the process, even if explicitly requested
			 * by command line options.
			 */
			break;
		}

		if (config.flags & CONFIG_FL_PROC_RSTIGN) {
			log_info("main(): Restarting child \'%s\' regardless of exit status code.", config.binary);

			/* Restart child regardless of signals and exit codes */
			_main_loop_restart_prepare();

			continue;
		} else if ((config.flags & CONFIG_FL_PROC_RSTSEGV) && WIFSIGNALED(status) && (WTERMSIG(status) == SIGSEGV)) {
			log_info("main(): Restarting child \'%s\' due to SIGSEGV.", config.binary);

			/* Restart child if a SIGSEGV was caught */
			_main_loop_restart_prepare();

			continue;
		} else if ((config.flags & CONFIG_FL_PROC_RESTART) && WEXITSTATUS(status)) {
			log_info("main(): Restarting child \'%s\' due to a non-zero exit status code.", config.binary);

			/* Restart child is exit status code isn't zero */
			_main_loop_restart_prepare();

			continue;
		}

		break;
	}

	if (status < 0)
		_failure("_bexec");

	if (config.flags & CONFIG_FL_PIDF_CREATE) {
		if (unlink(config.pidf_name) < 0)
			_failure("unlink");
	}

	_config_destroy();

	_log_destroy();

	return 0;
}

