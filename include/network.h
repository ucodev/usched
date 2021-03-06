/**
 * @file network.h
 * @brief uSched
 *        Network configuration and administration interface header
 *
 * Date: 20-02-2015
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

#ifndef USCHED_NETWORK_H
#define USCHED_NETWORK_H


/* Prototypes */
int network_admin_commit(void);
int network_admin_rollback(void);
int network_admin_show(void);
int network_admin_bind_addr_show(void);
int network_admin_bind_addr_change(const char *bind_addr);
int network_admin_bind_port_show(void);
int network_admin_bind_port_change(const char *bind_port);
int network_admin_conn_limit_show(void);
int network_admin_conn_limit_change(const char *conn_limit);
int network_admin_conn_timeout_show(void);
int network_admin_conn_timeout_change(const char *conn_timeout);
int network_admin_sock_name_show(void);
int network_admin_sock_name_change(const char *sock_named);

#endif

