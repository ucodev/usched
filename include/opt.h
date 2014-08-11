/**
 * @file opt.h
 * @brief uSched
 *        Optional arguments interface header
 *
 * Date: 11-08-2014
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

#ifndef USCHED_OPT
#define USCHED_OPT

struct usched_opt_client {
	/* Command line options */
	char remote_hostname[256];	/* IPv4 (15 bytes + 1 '\0')
					 * IPv6 (39 bytes + 1 '\0')
					 * Hostname (max 255 bytes + 1 '\0')
					 */
	char remote_port[6];		/* 5 digits (5 bytes + 1 '\0') */
	char remote_username[32];	/* Max 32 bytes */
	char remote_password[128];	/* Max 128 bytes */
};

/* Prototypes */
int opt_client_process(int argc, char **argv, struct usched_opt_client *opt_client);

#endif

