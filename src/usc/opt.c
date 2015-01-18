/**
 * @file opt.c
 * @brief uSched
 *        Optional arguments interface - Client
 *
 * Date: 18-01-2015
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

#include "config.h"

#ifndef COMPILE_WIN32
#include <unistd.h>
#endif

#include "usage.h"
#include "opt.h"
#include "input.h"

extern char *optarg;
extern int optind, optopt;

static int _opt_client_remote_host(const char *hostname, struct usched_opt_client *dest) {
	if (!hostname || !hostname[0]) {
		puts("Remote hostname value is empty.\n");
		errno = EINVAL;
		return -1;
	}

	if (strlen(hostname) >= sizeof(dest->remote_hostname)) {
		puts("Hostname too long.");
		errno = EINVAL;
		return -1;
	}

	memset(dest->remote_hostname, 0, sizeof(dest->remote_hostname));

	strcpy(dest->remote_hostname, hostname);
		
	return 0;
}

static int _opt_client_remote_port(const char *port, struct usched_opt_client *dest) {
	if (!port || !port[0]) {
		puts("Remote port value is empty.");
		errno = EINVAL;
		return -1;
	}

	if (strlen(port) >= sizeof(dest->remote_port)) {
		puts("Remote port value is too long.");
		errno = EINVAL;
		return -1;
	}

	memset(dest->remote_port, 0, sizeof(dest->remote_port));

	strcpy(dest->remote_port, port);

	return 0;
}

static int _opt_client_remote_username(const char *username, struct usched_opt_client *dest) {
	if (!username || !username[0]) {
		puts("Username is empty.");
		errno = EINVAL;
		return -1;
	}

	if (strlen(username) >= sizeof(dest->remote_username)) {
		puts("Username too long.");
		errno = EINVAL;
		return -1;
	}

	memset(dest->remote_username, 0, sizeof(dest->remote_username));

	strcpy(dest->remote_username, username);

	return 0;
}

static int _opt_client_remote_password(const char *password, struct usched_opt_client *dest) {
	if (!password || !password[0]) {
		puts("Password is empty.");
		errno = EINVAL;
		return -1;
	}

	if (strlen(password) < CONFIG_USCHED_AUTH_PASSWORD_MIN) {
		puts("Password too short.");
		errno = EINVAL;
		return -1;
	}

	if (strlen(password) >= sizeof(dest->remote_password)) {
		puts("Password too long.");
		errno = EINVAL;
		return -1;
	}

	memset(dest->remote_password, 0, sizeof(dest->remote_password));

	strcpy(dest->remote_password, password);

	return 0;
}

int opt_client_process(int argc, char **argv, struct usched_opt_client *opt_client) {
	int opt = 0;
	char password[CONFIG_USCHED_AUTH_PASSWORD_MAX + 1];

	/* Parse command line options */
	while ((opt = getopt(argc, argv, "hH:p:U:P:")) != -1) {
		if (opt == 'h') {
			usage_client_show();
			return 0;
		} else if (opt == 'H') {
			if (_opt_client_remote_host(optarg, opt_client) < 0) {
				usage_client_show();
				return -1;
			}
		} else if (opt == 'p') {
			if (_opt_client_remote_port(optarg, opt_client) < 0) {
				usage_client_show();
				return -1;
			}
		} else if (opt == 'U') {
			if (_opt_client_remote_username(optarg, opt_client) < 0) {
				usage_client_show();
				return -1;
			}
		} else if (opt == 'P') {
			if (_opt_client_remote_password(optarg, opt_client) < 0) {
				usage_client_show();
				return -1;
			}
		} else {
			usage_client_show();
			return -1;
		}
	}

	/* If a remote connection is required and no password is set, request it via terminal */
	if (opt_client->remote_hostname[0] && opt_client->remote_username[0] && !opt_client->remote_password[0]) {
		/* Request password from terminal */
		printf("Password: ");

		if (input_password(password, sizeof(password)) < 0)
			return -1;

		puts("");

		/* Set the password argument */
		if (_opt_client_remote_password(password, opt_client) < 0) {
			usage_client_show();
			return -1;
		}
	}

	/* If there are arguments, we must grant that they make sense */
	if ((optind != 1) && (!opt_client->remote_hostname[0] || !opt_client->remote_username[0] || !opt_client->remote_password[0])) {
		usage_client_show();
		return -1;
	}

	/* Set the default port if not defined */
	if (!opt_client->remote_port[0])
		memcpy(opt_client->remote_port, CONFIG_USCHED_NET_DEFAULT_PORT, sizeof(CONFIG_USCHED_NET_DEFAULT_PORT));

	return optind;
}

