/**
 * @file lib.h
 * @brief uSched
 *        Client Library interface header
 *
 * Date: 07-03-2015
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


#ifndef USCHED_LIB_H
#define USCHED_LIB_H

#include <stdint.h>

#include "config.h"
#include "entry.h"
#include "usage.h"

/* Library Interface Prototypes */

/**
 * @brief
 *   Initializes the uSched Client library interface.
 *
 * @return
 *   On success, 0 is returned. On error, -1 is returned and errno is set appropriately.
 *   \n\n
 *   Errors: ENOMEM
 *
 * @see usched_destroy()
 * @see usched_request()
 *
 */
#ifdef COMPILE_WIN32
DLLIMPORT
#endif
int usched_init(void);

/**
 * @brief
 *   Perform the scheduling request contained in the parameter 'req'.
 *
 * @param req
 *   A NULL terminated string containing a valid uSched Client request.
 *
 * @return
 *   On success, zero is returned. On error, -1 is returned and errno is set appropriately.
 *   \n\n
 *   Errors: ENOMEM, EINVAL, EAGAIN
 *
 * @see usched_init()
 * @see usched_opt_set_remote_hostname()
 * @see usched_opt_set_remote_port()
 * @see usched_opt_set_remote_username()
 * @see usched_opt_set_remote_password()
 * @see usched_result_get_run()
 * @see usched_result_get_show()
 * @see usched_result_get_stop()
 * @see usched_result_free_run()
 * @see usched_result_free_show()
 * @see usched_result_free_stop()
 * @see usched_usage_error()
 * @see usched_usage_error_str()
 * @see usched_destroy()
 *
 */
#ifdef COMPILE_WIN32
DLLIMPORT
#endif
int usched_request(char *req);

/**
 * @brief
 *   Set the remote 'hostname' to which the client library will connect to.
 *
 * @param hostname
 *   A NULL terminated string containing the remote hostname.
 *
 * @return
 *   On success, zero is returned. On error, -1 is returned and errno is set appropriately.
 *   \n\n
 *   Errors: ENOMEM, EINVAL
 *
 * @see usched_opt_set_remote_port()
 * @see usched_opt_set_remote_username()
 * @see usched_opt_set_remote_password()
 *
 */ 
#ifdef COMPILE_WIN32
DLLIMPORT
#endif
int usched_opt_set_remote_hostname(char *hostname);

/**
 * @brief
 *   Set the remote TCP 'port' to which the client library will connect to.
 *
 * @param port
 *   A NULL terminated string containing the remote TCP port.
 *
 * @return
 *   On success, zero is returned. On error, -1 is returned and errno is set appropriately.
 *   \n\n
 *   Errors: ENOMEM, EINVAL
 *
 * @see usched_opt_set_remote_hostname()
 * @see usched_opt_set_remote_username()
 * @see usched_opt_set_remote_password()
 *
 */ 
#ifdef COMPILE_WIN32
DLLIMPORT
#endif
int usched_opt_set_remote_port(char *port);

/**
 * @brief
 *   Set the remote authentication 'username'.
 *
 * @param username
 *   A NULL terminated string containing the remote authentication username.
 *
 * @return
 *   On success, zero is returned. On error, -1 is returned and errno is set appropriately.
 *   \n\n
 *   Errors: ENOMEM, EINVAL
 *
 * @see usched_opt_set_remote_hostname()
 * @see usched_opt_set_remote_port()
 * @see usched_opt_set_remote_password()
 *
 */ 
#ifdef COMPILE_WIN32
DLLIMPORT
#endif
int usched_opt_set_remote_username(char *username);

/**
 * @brief
 *   Set the remote authentication 'password'.
 *
 * @param password
 *   A NULL terminated string containing the remote authentication password.
 *
 * @return
 *   On success, zero is returned. On error, -1 is returned and errno is set appropriately.
 *   \n\n
 *   Errors: ENOMEM, EINVAL
 *
 * @see usched_opt_set_remote_hostname()
 * @see usched_opt_set_remote_port()
 * @see usched_opt_set_remote_username()
 *
 */ 
#ifdef COMPILE_WIN32
DLLIMPORT
#endif
int usched_opt_set_remote_password(char *password);

