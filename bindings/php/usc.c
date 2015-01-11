/**
 * @file usc.c
 * @brief uSched PHP Extension
 *        uSched PHP Extension interface - Client
 *
 * Date: 11-01-2015
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_usc.h"

#include <stdint.h>
#include <string.h>

#include <usched/lib.h>

static zend_function_entry usc_functions[] = {
	PHP_FE(usc_test, NULL)
	PHP_FE(usc_opt_set_remote_hostname, NULL)
	PHP_FE(usc_opt_set_remote_port, NULL)
	PHP_FE(usc_opt_set_remote_username, NULL)
	PHP_FE(usc_opt_set_remote_password, NULL)
	PHP_FE(usc_request, NULL)
	PHP_FE(usc_result_get_run, NULL)
	PHP_FE(usc_result_get_stop, NULL)
	PHP_FE(usc_result_get_show, NULL)
	PHP_FE(usc_result_free_run, NULL)
	PHP_FE(usc_result_free_stop, NULL)
	PHP_FE(usc_result_free_show, NULL)
	PHP_FE(usc_usage_error, NULL)
	PHP_FE(usc_usage_error_str, NULL)
	{ NULL, NULL, NULL }
};

zend_module_entry usc_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
	STANDARD_MODULE_HEADER,
#endif
	PHP_USC_EXTNAME, /* mod name */
	usc_functions, /* mod funcs */
	PHP_MINIT(usc_init), /* init */
	PHP_MSHUTDOWN(usc_shutdown), /* shutdown */
	NULL, /* req init */
	NULL, /* req shutdown */
	NULL, /* mod info */
#if ZEND_MODULE_API_NO >= 20010901
	PHP_USC_VERSION, /* version */
#endif
	STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_USC
ZEND_GET_MODULE(usc)
#endif

PHP_MINIT_FUNCTION(usc_init) {
	usched_init();

	return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(usc_shutdown) {
	usched_destroy();

	return SUCCESS;
}

PHP_FUNCTION(usc_test) {
	RETURN_STRING("Testing uSched extension module...\n", 1);
}

PHP_FUNCTION(usc_opt_set_remote_hostname) {
	char *hostname;
	int len;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &hostname, &len) == FAILURE)
		RETURN_BOOL(0);

	if (usched_opt_set_remote_hostname(hostname) < 0)
		RETURN_BOOL(0);

	RETURN_BOOL(1);
}

PHP_FUNCTION(usc_opt_set_remote_port) {
	char *port = NULL;
	int len;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &port, &len) == FAILURE)
		RETURN_BOOL(0);

	if (usched_opt_set_remote_port(port) < 0)
		RETURN_BOOL(0);

	RETURN_BOOL(1);
}

PHP_FUNCTION(usc_opt_set_remote_username) {
	char *username = NULL;
	int len;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &username, &len) == FAILURE)
		RETURN_BOOL(0);

	if (usched_opt_set_remote_username(username) < 0)
		RETURN_BOOL(0);

	RETURN_BOOL(1);
}

PHP_FUNCTION(usc_opt_set_remote_password) {
	char *password = NULL;
	int len;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &password, &len) == FAILURE)
		RETURN_BOOL(0);

	if (usched_opt_set_remote_password(password) < 0)
		RETURN_BOOL(0);

	RETURN_BOOL(1);
}

PHP_FUNCTION(usc_request) {
	char *req = NULL;
	int len;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &req, &len) == FAILURE)
		RETURN_BOOL(0);

	if (usched_request(req) < 0)
		RETURN_BOOL(0);

	RETURN_BOOL(1);
}

PHP_FUNCTION(usc_result_get_run) {
	int i;
	uint64_t *entry_list;
	size_t nmemb;
	char v[17];

	usched_result_get_run(&entry_list, &nmemb);

	array_init(return_value);

	for (i = 0; i < nmemb; i ++) {
		sprintf(v, "%016llX", (unsigned long long) entry_list[i]);
		add_next_index_string(return_value, v, 1);
	}
}

PHP_FUNCTION(usc_result_get_stop) {
	int i;
	uint64_t *entry_list;
	size_t nmemb;
	char v[17];

	usched_result_get_run(&entry_list, &nmemb);

	array_init(return_value);

	for (i = 0; i < nmemb; i ++) {
		sprintf(v, "%016llX", (unsigned long long) entry_list[i]);
		add_next_index_string(return_value, v, 1);
	}
}

PHP_FUNCTION(usc_result_get_show) {
	/* TODO */
}

PHP_FUNCTION(usc_result_free_run) {
	usched_result_free_run();
}

PHP_FUNCTION(usc_result_free_stop) {
	usched_result_free_stop();
}

PHP_FUNCTION(usc_result_free_show) {
	usched_result_free_show();
}

PHP_FUNCTION(usc_usage_error) {
	RETURN_LONG(usched_usage_error());
}

PHP_FUNCTION(usc_usage_error_str) {
	long error;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &error) == FAILURE)
		RETURN_BOOL(0);

	RETURN_STRING(usched_usage_error_str(error), 1);
}

