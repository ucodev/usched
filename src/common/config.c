/**
 * @file config.c
 * @brief uSched
 *        Configuration interface
 *
 * Date: 01-04-2015
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
#include <stdlib.h>
#include <errno.h>

#include <sys/types.h>
#include <pwd.h>
#include <grp.h>

#include <pall/cll.h>

#include <fsop/path.h>
#include <fsop/dir.h>

#include "config.h"
#include "mm.h"
#include "log.h"
#include "str.h"


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

static int _userinfo_compare(const void *d1, const void *d2) {
	struct usched_config_userinfo *u1 = (struct usched_config_userinfo *) d1;
	struct usched_config_userinfo *u2 = (struct usched_config_userinfo *) d2;

	return strcmp(u1->username, u2->username);
}

static void _userinfo_destroy(void *data) {
	struct usched_config_userinfo *userinfo = data;

	memset(userinfo->username, 0, strlen(userinfo->username));
	mm_free(userinfo->username);
	memset(userinfo->password, 0, strlen(userinfo->password));
	mm_free(userinfo->password);
	memset(userinfo->salt, 0, strlen(userinfo->salt));
	mm_free(userinfo->salt);
	memset(userinfo, 0, sizeof(struct usched_config_userinfo));
	mm_free(userinfo);
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
		log_warn("_list_init_uint_from_file(): pall_cll_init(): %s\n", strerror(errno));
		fclose(fp);
		errno = errsv;
		return -1;
	}

	/* Read file contents */
	while (fgets(line, (int) sizeof(line) - 1, fp)) {
		++ line_count;

		/* Ignore blank lines */
		if (!line[0] || (line[0] == '\n'))
			continue;

		/* Strip '\n' and/or '\r' */
		(void) strrtrim(line, "\n\r");

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
		*val = (unsigned int) strtoul(line, &endptr, 0);

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
	if (!fgets(line, (int) sizeof(line) - 1, fp)) {
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

	/* Strip '\n' and/or '\r' */
	(void) strrtrim(line, "\n\r");

	/* Retrieve line value */
	*val = (unsigned int) strtoul(line, &endptr, 0);

	/* Check if value is acceptable */
	if ((*endptr) || (endptr == line) || (errno == EINVAL) || (errno == ERANGE)) {
		log_warn("_value_init_uint_from_file(): Invalid value found on file %s.\n", file);
		errno = EINVAL;
		return -1;
	}

	/* Success */
	return 0;
}

static int _value_init_long_from_file(const char *file, long *val) {
	int errsv = 0;
	FILE *fp = NULL;
	char *endptr = NULL;
	char line[32];

	/* Reset line buffer memory */
	memset(line, 0, sizeof(line));

	/* Grant that file exists and is a regular file */
	if (!fsop_path_isreg(file)) {
		errsv = errno;
		log_warn("_value_init_long_from_file(): fsop_path_isreg(\"%s\"): %s\n", file, strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Try to open file */
	if (!(fp = fopen(file, "r"))) {
		errsv = errno;
		log_warn("_value_init_long_from_file(): fopen(\"%s\", \"r\"): %s\n", file, strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Retrieve value from file */
	if (!fgets(line, (int) sizeof(line) - 1, fp)) {
		errsv = errno;
		log_warn("_value_init_long_from_file(): fgets(): Unexpected EOF (%s).\n", file);
		fclose(fp);
		errno = errsv;
		return -1;
	}

	/* Close file */
	fclose(fp);

	/* Check if line is empty */
	if (!line[0] || (line[0] == '\n')) {
		errsv = errno;
		log_warn("_value_init_long_from_file(): First line of file %s is empty.\n", file);
		errno = errsv;
		return -1;
	}

	/* Strip '\n' and/or '\r' */
	(void) strrtrim(line, "\n\r");

	/* Retrieve line value */
	*val = strtol(line, &endptr, 0);

	/* Check if value is acceptable */
	if ((*endptr) || (endptr == line) || (errno == EINVAL) || (errno == ERANGE)) {
		log_warn("_value_init_long_from_file(): Invalid value found on file %s.\n", file);
		errno = EINVAL;
		return -1;
	}

	/* Success */
	return 0;
}

static char *_value_init_string_from_file(const char *file) {
	int errsv = 0;
	FILE *fp = NULL;
	char line[8192], *string = NULL;
	size_t len = 0;

	/* Reset line buffer memory */
	memset(line, 0, sizeof(line));

	/* Grant that file exists and is a regular file */
	if (!fsop_path_isreg(file)) {
		errsv = errno;
		log_warn("_value_init_string_from_file(): fsop_path_isreg(\"%s\"): %s\n", file, strerror(errno));
		errno = errsv;
		return NULL;
	}

	/* Try to open file */
	if (!(fp = fopen(file, "r"))) {
		errsv = errno;
		log_warn("_value_init_string_from_file(): fopen(\"%s\", \"r\"): %s\n", file, strerror(errno));
		errno = errsv;
		return NULL;
	}

	/* Retrieve value from file */
	if (!fgets(line, (int) sizeof(line) - 1, fp)) {
		errsv = errno;
		log_warn("_value_init_string_from_file(): fgets(): Unexpected EOF (%s).\n", file);
		fclose(fp);
		errno = errsv;
		return NULL;
	}

	/* Close file */
	fclose(fp);

	/* Check if line is empty */
	if (!line[0] || (line[0] == '\n')) {
		errsv = errno;
		log_warn("_value_init_string_from_file(): First line of file %s is empty.\n", file);
		errno = errsv;
		return NULL;
	}

	/* Strip '\n' and/or '\r' */
	len = strrtrim(line, "\n\r");

	/* Allocate memory for string */
	if (!(string = mm_alloc(len + 1))) {
		errsv = errno;
		log_warn("_value_init_string_from_file(): mm_alloc(): %s\n", strerror(errno));
		errno = errsv;
		return NULL;
	}

	strcpy(string, line);

	/* Success */
	return string;
}

static int _config_init_auth_blacklist_gid(struct usched_config_auth *auth) {
	return _list_init_uint_from_file(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_AUTH "/" CONFIG_USCHED_FILE_AUTH_BL_GID, &auth->blacklist_gid);
}

static int _config_validate_auth_blacklist_gid(const struct usched_config_auth *auth) {
	/* TODO */
	return 1;
}

static int _config_init_auth_whitelist_gid(struct usched_config_auth *auth) {
	return _list_init_uint_from_file(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_AUTH "/" CONFIG_USCHED_FILE_AUTH_WL_GID, &auth->whitelist_gid);
}

static int _config_validate_auth_whitelist_gid(const struct usched_config_auth *auth) {
	/* TODO */
	return 1;
}

static int _config_init_auth_blacklist_uid(struct usched_config_auth *auth) {
	return _list_init_uint_from_file(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_AUTH "/" CONFIG_USCHED_FILE_AUTH_BL_UID, &auth->blacklist_uid);
}

static int _config_validate_auth_blacklist_uid(const struct usched_config_auth *auth) {
	/* TODO */
	return 1;
}

static int _config_init_auth_whitelist_uid(struct usched_config_auth *auth) {
	return _list_init_uint_from_file(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_AUTH "/" CONFIG_USCHED_FILE_AUTH_WL_UID, &auth->whitelist_uid);
}

static int _config_validate_auth_whitelist_uid(const struct usched_config_auth *auth) {
	/* TODO */
	return 1;
}

static int _config_init_auth_local_use(struct usched_config_auth *auth) {
	return _value_init_uint_from_file(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_AUTH "/" CONFIG_USCHED_FILE_AUTH_LOCAL_USE, &auth->local_use);
}

static int _config_validate_auth_local_use(const struct usched_config_auth *auth) {
	return (auth->local_use == 0) || (auth->local_use == 1);
}

static int _config_init_auth_remote_users(struct usched_config_auth *auth) {
	return _value_init_uint_from_file(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_AUTH "/" CONFIG_USCHED_FILE_AUTH_REMOTE_USERS, &auth->remote_users);
}

static int _config_validate_auth_remote_users(const struct usched_config_auth *auth) {
	return (auth->remote_users == 0) || (auth->remote_users == 1);
}

int config_init_auth(struct usched_config_auth *auth) {
	int errsv = 0;

	/* Read GID blacklist */
	if (_config_init_auth_blacklist_gid(auth) < 0) {
		errsv = errno;
		log_warn("_config_init_auth(): _config_init_auth_blacklist_gid(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Validate GID blacklist */
	if (!_config_validate_auth_blacklist_gid(auth)) {
		log_warn("_config_init_auth(): _config_validate_auth_blacklist_gid(): Invalid auth.blacklist.gid value.\n");
		errno = EINVAL;
		return -1;
	}

	/* Read GID whitelist */
	if (_config_init_auth_whitelist_gid(auth) < 0) {
		errsv = errno;
		log_warn("_config_init_auth(): _config_init_auth_whitelist_gid(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Validate GID whitelist */
	if (!_config_validate_auth_whitelist_gid(auth)) {
		log_warn("_config_init_auth(): _config_validate_auth_whitelist_gid(): Invalid auth.whitelist.gid value.\n");
		errno = EINVAL;
		return -1;
	}

	/* Read UID blacklist */
	if (_config_init_auth_blacklist_uid(auth) < 0) {
		errsv = errno;
		log_warn("_config_init_auth(): _config_init_auth_blacklist_uid(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Validate UID blacklist */
	if (!_config_validate_auth_blacklist_uid(auth)) {
		log_warn("_config_init_auth(): _config_validate_auth_blacklist_uid(): Invalid auth.blacklist.uid value.\n");
		errno = EINVAL;
		return -1;
	}

	/* Read UID whitelist */
	if (_config_init_auth_whitelist_uid(auth) < 0) {
		errsv = errno;
		log_warn("_config_init_auth(): _config_init_auth_whitelist_uid(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Validate UID whitelist */
	if (!_config_validate_auth_whitelist_uid(auth)) {
		log_warn("_config_init_auth(): _config_validate_auth_whitelist_uid(): Invalid auth.whitelist.uid value.\n");
		errno = EINVAL;
		return -1;
	}

	/* Read local use */
	if (_config_init_auth_local_use(auth) < 0) {
		errsv = errno;
		log_warn("_config_init_auth(): _config_init_auth_local_use(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Validate local use */
	if (!_config_validate_auth_local_use(auth)) {
		log_warn("_config_init_auth(): _config_validate_auth_local_use(): Invalid auth.local.use value.\n");
		errno = EINVAL;
		return -1;
	}

	/* Read users remote */
	if (_config_init_auth_remote_users(auth) < 0) {
		errsv = errno;
		log_warn("_config_init_auth(): _config_init_auth_remote_users(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Validate remote users */
	if (!_config_validate_auth_remote_users(auth)) {
		log_warn("_config_init_auth(): _config_validate_auth_remote_users(): Invalid auth.remote.users value.\n");
		errno = EINVAL;
		return -1;
	}

	/* Success */
	return 0;
}

static int _config_init_core_delta_noexec(struct usched_config_core *core) {
	return _value_init_uint_from_file(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_CORE "/" CONFIG_USCHED_FILE_CORE_DELTA_NOEXEC, &core->delta_noexec);
}

static int _config_validate_core_delta_noexec(const struct usched_config_core *core) {
	/* TODO */
	return 1;
}

static int _config_init_core_delta_reload(struct usched_config_core *core) {
	return _value_init_uint_from_file(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_CORE "/" CONFIG_USCHED_FILE_CORE_DELTA_RELOAD, &core->delta_reload);
}

static int _config_validate_core_delta_reload(const struct usched_config_core *core) {
	/* TODO */
	return 1;
}

static int _config_init_core_serialize_file(struct usched_config_core *core) {
	if (!(core->serialize_file = _value_init_string_from_file(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_CORE "/" CONFIG_USCHED_FILE_CORE_SERIALIZE_FILE)))
		return -1;

	return 0;
}

static int _config_validate_core_serialize_file(const struct usched_config_core *core) {
	/* TODO */
	return 1;
}

static int _config_init_core_jail_dir(struct usched_config_core *core) {
	if (!(core->jail_dir = _value_init_string_from_file(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_CORE "/" CONFIG_USCHED_FILE_CORE_JAIL_DIR)))
		return -1;

	return 0;
}

static int _config_validate_core_jail_dir(const struct usched_config_core *core) {
	return fsop_path_isdir(core->jail_dir);
}

static int _config_init_core_ipc_msgmax(struct usched_config_core *core) {
	return _value_init_long_from_file(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_CORE "/" CONFIG_USCHED_FILE_CORE_IPC_MSGMAX, &core->ipc_msgmax);
}

static int _config_validate_core_ipc_msgmax(const struct usched_config_core *core) {
	return core->ipc_msgmax > 0;
}

static int _config_init_core_ipc_msgsize(struct usched_config_core *core) {
	return _value_init_long_from_file(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_CORE "/" CONFIG_USCHED_FILE_CORE_IPC_MSGSIZE, &core->ipc_msgsize);
}

static int _config_validate_core_ipc_msgsize(const struct usched_config_core *core) {
	return core->ipc_msgsize > 0;
}

static int _config_init_core_ipc_name(struct usched_config_core *core) {
	if (!(core->ipc_name = _value_init_string_from_file(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_CORE "/" CONFIG_USCHED_FILE_CORE_IPC_NAME)))
		return -1;

	return 0;
}

static int _config_validate_core_ipc_name(const struct usched_config_core *core) {
#if CONFIG_USE_IPC_PMQ == 1
	if (core->ipc_name[0] != '/') {
		errno = EINVAL;
		return -1;
	}
#elif CONFIG_USE_IPC_UNIX == 1
	/* TODO */
#elif CONFIG_USE_IPC_INET == 1
	int port = 0;

	if (!strisnum(core->ipc_name)) {
		errno = EINVAL;
		return -1;
	}

	if ((port < 1) || (port > 65535)) {
		errno = EINVAL;
		return -1;
	}
#endif
	return 1;
}

static int _config_init_core_ipc_key(struct usched_config_core *core) {
	if (!(core->ipc_key = _value_init_string_from_file(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_CORE "/" CONFIG_USCHED_FILE_CORE_IPC_KEY)))
		return -1;

	return 0;
}

static int _config_validate_core_ipc_key(const struct usched_config_core *core) {
	if (strlen(core->ipc_key) < 32)
		return 0;

	if (strlen(core->ipc_key) > CONFIG_USCHED_AUTH_IPC_SIZE)
		return 0;

	return 1;
}

static int _config_init_core_privdrop_user(struct usched_config_core *core) {
	if (!(core->privdrop_user = _value_init_string_from_file(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_CORE "/" CONFIG_USCHED_FILE_CORE_PRIVDROP_USER)))
		return -1;

	return 0;
}

static int _config_validate_core_privdrop_user(const struct usched_config_core *core) {
	/* TODO */
	return 1;
}

static int _config_init_core_privdrop_group(struct usched_config_core *core) {
	if (!(core->privdrop_group = _value_init_string_from_file(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_CORE "/" CONFIG_USCHED_FILE_CORE_PRIVDROP_GROUP)))
		return -1;

	return 0;
}

static int _config_validate_core_privdrop_group(const struct usched_config_core *core) {
	/* TODO */
	return 1;
}

static int _config_init_core_thread_priority(struct usched_config_core *core) {
	return _value_init_long_from_file(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_CORE "/" CONFIG_USCHED_FILE_CORE_THREAD_PRIORITY, &core->thread_priority);
}

static int _config_validate_core_thread_priority(const struct usched_config_core *core) {
	return (core->thread_priority >= 0) && (core->thread_priority <= 32);
}

static int _config_init_core_thread_workers(struct usched_config_core *core) {
	return _value_init_uint_from_file(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_CORE "/" CONFIG_USCHED_FILE_CORE_THREAD_WORKERS, &core->thread_workers);
}

static int _config_validate_core_thread_workers(const struct usched_config_core *core) {
	/* TODO */
	return 1;
}

int config_init_core(struct usched_config_core *core) {
	int errsv = 0;
	struct passwd passwd_buf, *passwd = NULL;
	struct group group_buf, *group = NULL;
	char buf[8192];

	/* Read delta noexec */
	if (_config_init_core_delta_noexec(core) < 0) {
		errsv = errno;
		log_warn("_config_init_core(): _config_init_core_delta_noexec(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Validate delta noexec */
	if (!_config_validate_core_delta_noexec(core)) {
		log_warn("_config_init_core(): _config_validate_core_delta_noexec(): Invalid core.delta.noexec value.\n");
		errno = EINVAL;
		return -1;
	}

	/* Read delta reload */
	if (_config_init_core_delta_reload(core) < 0) {
		errsv = errno;
		log_warn("_config_init_core(): _config_init_core_delta_reload(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Validate delta reload */
	if (!_config_validate_core_delta_reload(core)) {
		log_warn("_config_init_core(): _config_validate_core_delta_reload(): Invalid core.delta.reload value.\n");
		errno = EINVAL;
		return -1;
	}

	/* Read serialize file */
	if (_config_init_core_serialize_file(core) < 0) {
		errsv = errno;
		log_warn("_config_init_core(): _config_init_core_serialize_file(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Validate serialize file */
	if (!_config_validate_core_serialize_file(core)) {
		log_warn("_config_init_core(): _config_validate_core_serialize_file(): Invalid core.serialize.file value.\n");
		errno = EINVAL;
		return -1;
	}

	/* Read the jail directory */
	if (_config_init_core_jail_dir(core) < 0) {
		errsv = errno;
		log_warn("_config_init_core(): _config_init_core_jail_dir(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Validate jail dir */
	if (!_config_validate_core_jail_dir(core)) {
		log_warn("_config_init_core(): _config_validate_core_jail_dir(): Invalid core.jail.dir value.\n");
		errno = EINVAL;
		return -1;
	}

	/* Read ipc msg max */
	if (_config_init_core_ipc_msgmax(core) < 0) {
		errsv = errno;
		log_warn("_config_init_core(): _config_init_core_ipc_msgmax(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Validate ipc msgmax */
	if (!_config_validate_core_ipc_msgmax(core)) {
		log_warn("_config_init_core(): _config_validate_core_ipc_msgmax(): Invalid core.ipc.msgmax value.\n");
		errno = EINVAL;
		return -1;
	}

	/* Read ipc msg size */
	if (_config_init_core_ipc_msgsize(core) < 0) {
		errsv = errno;
		log_warn("_config_init_core(): _config_init_core_ipc_msgsize(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Validate ipc msgsize */
	if (!_config_validate_core_ipc_msgsize(core)) {
		log_warn("_config_init_core(): _config_validate_core_ipc_msgsize(): Invalid core.ipc.msgsize value.\n");
		errno = EINVAL;
		return -1;
	}

	/* Read ipc name */
	if (_config_init_core_ipc_name(core) < 0) {
		errsv = errno;
		log_warn("_config_init_core(): _config_init_core_ipc_name(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Validate ipc name */
	if (!_config_validate_core_ipc_name(core)) {
		log_warn("_config_init_core(): _config_validate_core_ipc_name(): Invalid core.ipc.name value.\n");
		errno = EINVAL;
		return -1;
	}

	/* Read ipc key */
	if (_config_init_core_ipc_key(core) < 0) {
		errsv = errno;
		log_warn("_config_init_core(): _config_init_core_ipc_key(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Validate ipc key */
	if (!_config_validate_core_ipc_key(core)) {
		log_warn("_config_init_core(): _config_validate_core_ipc_key(): Invalid core.ipc.key value.\n");
		errno = EINVAL;
		return -1;
	}

	/* Read privilege drop user */
	if (_config_init_core_privdrop_user(core) < 0) {
		errsv = errno;
		log_warn("_config_init_core(): _config_init_core_privdrop_user(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Retrieve the UID associated to the user */
	memset(buf, 0, sizeof(buf));
	memset(&passwd_buf, 0, sizeof(passwd_buf));

	if (getpwnam_r(core->privdrop_user, &passwd_buf, buf, sizeof(buf), &passwd) || (passwd != &passwd_buf)) {
		errsv = errno;
		log_warn("_config_init_core(): getpwnam_r(\"%s\", ...): %s\n.", core->privdrop_user, strerror(errno));
		errno = errsv;
		return -1;
	}

	core->privdrop_uid = passwd->pw_uid;

	/* Validate privilege drop user */
	if (!_config_validate_core_privdrop_user(core)) {
		log_warn("_config_init_core(): _config_validate_core_privdrop_user(): Invalid core.privdrop.user value.\n");
		errno = EINVAL;
		return -1;
	}

	/* Read privilege drop group */
	if (_config_init_core_privdrop_group(core) < 0) {
		errsv = errno;
		log_warn("_config_init_core(): _config_init_core_privdrop_group(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Retrieve the GID associated to the group */
	memset(buf, 0, sizeof(buf));
	memset(&group_buf, 0, sizeof(group_buf));

	if (getgrnam_r(core->privdrop_group, &group_buf, buf, sizeof(buf), &group) || (group != &group_buf)) {
		errsv = errno;
		log_warn("_config_init_core(): getgrnam_r(\"%s\", ...): %s\n.", core->privdrop_group, strerror(errno));
		errno = errsv;
		return -1;
	}

	core->privdrop_gid = group->gr_gid;

	/* Validate privilege drop group */
	if (!_config_validate_core_privdrop_group(core)) {
		log_warn("_config_init_core(): _config_validate_core_privdrop_group(): Invalid core.privdrop.group value.\n");
		errno = EINVAL;
		return -1;
	}

	/* Read thread priority */
	if (_config_init_core_thread_priority(core) < 0) {
		errsv = errno;
		log_warn("_config_init_core(): _config_init_core_thread_priority(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Validate thread priority */
	if (!_config_validate_core_thread_priority(core)) {
		log_warn("_config_init_core(): _config_validate_core_thread_priority(): Invalid core.thread.priority value.\n");
		errno = EINVAL;
		return -1;
	}

	/* Read thread workers */
	if (_config_init_core_thread_workers(core) < 0) {
		errsv = errno;
		log_warn("_config_init_core(): _config_init_core_thread_workers(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Validate thread workers */
	if (!_config_validate_core_thread_workers(core)) {
		log_warn("_config_init_core(): _config_validate_core_thread_workers(): Invalid core.thread.workers value.\n");
		errno = EINVAL;
		return -1;
	}

	/* Success */
	return 0;
}

static int _config_init_network_bind_addr(struct usched_config_network *network) {
	if (!(network->bind_addr = _value_init_string_from_file(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_NETWORK "/" CONFIG_USCHED_FILE_NETWORK_BIND_ADDR)))
		return -1;

	return 0;
}

static int _config_validate_network_bind_addr(const struct usched_config_network *network) {
	/* TODO */
	return 1;
}

static int _config_init_network_bind_port(struct usched_config_network *network) {
	if (!(network->bind_port = _value_init_string_from_file(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_NETWORK "/" CONFIG_USCHED_FILE_NETWORK_BIND_PORT)))
		return -1;

	return 0;
}

static int _config_validate_network_bind_port(const struct usched_config_network *network) {
	long port = atol(network->bind_port);

	return (port > 1) && (port <= 0xffff);
}

static int _config_init_network_conn_limit(struct usched_config_network *network) {
	return _value_init_uint_from_file(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_NETWORK "/" CONFIG_USCHED_FILE_NETWORK_CONN_LIMIT, &network->conn_limit);
}

static int _config_validate_network_conn_limit(const struct usched_config_network *network) {
	/* TODO */
	return 1;
}

static int _config_init_network_conn_timeout(struct usched_config_network *network) {
	return _value_init_uint_from_file(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_NETWORK "/" CONFIG_USCHED_FILE_NETWORK_CONN_TIMEOUT, &network->conn_timeout);
}

static int _config_validate_network_conn_timeout(const struct usched_config_network *network) {
	/* TODO */
	return 1;
}

static int _config_init_network_sock_name(struct usched_config_network *network) {
	if (!(network->sock_name = _value_init_string_from_file(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_NETWORK "/" CONFIG_USCHED_FILE_NETWORK_SOCK_NAME)))
		return -1;

	return 0;
}

static int _config_validate_network_sock_name(const struct usched_config_network *network) {
	/* TODO */
	return 1;
}

int config_init_network(struct usched_config_network *network) {
	int errsv = 0;

	/* Read bind addr */
	if (_config_init_network_bind_addr(network) < 0) {
		errsv = errno;
		log_warn("_config_init_network(): _config_init_network_bind_addr(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Validate bind addr */
	if (!_config_validate_network_bind_addr(network)) {
		log_warn("_config_init_network(): _config_validate_network_bind_addr(): Invalid network.bind.addr value.\n");
		errno = EINVAL;
		return -1;
	}

	/* Read bind port */
	if (_config_init_network_bind_port(network) < 0) {
		errsv = errno;
		log_warn("_config_init_network(): _config_init_network_bind_port(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Validate bind port */
	if (!_config_validate_network_bind_port(network)) {
		log_warn("_config_init_network(): _config_validate_network_bind_port(): Invalid network.bind.port value.\n");
		errno = EINVAL;
		return -1;
	}

	/* Read conn limit */
	if (_config_init_network_conn_limit(network) < 0) {
		errsv = errno;
		log_warn("_config_init_network(): _config_init_network_conn_limit(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Validate conn limit */
	if (!_config_validate_network_conn_limit(network)) {
		log_warn("_config_init_network(): _config_validate_network_conn_limit(): Invalid network.conn.limit value.\n");
		errno = EINVAL;
		return -1;
	}

	/* Read conn timeout */
	if (_config_init_network_conn_timeout(network) < 0) {
		errsv = errno;
		log_warn("_config_init_network(): _config_init_network_conn_timeout(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Validate conn timeout */
	if (!_config_validate_network_conn_timeout(network)) {
		log_warn("_config_init_network(): _config_validate_network_conn_timeout(): Invalid network.conn.timeout value.\n");
		errno = EINVAL;
		return -1;
	}

	/* Read sock name */
	if (_config_init_network_sock_name(network) < 0) {
		errsv = errno;
		log_warn("_config_init_network(): _config_init_network_sock_name(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Validate sock name */
	if (!_config_validate_network_sock_name(network)) {
		log_warn("_config_init_network(): _config_validate_network_sock_name(): Invalid network.sock.name value.\n");
		errno = EINVAL;
		return -1;
	}

	/* Success */
	return 0;
}

static int _config_init_stat_jail_dir(struct usched_config_stat *stat) {
	if (!(stat->jail_dir = _value_init_string_from_file(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_STAT "/" CONFIG_USCHED_FILE_STAT_JAIL_DIR)))
		return -1;

	return 0;
}

static int _config_validate_stat_jail_dir(const struct usched_config_stat *stat) {
	return fsop_path_isdir(stat->jail_dir);
}

static int _config_init_stat_ipc_msgmax(struct usched_config_stat *stat) {
	return _value_init_long_from_file(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_STAT "/" CONFIG_USCHED_FILE_STAT_IPC_MSGMAX, &stat->ipc_msgmax);
}

static int _config_validate_stat_ipc_msgmax(const struct usched_config_stat *stat) {
	return stat->ipc_msgmax > 0;
}

static int _config_init_stat_ipc_msgsize(struct usched_config_stat *stat) {
	return _value_init_long_from_file(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_STAT "/" CONFIG_USCHED_FILE_STAT_IPC_MSGSIZE, &stat->ipc_msgsize);
}

static int _config_validate_stat_ipc_msgsize(const struct usched_config_stat *stat) {
	return stat->ipc_msgsize > 0;
}

static int _config_init_stat_ipc_name(struct usched_config_stat *stat) {
	if (!(stat->ipc_prefix = _value_init_string_from_file(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_STAT "/" CONFIG_USCHED_FILE_STAT_IPC_NAME)))
		return -1;

	return 0;
}

static int _config_validate_stat_ipc_name(const struct usched_config_stat *stat) {
#if CONFIG_USE_IPC_PMQ == 1
	if (stat->ipc_prefix[0] != '/') {
		errno = EINVAL;
		return -1;
	}
#elif CONFIG_USE_IPC_UNIX == 1
	/* TODO */
#elif CONFIG_USE_IPC_INET == 1
	int port = atoi(stat->ipc_prefix);

	if (!strisnum(stat->ipc_prefix)) {
		errno = EINVAL;
		return -1;
	}

	/* Note that the max value here _IS_ (and must be) 65534, because two ports will be used */
	if ((port < 1) || (port > 65534)) {
		errno = EINVAL;
		return -1;
	}
#endif
	return 1;
}

static int _config_init_stat_ipc_key(struct usched_config_stat *stat) {
	if (!(stat->ipc_key = _value_init_string_from_file(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_STAT "/" CONFIG_USCHED_FILE_STAT_IPC_KEY)))
		return -1;

	return 0;
}

static int _config_validate_stat_ipc_key(const struct usched_config_stat *stat) {
	if (strlen(stat->ipc_key) < 32)
		return 0;

	if (strlen(stat->ipc_key) > CONFIG_USCHED_AUTH_IPC_SIZE)
		return 0;

	return 1;
}

static int _config_init_stat_privdrop_user(struct usched_config_stat *stat) {
	if (!(stat->privdrop_user = _value_init_string_from_file(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_STAT "/" CONFIG_USCHED_FILE_STAT_PRIVDROP_USER)))
		return -1;

	return 0;
}

static int _config_validate_stat_privdrop_user(const struct usched_config_stat *stat) {
	/* TODO */
	return 1;
}

static int _config_init_stat_privdrop_group(struct usched_config_stat *stat) {
	if (!(stat->privdrop_group = _value_init_string_from_file(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_STAT "/" CONFIG_USCHED_FILE_STAT_PRIVDROP_GROUP)))
		return -1;

	return 0;
}

static int _config_validate_stat_privdrop_group(const struct usched_config_stat *stat) {
	/* TODO */
	return 1;
}

int config_init_stat(struct usched_config_stat *stat) {
	int errsv = 0;
	struct passwd passwd_buf, *passwd = NULL;
	struct group group_buf, *group = NULL;
	char buf[8192];

	/* Read the jail directory */
	if (_config_init_stat_jail_dir(stat) < 0) {
		errsv = errno;
		log_warn("_config_init_stat(): _config_init_stat_jail_dir(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Validate jail dir */
	if (!_config_validate_stat_jail_dir(stat)) {
		log_warn("_config_init_stat(): _config_validate_stat_jail_dir(): Invalid stat.jail.dir value.\n");
		errno = EINVAL;
		return -1;
	}

	/* Read ipc msg max */
	if (_config_init_stat_ipc_msgmax(stat) < 0) {
		errsv = errno;
		log_warn("_config_init_stat(): _config_init_stat_ipc_msgmax(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Validate ipc msgmax */
	if (!_config_validate_stat_ipc_msgmax(stat)) {
		log_warn("_config_init_stat(): _config_validate_stat_ipc_msgmax(): Invalid stat.ipc.msgmax value.\n");
		errno = EINVAL;
		return -1;
	}

	/* Read ipc msg size */
	if (_config_init_stat_ipc_msgsize(stat) < 0) {
		errsv = errno;
		log_warn("_config_init_stat(): _config_init_stat_ipc_msgsize(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Validate ipc msgsize */
	if (!_config_validate_stat_ipc_msgsize(stat)) {
		log_warn("_config_init_stat(): _config_validate_stat_ipc_msgsize(): Invalid stat.ipc.msgsize value.\n");
		errno = EINVAL;
		return -1;
	}

	/* Read ipc name (used as prefix, since two IPC queues will be created) */
	if (_config_init_stat_ipc_name(stat) < 0) {
		errsv = errno;
		log_warn("_config_init_stat(): _config_init_stat_ipc_name(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Validate ipc name */
	if (!_config_validate_stat_ipc_name(stat)) {
		log_warn("_config_init_stat(): _config_validate_stat_ipc_name(): Invalid stat.ipc.name value.\n");
		errno = EINVAL;
		return -1;
	}

	/* Setup the ipc name for reading (will be written by usd and use) */
	if (!(stat->ipc_name_uss_ro = mm_alloc(strlen(stat->ipc_prefix) + sizeof("_read")))) {
		errsv = errno;
		log_warn("_config_init_stat(): mm_alloc(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

#if CONFIG_USE_IPC_PMQ == 1 || CONFIG_USE_IPC_UNIX == 1
	strcpy(stat->ipc_name_uss_ro, stat->ipc_prefix);
	strcat(stat->ipc_name_uss_ro, "_read");
#elif CONFIG_USE_IPC_INET == 1
	sprintf(stat->ipc_name_uss, "%d", atoi(stat->ipc_prefix));
#endif

	/* Setup the ipc name for writing (will be read by usd) */
	if (!(stat->ipc_name_usd_wo = mm_alloc(strlen(stat->ipc_prefix) + sizeof("_write")))) {
		errsv = errno;
		log_warn("_config_init_stat(): mm_alloc(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

#if CONFIG_USE_IPC_PMQ == 1 || CONFIG_USE_IPC_UNIX == 1
	strcpy(stat->ipc_name_usd_wo, stat->ipc_prefix);
	strcat(stat->ipc_name_usd_wo, "_write");
#elif CONFIG_USE_IPC_INET == 1
	/* Add 1 to the port assigned to the read end */
	sprintf(stat->ipc_name_usd_wo, "%d", atoi(stat->ipc_prefix) + 1);
#endif

	/* Read ipc key */
	if (_config_init_stat_ipc_key(stat) < 0) {
		errsv = errno;
		log_warn("_config_init_stat(): _config_init_stat_ipc_key(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Validate ipc key */
	if (!_config_validate_stat_ipc_key(stat)) {
		log_warn("_config_init_stat(): _config_validate_stat_ipc_key(): Invalid stat.ipc.key value.\n");
		errno = EINVAL;
		return -1;
	}

	/* Read privilege drop user */
	if (_config_init_stat_privdrop_user(stat) < 0) {
		errsv = errno;
		log_warn("_config_init_stat(): _config_init_stat_privdrop_user(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Retrieve the UID associated to the user */
	memset(buf, 0, sizeof(buf));
	memset(&passwd_buf, 0, sizeof(passwd_buf));

	if (getpwnam_r(stat->privdrop_user, &passwd_buf, buf, sizeof(buf), &passwd) || (passwd != &passwd_buf)) {
		errsv = errno;
		log_warn("_config_init_stat(): getpwnam_r(\"%s\", ...): %s\n.", stat->privdrop_user, strerror(errno));
		errno = errsv;
		return -1;
	}

	stat->privdrop_uid = passwd->pw_uid;

	/* Validate privilege drop user */
	if (!_config_validate_stat_privdrop_user(stat)) {
		log_warn("_config_init_stat(): _config_validate_stat_privdrop_user(): Invalid stat.privdrop.user value.\n");
		errno = EINVAL;
		return -1;
	}

	/* Read privilege drop group */
	if (_config_init_stat_privdrop_group(stat) < 0) {
		errsv = errno;
		log_warn("_config_init_stat(): _config_init_stat_privdrop_group(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Retrieve the GID associated to the group */
	memset(buf, 0, sizeof(buf));
	memset(&group_buf, 0, sizeof(group_buf));

	if (getgrnam_r(stat->privdrop_group, &group_buf, buf, sizeof(buf), &group) || (group != &group_buf)) {
		errsv = errno;
		log_warn("_config_init_stat(): getgrnam_r(\"%s\", ...): %s\n.", stat->privdrop_group, strerror(errno));
		errno = errsv;
		return -1;
	}

	stat->privdrop_gid = group->gr_gid;

	/* Validate privilege drop group */
	if (!_config_validate_stat_privdrop_group(stat)) {
		log_warn("_config_init_stat(): _config_validate_stat_privdrop_group(): Invalid stat.privdrop.group value.\n");
		errno = EINVAL;
		return -1;
	}

	/* Success */
	return 0;
}

static int _config_init_users_list_add_from_file(
		struct usched_config_users *users,
		const char *file,
		const char *user) 
{
	int errsv = 0;
	char *userinfo_raw = NULL, *ptr = NULL, *endptr = NULL, *pw_ptr = NULL;
	struct usched_config_userinfo *userinfo = NULL;
	size_t userinfo_raw_len = 0;

	/* Read file contents (one line only ) */
	if (!(userinfo_raw = _value_init_string_from_file(file))) {
		errsv = errno;
		log_warn("_config_init_users_list_add_from_file(<struct usched_config_users *>, \"%s\", \"%s\"): _value_init_string_from_file(\"%s\", ...): %s\n", file, user, file, strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Set userinfo raw data length */
	userinfo_raw_len = strlen(userinfo_raw);

	/* Initialize userinfo structure */
	if (!(userinfo = mm_alloc(sizeof(struct usched_config_userinfo)))) {
		errsv = errno;
		log_warn("_config_init_users_list_add_from_file(<struct usched_config_users *>, \"%s\", \"%s\"): mm_alloc(): %s\n", strerror(errno));
		memset(userinfo_raw, 0, userinfo_raw_len);
		mm_free(userinfo_raw);
		errno = errsv;
		return -1;
	}

	/* Reset userinfo memory */
	memset(userinfo, 0, sizeof(struct usched_config_userinfo));

	/* Parse first field [UID] */
	if (!(ptr = strtok(userinfo_raw, ":"))) {
		log_warn("_config_init_users_list_add_from_file(<struct usched_config_users *>, \"%s\", \"%s\"): Parse error: Unexpected value: %s.\n", file, user, userinfo_raw);
		memset(userinfo_raw, 0, userinfo_raw_len);
		mm_free(userinfo_raw);
		mm_free(userinfo);
		errno = EINVAL;
		return -1;
	}

	/* Get UID */
	userinfo->uid = strtoul(ptr, &endptr, 0);

	if ((*endptr) || (endptr == ptr) || (errno == ERANGE) || (errno == EINVAL)) {
		log_warn("_config_init_users_list_add_from_file(<struct usched_config_users *>, \"%s\", \"%s\"): Parse error: Invalid UID value: %s.\n", file, user, ptr);
		memset(userinfo_raw, 0, userinfo_raw_len);
		mm_free(userinfo_raw);
		memset(userinfo, 0, sizeof(struct usched_config_userinfo));
		mm_free(userinfo);
		errno = EINVAL;
		return -1;
	}

	/* Parse second field [GID] */
	if (!(ptr = strtok(NULL, ":"))) {
		log_warn("_config_init_users_list_add_from_file(<struct usched_config_users *>, \"%s\", \"%s\"): Parse error: No ':' delimiter found (2 expected).\n", file, user);
		memset(userinfo_raw, 0, userinfo_raw_len);
		mm_free(userinfo_raw);
		memset(userinfo, 0, sizeof(struct usched_config_userinfo));
		mm_free(userinfo);
		errno = EINVAL;
		return -1;
	}

	/* Get GID */
	userinfo->gid = strtoul(ptr, &endptr, 0);

	if ((*endptr) || (endptr == ptr) || (errno == ERANGE) || (errno == EINVAL)) {
		log_warn("_config_init_users_list_add_from_file(<struct usched_config_users *>, \"%s\", \"%s\"): Parse error: Invalid GID value: %s.\n", file, user, ptr);
		memset(userinfo_raw, 0, userinfo_raw_len);
		mm_free(userinfo_raw);
		memset(userinfo, 0, sizeof(struct usched_config_userinfo));
		mm_free(userinfo);
		errno = EINVAL;
		return -1;
	}

	/* Parse third field [password] */
	if (!(ptr = strtok(NULL, ":"))) {
		log_warn("_config_init_users_list_add_from_file(<struct usched_config_users *>, \"%s\", \"%s\"): Parse error: Only one ':' delimiter found (2 expected).\n", file, user);
		memset(userinfo_raw, 0, userinfo_raw_len);
		mm_free(userinfo_raw);
		memset(userinfo, 0, sizeof(struct usched_config_userinfo));
		mm_free(userinfo);
		errno = EINVAL;
		return -1;
	}

	/* Check if password is empty */
	if (!*ptr) {
		log_warn("_config_init_users_list_add_from_file(<struct usched_config_users *>, \"%s\", \"%s\"): Parse error: Password field is empty.\n", file, user);
		memset(userinfo_raw, 0, userinfo_raw_len);
		mm_free(userinfo_raw);
		memset(userinfo, 0, sizeof(struct usched_config_userinfo));
		mm_free(userinfo);
		errno = EINVAL;
		return -1;
	}

	/* Split salt from password */
	if (!(pw_ptr = strchr(ptr, '$'))) {
		log_warn("_config_init_users_list_add_from_file(<struct usched_config_users *>, \"%s\", \"%s\"): Parse error: Invalid syntax for password field: No salt found.\n", file, user);
		memset(userinfo_raw, 0, userinfo_raw_len);
		mm_free(userinfo_raw);
		memset(userinfo, 0, sizeof(struct usched_config_userinfo));
		mm_free(userinfo);
		errno = EINVAL;
		return -1;
	}

	/* Check if password field isn't empty */
	if (!*(++ pw_ptr)) {
		log_warn("_config_init_users_list_add_from_file(<struct usched_config_users *>, \"%s\", \"%s\"): Parse error: Invalid syntax for password field: Password is empty.\n", file, user);
		memset(userinfo_raw, 0, userinfo_raw_len);
		mm_free(userinfo_raw);
		memset(userinfo, 0, sizeof(struct usched_config_userinfo));
		mm_free(userinfo);
		errno = EINVAL;
		return -1;
	}

	/* Allocate password memory */
	if (!(userinfo->password = mm_alloc(strlen(pw_ptr) + 1))) {
		errsv = errno;
		log_warn("_config_init_users_list_add_from_file(<struct usched_config_users *>, \"%s\", \"%s\"): mm_alloc(): %s\n", file, user, strerror(errno));
		memset(userinfo_raw, 0, userinfo_raw_len);
		mm_free(userinfo_raw);
		memset(userinfo, 0, sizeof(struct usched_config_userinfo));
		mm_free(userinfo);
		errno = errsv;
		return -1;
	}

	/* Copy password */
	strcpy(userinfo->password, pw_ptr);

	/* Set salt/password division */
	*(pw_ptr - 1) = 0;

	/* Allocate salt memory */
	if (!(userinfo->salt = mm_alloc(strlen(ptr) + 1))) {
		errsv = errno;
		log_warn("_config_init_users_list_add_from_file(<struct usched_config_users *>, \"%s\", \"%s\"): mm_alloc(): %s\n", file, user, strerror(errno));
		memset(userinfo_raw, 0, userinfo_raw_len);
		mm_free(userinfo_raw);
		memset(userinfo, 0, sizeof(struct usched_config_userinfo));
		mm_free(userinfo);
		errno = errsv;
		return -1;
	}

	/* Copy salt */
	strcpy(userinfo->salt, ptr);

	/* Free userinfo raw string */
	memset(userinfo_raw, 0, userinfo_raw_len);
	mm_free(userinfo_raw);

	/* Allocate username memory */
	if (!(userinfo->username = mm_alloc(strlen(user) + 1))) {
		errsv = errno;
		log_warn("_config_init_users_list_add_from_file(<struct usched_config_users *>, \"%s\", \"%s\"): mm_alloc(): %s\n", file, user, strerror(errno));
		memset(userinfo->password, 0, strlen(userinfo->password));
		mm_free(userinfo->password);
		memset(userinfo, 0, sizeof(struct usched_config_userinfo));
		mm_free(userinfo);
		errno = errsv;
		return -1;
	}

	/* Copy username */
	strcpy(userinfo->username, user);

	/* Insert userinfo structure into users list */
	if (users->list->insert(users->list, userinfo) < 0) {
		errsv = errno;
		log_warn("_config_init_users_list_add_from_file(<struct usched_config_users *>, \"%s\", \"%s\"): users->list->insert(): %s\n", file, user, strerror(errno));
		memset(userinfo->username, 0, strlen(userinfo->username));
		mm_free(userinfo->username);
		memset(userinfo->password, 0, strlen(userinfo->password));
		mm_free(userinfo->password);
		memset(userinfo, 0, sizeof(struct usched_config_userinfo));
		mm_free(userinfo);
		errno = errsv;
		return -1;
	}

	/* Success */
	return 0;
}

static int _config_init_users_action(int order, const char *fpath, const char *rpath, void *arg) {
	struct usched_config_users *users = arg;

	if (order != FSOP_WALK_INORDER)
		return 0;

	/* Ignore files starting with '.', as they represent temporary values and have no
	 * meaninful username
	 */
	if (rpath[0] == '.')
		return 0;

	if (!fsop_path_isreg(fpath))
		return 0;

	if (_config_init_users_list_add_from_file(users, fpath, rpath) < 0) {
		log_warn("_config_init_users_action(): _config_users_list_add_from_file(): %s\n", strerror(errno));
	}

	return 0;
}

static int _config_validate_users(const struct usched_config_users *users) {
	/* TODO */
	return 1;
}

int config_init_users(struct usched_config_users *users) {
	int errsv = 0;

	/* Initialize users list */
	if (!(users->list = pall_cll_init(&_userinfo_compare, &_userinfo_destroy, NULL, NULL))) {
		errsv = errno;
		log_warn("config_init_users(): pall_cll_init(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Read all users from the files in the users configuration directory */
	if (fsop_walkdir(CONFIG_USCHED_DIR_BASE "/" CONFIG_USCHED_DIR_USERS, NULL, &_config_init_users_action, users) < 0) {
		errsv = errno;
		log_warn("config_init_users(): fsop_walkdir(): %s\n", strerror(errno));
		pall_cll_destroy(users->list);
		errno = errsv;
		return -1;
	}

	/* Validate users */
	if (!_config_validate_users(users)) {
		errsv = errno;
		log_warn("config_init_users(): _config_validate_users(): %s\n", strerror(errno));
		pall_cll_destroy(users->list);
		errno = errsv;
		return -1;
	}

	/* Success */
	return 0;
}

void config_destroy_auth(struct usched_config_auth *auth) {
	pall_cll_destroy(auth->blacklist_gid);
	pall_cll_destroy(auth->whitelist_gid);
	pall_cll_destroy(auth->blacklist_uid);
	pall_cll_destroy(auth->whitelist_uid);

	memset(auth, 0, sizeof(struct usched_config_auth));
}

void config_destroy_core(struct usched_config_core *core) {
	memset(core->serialize_file, 0, strlen(core->serialize_file));
	mm_free(core->serialize_file);
	memset(core->ipc_name, 0, strlen(core->ipc_name));
	mm_free(core->ipc_name);
	memset(core->jail_dir, 0, strlen(core->jail_dir));
	mm_free(core->jail_dir);
	memset(core->privdrop_user, 0, strlen(core->privdrop_user));
	mm_free(core->privdrop_user);
	memset(core->privdrop_group, 0, strlen(core->privdrop_group));
	mm_free(core->privdrop_group);

	memset(core, 0, sizeof(struct usched_config_core));
}

void config_destroy_network(struct usched_config_network *network) {
	memset(network->bind_addr, 0, strlen(network->bind_addr));
	mm_free(network->bind_addr);
	memset(network->bind_port, 0, strlen(network->bind_port));
	mm_free(network->bind_port);
	memset(network->sock_name, 0, strlen(network->sock_name));
	mm_free(network->sock_name);

	memset(network, 0, sizeof(struct usched_config_network));
}

void config_destroy_stat(struct usched_config_stat *stat) {
	/* TODO */
	return ;
}

void config_destroy_users(struct usched_config_users *users) {
	pall_cll_destroy(users->list);

	memset(users, 0, sizeof(struct usched_config_users));
}