/**
 * @brief
 *   Retrieves the results of a successful RUN request, performed by usched_request(). The results
 *   are stored into the 'entry_list' parameter. The number of entries contained in the 'entry_list'
 *   are stored by the variable pointed by 'nmemb'.
 *
 * @param entry_list
 *   An array of type 'uint64_t'. Each entry contains the ID of the successfully installed entries.
 *
 * @param nmemb
 *   The number of entries in the 'entry_list' array.
 *
 * @see usched_request()
 * @see usched_result_get_run()
 * @see usched_result_get_stop()
 *
 */ 
#ifdef COMPILE_WIN32
DLLIMPORT
#endif
void usched_result_get_run(uint64_t **entry_list, size_t *nmemb);

/**
 * @brief
 *   Retrieves the results of a successful STOP request, performed by usched_request(). The results
 *   are stored into the 'entry_list' parameter. The number of entries contained in the 'entry_list'
 *   are stored by the variable pointed by 'nmemb'.
 *
 * @param entry_list
 *   An array of type 'uint64_t'. Each entry contains the ID of the successfully removed entries.
 *
 * @param nmemb
 *   The number of entries in the 'entry_list' array.
 *
 * @see usched_request()
 * @see usched_result_get_run()
 * @see usched_result_get_stop()
 *
 */ 
#ifdef COMPILE_WIN32
DLLIMPORT
#endif
void usched_result_get_stop(uint64_t **entry_list, size_t *nmemb);

/**
 * @brief
 *   Retrieves the results of a successful SHOW request, performed by usched_request(). The results
 *   are stored into the 'entry_list' parameter. The number of entries contained in the 'entry_list'
 *   are stored by the variable pointed by 'nmemb'.
 *
 * @param entry_list
 *   An array of type 'struct usched_entry'. Each element of the array is an installed scheduled
 *   entry on the target uSched Daemon.
 *
 * @param nmemb
 *   The number of entries in the 'entry_list' array.
 *
 * @see usched_entry
 * @see usched_request()
 * @see usched_result_get_run()
 * @see usched_result_get_stop()
 *
 */ 
#ifdef COMPILE_WIN32
DLLIMPORT
#endif
void usched_result_get_show(struct usched_entry **entry_list, size_t *nmemb);

/**
 * @brief
 *   Releases the resources used by a successful RUN request, performed by the usched_request().
 *   This function should only be called after the usched_result_get_run() was processed.
 *
 * @see usched_result_free_stop()
 * @see usched_result_free_show()
 *
 */ 
#ifdef COMPILE_WIN32
DLLIMPORT
#endif
void usched_result_free_run(void);

/**
 * @brief
 *   Releases the resources used by a successful STOP request, performed by the usched_request().
 *   This function should only be called after the usched_result_get_stop() was processed.
 *
 * @see usched_result_free_run()
 * @see usched_result_free_show()
 *
 */ 
#ifdef COMPILE_WIN32
DLLIMPORT
#endif
void usched_result_free_stop(void);

/**
 * @brief
 *   Releases the resources used by a successful SHOW request, performed by the usched_request().
 *   This function should only be called after the usched_result_get_show() was processed.
 *
 * @see usched_reuslt_free_run()
 * @see usched_result_free_show()
 *
 */ 
#ifdef COMPILE_WIN32
DLLIMPORT
#endif
void usched_result_free_show(void);

/**
 * @brief
 *   Retrieves the error value of a failed usched_request() call.
 *
 * @return
 *   Returns an error value that can be passed to the usched_usage_error_str() function.
 *
 * @see usched_usage_error_str()
 *
 */ 
#ifdef COMPILE_WIN32
DLLIMPORT
#endif
usched_usage_client_err_t usched_usage_error(void);

/**
 * @brief
 *   Converts an 'error' value into a human readable string.
 *
 * @param error
 *   An error value returned by the usched_usage_error() function.
 *
 * @return
 *   Returns a human readable string based on the 'error' value.
 *
 * @see usched_usage_error()
 *
 */ 
#ifdef COMPILE_WIN32
DLLIMPORT
#endif
char *usched_usage_error_str(usched_usage_client_err_t error);

/**
 * @brief
 *   Unitializes the uSched Client library interface.
 *
 * @see usched_init()
 * @see usched_request()
 *
 */
#ifdef COMPILE_WIN32
DLLIMPORT
#endif
void usched_destroy(void);

#endif

