/**
 * @file network.c
 * @brief uSched
 *        Network configuration and administration interface
 *
 * Date: 03-02-2015
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

#include "config.h"
#include "network.h"


char *network_admin_bind_addr_get(void) {
	return NULL;
}

int network_admin_bind_addr_set(const char *bind_addr) {
	return -1;
}

char *network_admin_bind_port_get(void) {
	return NULL;
}

int network_admin_bind_port_set(const char *bind_port) {
	return -1;
}

char *network_admin_conn_limit_get(void) {
	return NULL;
}

int network_admin_conn_limit_set(const char *conn_limit) {
	return -1;
}

char *network_admin_conn_timeout_get(void) {
	return NULL;
}

int network_admin_conn_timeout_set(const char *conn_timeout) {
	return -1;
}

char *network_admin_sock_named_get(void) {
	return NULL;
}

int network_admin_sock_named_set(const char *sock_named) {
	return -1;
}

