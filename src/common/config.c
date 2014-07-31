/**
 * @file config.c
 * @brief uSched
 *        Configuration interface
 *
 * Date: 31-07-2014
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include <pall/cll.h>

#include <fsop/path.h>

#include "config.h"
#include "mm.h"
#include "log.h"


static int _list_uint_compare(const void *d1, const void *d2) {
	unsigned int u1 = *(unsigned int *) d1, u2 = *(unsigned int *) d2;

	if (u1 > u2)
		return 1;

	if (u1 < u2)
		return -1;

	return 0;
}

static void _list_uint_destroy(void *data) {
	mm_free(data);
}

static int _list_init_uint_from_file(const char *file, struct cll_handler **list) {
	int errsv = 0;
	FILE *fp = NULL;
	char *endptr = NULL;
	char line[32];
	unsigned int *val = NULL;
	unsigned int line_count = 0;

	/* Reset line buffer memory */
	memset(line, 0, sizeof(line));

	/* Grant that file exists and is a regular file */
	if (!fsop_path_isreg(file)) {
		errsv = errno;
		log_warn("_list_init_uint_from_file(): fsop_path_isreg(\"%s\"): %s\n", file, strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Try to open file */
	if (!(fp = fopen(file, "r"))) {
		errsv = errno;
		log_warn("_list_init_uint_from_file(): fopen(\"%s\", \"r\"): %s\n", file, strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Initialize gid blacklist */
	if (!(*list = pall_cll_init(&_list_uint_compare, &_list_uint_destroy, NULL, NULL))) {
		errsv = errno;
		log_warn("_list_init_uint_from_file(): fopen(\"%s\", \"r\"): %s\n", file, strerror(errno));
		fclose(fp);
		errno = errsv;
		return -1;
	}

	/* Read file contents */
	while (fgets(line, sizeof(line) - 1, fp)) {
		++ line_count;

		/* Ignore blank lines */
		if (!line[0] || (line[0] == '\n'))
			continue;

		/* Strip '\n' */
		line[strlen(line) - 1] = 0;

		/* Allocate value memory */
		if (!(val = mm_alloc(sizeof(unsigned int)))) {
			errsv = errno;
			log_warn("_list_init_uint_from_file(): mm_alloc(): %s\n", strerror(errno));
			fclose(fp);
			pall_cll_destroy(*list);
			errno = errsv;
			return -1;
		}

		/* Retrieve line value */
		*val = strtoul(line, &endptr, 0);

		/* Check if value is acceptable */
		if ((*endptr) || (endptr == line) || (errno == EINVAL) || (errno == ERANGE)) {
			log_warn("_list_init_uint_from_file(): Invalid value found on line %s:%lu.\n", file, line_count);
			fclose(fp);
			pall_cll_destroy(*list);
			errno = EINVAL;
			return -1;
		}

		/* Insert value into list */
		if ((*list)->insert(*list, val) < 0) {
			errsv = errno;
			log_warn("_list_init_uint_from_file(): list->insert(): %s\n", strerror(errno));
			fclose(fp);
			pall_cll_destroy(*list);
			errno = errsv;
			return -1;
		}
	}

	/* Close file */
	fclose(fp);

	/* Success */
	return 0;
}

static int _value_init_uint_from_file(const char *file, unsigned int *val) {
	int errsv = 0;
	FILE *fp = NULL;
	char *endptr = NULL;
	char line[32];

	/* Reset line buffer memory */
	memset(line, 0, sizeof(line));

	/* Grant that file exists and is a regular file */
	if (!fsop_path_isreg(file)) {
		errsv = errno;
		log_warn("_value_init_uint_from_file(): fsop_path_isreg(\"%s\"): %s\n", file, strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Try to open file */
	if (!(fp = fopen(file, "r"))) {
		errsv = errno;
		log_warn("_value_init_uint_from_file(): fopen(\"%s\", \"r\"): %s\n", file, strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Retrieve value from file */
	if (!fgets(line, sizeof(line) - 1, fp)) {
		errsv = errno;
		log_warn("_value_init_uint_from_file(): fgets(): Unexpected EOF (%s).\n", file);
		fclose(fp);
		errno = errsv;
		return -1;
	}

	/* Close file */
	fclose(fp);

	/* Check if line is empty */
	if (!line[0] || (line[0] == '\n')) {
		errsv = errno;
		log_warn("_value_init_uint_from_file(): First line of file %s is empty.\n", file);
		errno = errsv;
		return -1;
	}

	/* Strip '\n' */
	line[strlen(line) - 1] = 0;

	/* Retrieve line value */
	*val = strtoul(line, &endptr, 0);

	/* Check if value is acceptable */
	if ((*endptr) || (endptr == line) || (errno == EINVAL) || (errno == ERANGE)) {
		log_warn("_value_init_uint_from_file(): Invalid value found on file %s.\n", file);
		errno = EINVAL;
		return -1;
	}

	/* Success */
	return 0;
}

static int _value_init_int_from_file(const char *file, int *val) {
	int errsv = 0;
	FILE *fp = NULL;
	char *endptr = NULL;
	char line[32];

	/* Reset line buffer memory */
	memset(line, 0, sizeof(line));

	/* Grant that file exists and is a regular file */
	if (!fsop_path_isreg(file)) {
		errsv = errno;
		log_warn("_value_init_int_from_file(): fsop_path_isreg(\"%s\"): %s\n", file, strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Try to open file */
	if (!(fp = fopen(file, "r"))) {
		errsv = errno;
		log_warn("_value_init_int_from_file(): fopen(\"%s\", \"r\"): %s\n", file, strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Retrieve value from file */
	if (!fgets(line, sizeof(line) - 1, fp)) {
		errsv = errno;
		log_warn("_value_init_int_from_file(): fgets(): Unexpected EOF (%s).\n", file);
		fclose(fp);
		errno = errsv;
		return -1;
	}

	/* Close file */
	fclose(fp);

	/* Check if line is empty */
	if (!line[0] || (line[0] == '\n')) {
		errsv = errno;
		log_warn("_value_init_int_from_file(): First line of file %s is empty.\n", file);
		errno = errsv;
		return -1;
	}

	/* Strip '\n' */
	line[strlen(line) - 1] = 0;

	/* Retrieve line value */
	*val = strtol(line, &endptr, 0);

	/* Check if value is acceptable */
	if ((*endptr) || (endptr == line) || (errno == EINVAL) || (errno == ERANGE)) {
		log_warn("_value_init_int_from_file(): Invalid value found on file %s.\n", file);
		errno = EINVAL;
		return -1;
	}

	/* Success */
	return 0;
}

static int _value_init_string_from_file(const char *file, char **string) {
	int errsv = 0;
	FILE *fp = NULL;
	char line[8192];
	size_t len = 0;

	/* Reset line buffer memory */
	memset(line, 0, sizeof(line));

	/* Grant that file exists and is a regular file */
	if (!fsop_path_isreg(file)) {
		errsv = errno;
		log_warn("_value_init_string_from_file(): fsop_path_isreg(\"%s\"): %s\n", file, strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Try to open file */
	if (!(fp = fopen(file, "r"))) {
		errsv = errno;
		log_warn("_value_init_string_from_file(): fopen(\"%s\", \"r\"): %s\n", file, strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Retrieve value from file */
	if (!fgets(line, sizeof(line) - 1, fp)) {
		errsv = errno;
		log_warn("_value_init_string_from_file(): fgets(): Unexpected EOF (%s).\n", file);
		fclose(fp);
		errno = errsv;
		return -1;
	}

	/* Close file */
	fclose(fp);

	/* Check if line is empty */
	if (!line[0] || (line[0] == '\n')) {
		errsv = errno;
		log_warn("_value_init_string_from_file(): First line of file %s is empty.\n", file);
		errno = errsv;
		return -1;
	}

	/* Strip '\n' */
	len = strlen(line);
	line[len - 1] = 0;

	/* Allocate memory for string */
	if (!(*string = mm_alloc(len))) {
		errsv = errno;
		log_warn("_value_init_string_from_file(): mm_alloc(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	memset(*string, 0, len);

	memcpy(*string, line, len - 1);

	/* Success */
	return 0;
}

static int _config_init_auth_gid_blacklist(struct usched_config_auth *auth) {
	return _list_init_uint_from_file(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_AUTH "/" CONFIG_USCHED_FILE_AUTH_GID_BL, &auth->gid_blacklist);
}

static int _config_init_auth_gid_whitelist(struct usched_config_auth *auth) {
	return _list_init_uint_from_file(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_AUTH "/" CONFIG_USCHED_FILE_AUTH_GID_WL, &auth->gid_whitelist);
}

static int _config_init_auth_uid_blacklist(struct usched_config_auth *auth) {
	return _list_init_uint_from_file(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_AUTH "/" CONFIG_USCHED_FILE_AUTH_UID_BL, &auth->uid_blacklist);
}

static int _config_init_auth_uid_whitelist(struct usched_config_auth *auth) {
	return _list_init_uint_from_file(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_AUTH "/" CONFIG_USCHED_FILE_AUTH_UID_WL, &auth->uid_whitelist);
}

static int _config_init_auth_use_local(struct usched_config_auth *auth) {
	return _value_init_uint_from_file(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_AUTH "/" CONFIG_USCHED_FILE_AUTH_USE_LOCAL, &auth->use_local);
}

static int _config_init_auth_use_pam(struct usched_config_auth *auth) {
	return _value_init_uint_from_file(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_AUTH "/" CONFIG_USCHED_FILE_AUTH_USE_PAM, &auth->use_pam);
}

static int _config_init_auth_users_remote(struct usched_config_auth *auth) {
	return _value_init_uint_from_file(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_AUTH "/" CONFIG_USCHED_FILE_AUTH_USERS_REMOTE, &auth->users_remote);
}

static int _config_init_auth(struct usched_config_auth *auth) {
	int errsv = 0;

	/* Read GID blacklist */
	if (_config_init_auth_gid_blacklist(auth) < 0) {
		errsv = errno;
		log_warn("_config_init_auth(): _config_init_auth_gid_blacklist(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Read GID whitelist */
	if (_config_init_auth_gid_whitelist(auth) < 0) {
		errsv = errno;
		log_warn("_config_init_auth(): _config_init_auth_gid_whitelist(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Read UID blacklist */
	if (_config_init_auth_uid_blacklist(auth) < 0) {
		errsv = errno;
		log_warn("_config_init_auth(): _config_init_auth_uid_blacklist(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Read UID whitelist */
	if (_config_init_auth_uid_whitelist(auth) < 0) {
		errsv = errno;
		log_warn("_config_init_auth(): _config_init_auth_uid_whitelist(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Read use local */
	if (_config_init_auth_use_local(auth) < 0) {
		errsv = errno;
		log_warn("_config_init_auth(): _config_init_auth_use_local(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Read use pam */
	if (_config_init_auth_use_pam(auth) < 0) {
		errsv = errno;
		log_warn("_config_init_auth(): _config_init_auth_use_pam(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Read users remote */
	if (_config_init_auth_users_remote(auth) < 0) {
		errsv = errno;
		log_warn("_config_init_auth(): _config_init_auth_users_remote(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Success */
	return 0;
}

static int _config_init_core_file_serialize(struct usched_config_core *core) {
	return _value_init_string_from_file(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_CORE "/" CONFIG_USCHED_FILE_CORE_FILE_SERIALIZE, &core->file_serialize);
}

static int _config_init_core_pmq_msgmax(struct usched_config_core *core) {
	return _value_init_uint_from_file(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_CORE "/" CONFIG_USCHED_FILE_CORE_PMQ_MSGMAX, &core->pmq_msgmax);
}

static int _config_init_core_pmq_msgsize(struct usched_config_core *core) {
	return _value_init_uint_from_file(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_CORE "/" CONFIG_USCHED_FILE_CORE_PMQ_MSGSIZE, &core->pmq_msgsize);
}

static int _config_init_core_pmq_name(struct usched_config_core *core) {
	return _value_init_string_from_file(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_CORE "/" CONFIG_USCHED_FILE_CORE_PMQ_NAME, &core->pmq_name);
}

static int _config_init_core_thread_priority(struct usched_config_core *core) {
	return _value_init_int_from_file(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_CORE "/" CONFIG_USCHED_FILE_CORE_THREAD_PRIORITY, &core->thread_priority);
}

static int _config_init_core_thread_workers(struct usched_config_core *core) {
	return _value_init_uint_from_file(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_CORE "/" CONFIG_USCHED_FILE_CORE_THREAD_WORKERS, &core->thread_workers);
}

static int _config_init_core(struct usched_config_core *core) {
	int errsv = 0;

	/* Read file serialize */
	if (_config_init_core_file_serialize(core) < 0) {
		errsv = errno;
		log_warn("_config_init_core(): _config_init_core_file_serialize(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Read pmq msg max */
	if (_config_init_core_pmq_msgmax(core) < 0) {
		errsv = errno;
		log_warn("_config_init_core(): _config_init_core_pmq_msgmax(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Read pmq msg size */
	if (_config_init_core_pmq_msgsize(core) < 0) {
		errsv = errno;
		log_warn("_config_init_core(): _config_init_core_pmq_msgsize(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Read pmq name */
	if (_config_init_core_pmq_name(core) < 0) {
		errsv = errno;
		log_warn("_config_init_core(): _config_init_core_pmq_name(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Read thread priority */
	if (_config_init_core_thread_priority(core) < 0) {
		errsv = errno;
		log_warn("_config_init_core(): _config_init_core_thread_priority(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Read thread workers */
	if (_config_init_core_thread_workers(core) < 0) {
		errsv = errno;
		log_warn("_config_init_core(): _config_init_core_thread_workers(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Success */
	return 0;
}

static int _config_init_network_bind_addr(struct usched_config_network *network) {
	return _value_init_string_from_file(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_NETWORK "/" CONFIG_USCHED_FILE_NETWORK_BIND_ADDR, &network->bind_addr);
}

static int _config_init_network_bind_port(struct usched_config_network *network) {
	return _value_init_string_from_file(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_NETWORK "/" CONFIG_USCHED_FILE_NETWORK_BIND_PORT, &network->bind_port);
}

static int _config_init_network_conn_limit(struct usched_config_network *network) {
	return _value_init_uint_from_file(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_NETWORK "/" CONFIG_USCHED_FILE_NETWORK_CONN_LIMIT, &network->conn_limit);
}

static int _config_init_network_conn_timeout(struct usched_config_network *network) {
	return _value_init_uint_from_file(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_NETWORK "/" CONFIG_USCHED_FILE_NETWORK_CONN_TIMEOUT, &network->conn_timeout);
}

static int _config_init_network_sock_named(struct usched_config_network *network) {
	return _value_init_string_from_file(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_NETWORK "/" CONFIG_USCHED_FILE_NETWORK_SOCK_NAMED, &network->sock_named);
}

static int _config_init_network(struct usched_config_network *network) {
	int errsv = 0;

	/* Read bind addr */
	if (_config_init_network_bind_addr(network) < 0) {
		errsv = errno;
		log_warn("_config_init_network(): _config_init_network_bind_addr(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Read bind port */
	if (_config_init_network_bind_port(network) < 0) {
		errsv = errno;
		log_warn("_config_init_network(): _config_init_network_bind_port(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Read conn limit */
	if (_config_init_network_conn_limit(network) < 0) {
		errsv = errno;
		log_warn("_config_init_network(): _config_init_network_conn_limit(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Read conn timeout */
	if (_config_init_network_conn_timeout(network) < 0) {
		errsv = errno;
		log_warn("_config_init_network(): _config_init_network_conn_timeout(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Read sock named */
	if (_config_init_network_sock_named(network) < 0) {
		errsv = errno;
		log_warn("_config_init_network(): _config_init_network_sock_named(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Success */
	return 0;
}

static int _config_init_users(struct usched_config_users *users) {
	errno = ENOSYS;
	return -1;
}

static void _config_destroy_auth(struct usched_config_auth *auth) {
	return ;
}

static void _config_destroy_core(struct usched_config_core *core) {
	return ;
}

static void _config_destroy_network(struct usched_config_network *network) {
	return ;
}

static void _config_destroy_users(struct usched_config_users *users) {
	return ;
}

int config_init(struct usched_config *config) {
	int errsv = 0;

	if (_config_init_auth(&config->auth) < 0) {
		errsv = errno;
		log_warn("config_init(): _config_read_auth(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	if (_config_init_core(&config->core) < 0) {
		errsv = errno;
		log_warn("config_init(): _config_read_core(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	if (_config_init_network(&config->network) < 0) {
		errsv = errno;
		log_warn("config_init(): _config_read_network(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	if (_config_init_users(&config->users) < 0) {
		errsv = errno;
		log_warn("config_init(): _config_read_users(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	return 0;
}

void config_destroy(struct usched_config *config) {
	_config_destroy_auth(&config->auth);
	_config_destroy_core(&config->core);
	_config_destroy_network(&config->network);
	_config_destroy_users(&config->users);
}

