/**
 * @file usc.c
 * @brief uSched PHP Extension
 *        uSched PHP Extension interface header - Client
 *
 * Date: 08-03-2015
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

#ifndef PHP_USC_H
#define PHP_USC_H 1

#define PHP_USC_VERSION "0.9a"
#define PHP_USC_EXTNAME "usc"

PHP_MINIT_FUNCTION(usc_minit);
PHP_MSHUTDOWN_FUNCTION(usc_mshutdown);
PHP_RINIT_FUNCTION(usc_rinit);
PHP_RSHUTDOWN_FUNCTION(usc_rshutdown);
PHP_FUNCTION(usc_init);
PHP_FUNCTION(usc_shutdown);
PHP_FUNCTION(usc_test);
PHP_FUNCTION(usc_opt_set_remote_hostname);
PHP_FUNCTION(usc_opt_set_remote_port);
PHP_FUNCTION(usc_opt_set_remote_username);
PHP_FUNCTION(usc_opt_set_remote_password);
PHP_FUNCTION(usc_request);
PHP_FUNCTION(usc_result_get_run);
PHP_FUNCTION(usc_result_get_stop);
PHP_FUNCTION(usc_result_get_show);
PHP_FUNCTION(usc_result_free_run);
PHP_FUNCTION(usc_result_free_stop);
PHP_FUNCTION(usc_result_free_show);
PHP_FUNCTION(usc_usage_error);
PHP_FUNCTION(usc_usage_error_str);

extern zend_module_entry usc_module_entry;
#define phpext_usc_ptr &usc_module_entry

#endif

