/**
 * @file category.c
 * @brief uSched
 *        Category processing interface
 *
 * Date: 19-03-2015
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
#include <strings.h>
#include <stdlib.h>
#include <errno.h>

#include <sys/types.h>

#include "config.h"
#include "all.h"
#include "auth.h"
#include "core.h"
#include "network.h"
#include "log.h"
#include "users.h"
#include "usage.h"
#include "print.h"
#include "input.h"

int category_all_show(size_t argc, char **args) {
	int errsv = 0;

	/* Usage: add auth <component> <property> <value> */
	if (argc) {
		usage_admin_error_set(USCHED_USAGE_ADMIN_ERR_INSUFF_ARGS, "show all");
		log_warn("category_all_show(): Too many arguments.\n");
		errno = EINVAL;
		return -1;
	}

	/* Show all configurations */
	if (all_admin_show() < 0) {
		errsv = errno;
		log_warn("category_all_show(): all_admin_show(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	return 0;
}

int category_all_commit(size_t argc, char **args) {
	int errsv = 0;

	/* Usage: commit auth */
	if (argc) {
		usage_admin_error_set(USCHED_USAGE_ADMIN_ERR_INSUFF_ARGS, "commit all");
		log_warn("category_all_commit(): Too many arguments.\n");
		errno = EINVAL;
		return -1;
	}

	/* Commit all changes */
	if (all_admin_commit() < 0) {
		errsv = errno;
		log_warn("category_all_commit(): all_admin_commit(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* All good */
	return 0;
}

int category_all_rollback(size_t argc, char **args) {
	int errsv = 0;

	/* Usage: rollback auth */
	if (argc) {
		usage_admin_error_set(USCHED_USAGE_ADMIN_ERR_INSUFF_ARGS, "rollback all");
		log_warn("category_all_rollback(): Too many arguments.\n");
		errno = EINVAL;
		return -1;
	}

	/* Rollback all changes */
	if (all_admin_rollback() < 0) {
		errsv = errno;
		log_warn("category_all_rollback(): all_admin_rollback(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* All good */
	return 0;
}

int category_auth_commit(size_t argc, char **args) {
	int errsv = 0;

	/* Usage: commit auth */
	if (argc) {
		usage_admin_error_set(USCHED_USAGE_ADMIN_ERR_INSUFF_ARGS, "commit auth");
		log_warn("category_auth_commit(): Too many arguments.\n");
		errno = EINVAL;
		return -1;
	}

	if (auth_admin_commit() < 0) {
		errsv = errno;
		log_warn("category_auth_commit(): auth_admin_commit(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	return 0;
}

int category_auth_rollback(size_t argc, char **args) {
	int errsv = 0;

	/* Usage: commit auth */
	if (argc) {
		usage_admin_error_set(USCHED_USAGE_ADMIN_ERR_INSUFF_ARGS, "rollback auth");
		log_warn("category_auth_rollback(): Too many arguments.\n");
		errno = EINVAL;
		return -1;
	}

	if (auth_admin_rollback() < 0) {
		errsv = errno;
		log_warn("category_auth_rollback(): auth_admin_rollback(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	return 0;
}

int category_auth_add(size_t argc, char **args) {
	int errsv = 0;

	/* Usage: add auth <component> <property> <value> */
	if (argc < 3) {
		usage_admin_error_set(USCHED_USAGE_ADMIN_ERR_INSUFF_ARGS, "add auth");
		log_warn("category_auth_add(): Insufficient arguments.\n");
		errno = EINVAL;
		return -1;
	}

	if (!strcasecmp(args[0], USCHED_COMPONENT_BLACKLIST_STR)) {
		if (!strcasecmp(args[1], USCHED_PROPERTY_GID_STR)) {
			/* add blacklist.gid */
			if (auth_admin_blacklist_gid_add(args[2]) < 0) {
				errsv = errno;
				log_warn("category_auth_add(): auth_admin_blacklist_gid_add(): %s\n", strerror(errno));
				errno = errsv;
				return -1;
			}

			/* All good */
			return 0;
		} else if (!strcasecmp(args[1], USCHED_PROPERTY_UID_STR)) {
			/* add blacklist.gid */
			if (auth_admin_blacklist_uid_add(args[2]) < 0) {
				errsv = errno;
				log_warn("category_auth_add(): auth_admin_blacklist_uid_add(): %s\n", strerror(errno));
				errno = errsv;
				return -1;
			}

			/* All good */
			return 0;
		}

		/* Unknown property */
		usage_admin_error_set(USCHED_USAGE_ADMIN_ERR_INVALID_PROPERTY, "add auth blacklist");
		log_warn("category_auth_add(): Invalid 'blacklist' property: %s\n", args[1]);
		errno = EINVAL;

		return -1;
	} else if (!strcasecmp(args[0], USCHED_COMPONENT_WHITELIST_STR)) {
		if (!strcasecmp(args[1], USCHED_PROPERTY_GID_STR)) {
			/* add blacklist.gid */
			if (auth_admin_whitelist_gid_add(args[2]) < 0) {
				errsv = errno;
				log_warn("category_auth_add(): auth_admin_whitelist_gid_add(): %s\n", strerror(errno));
				errno = errsv;
				return -1;
			}

			/* All good */
			return 0;
		} else if (!strcasecmp(args[1], USCHED_PROPERTY_UID_STR)) {
			/* add blacklist.gid */
			if (auth_admin_blacklist_uid_add(args[2]) < 0) {
				errsv = errno;
				log_warn("category_auth_add(): auth_admin_whitelist_uid_add(): %s\n", strerror(errno));
				errno = errsv;
				return -1;
			}

			/* All good */
			return 0;
		}

		/* Unknown property */
		usage_admin_error_set(USCHED_USAGE_ADMIN_ERR_INVALID_PROPERTY, "add auth whitelist");
		log_warn("category_auth_add(): Invalid 'whitelist' property: %s\n", args[1]);
		errno = EINVAL;

		return -1;
	}

	/* Unknown component */
	usage_admin_error_set(USCHED_USAGE_ADMIN_ERR_INVALID_COMPONENT, "add auth");
	log_warn("category_auth_add(): Invalid 'add auth' component: %s\n", args[0]);
	errno = EINVAL;

	return -1;
}

int category_auth_change(size_t argc, char **args) {
	int errsv = 0;

	/* Usage: change auth <component> <property> <value> */
	if (argc < 3) {
		usage_admin_error_set(USCHED_USAGE_ADMIN_ERR_INSUFF_ARGS, "change auth");
		log_warn("category_auth_change(): Insufficient arguments.\n");
		errno = EINVAL;
		return -1;
	}

	if (!strcasecmp(args[0], USCHED_COMPONENT_BLACKLIST_STR)) {
		if (!strcasecmp(args[1], USCHED_PROPERTY_GID_STR)) {
			/* set local.use */
			if (auth_admin_blacklist_gid_change(args[2]) < 0) {
				errsv = errno;
				log_warn("category_auth_change(): auth_admin_blacklist_gid_change(): %s\n", strerror(errno));
				errno = errsv;
				return -1;
			}

			/* All good */
			return 0;
		} else if (!strcasecmp(args[1], USCHED_PROPERTY_UID_STR)) {
			/* set local.use */
			if (auth_admin_blacklist_uid_change(args[2]) < 0) {
				errsv = errno;
				log_warn("category_auth_change(): auth_admin_blacklist_uid_change(): %s\n", strerror(errno));
				errno = errsv;
				return -1;
			}

			/* All good */
			return 0;
		}

		/* Unknown property */
		usage_admin_error_set(USCHED_USAGE_ADMIN_ERR_INVALID_PROPERTY, "change auth blacklist");
		log_warn("category_auth_change(): Invalid 'blacklist' property: %s\n", args[1]);
		errno = EINVAL;

		return -1;
	} else if (!strcasecmp(args[0], USCHED_COMPONENT_LOCAL_STR)) {
		if (!strcasecmp(args[1], USCHED_PROPERTY_USE_STR)) {
			/* set local.use */
			if (auth_admin_local_use_change(args[2]) < 0) {
				errsv = errno;
				log_warn("category_auth_change(): auth_admin_local_use_change(): %s\n", strerror(errno));
				errno = errsv;
				return -1;
			}

			/* All good */
			return 0;
		}

		/* Unknown property */
		usage_admin_error_set(USCHED_USAGE_ADMIN_ERR_INVALID_PROPERTY, "change auth local");
		log_warn("category_auth_change(): Invalid 'local' property: %s\n", args[1]);
		errno = EINVAL;

		return -1;
	} else if (!strcasecmp(args[0], USCHED_COMPONENT_REMOTE_STR)) {
		if (!strcasecmp(args[1], USCHED_PROPERTY_USERS_STR)) {
			/* set remote.users */
			if (auth_admin_remote_users_change(args[2]) < 0) {
				errsv = errno;
				log_warn("category_auth_change(): auth_admin_remote_users_change(): %s\n", strerror(errno));
				errno = errsv;
				return -1;
			}

			/* All good */
			return 0;
		}

		/* Unknown property */
		usage_admin_error_set(USCHED_USAGE_ADMIN_ERR_INVALID_PROPERTY, "change auth remote");
		log_warn("category_auth_change(): Invalid 'remote' property: %s\n", args[1]);
		errno = EINVAL;

		return -1;
	} else if (!strcasecmp(args[0], USCHED_COMPONENT_WHITELIST_STR)) {
		if (!strcasecmp(args[1], USCHED_PROPERTY_GID_STR)) {
			/* set local.use */
			if (auth_admin_whitelist_gid_change(args[2]) < 0) {
				errsv = errno;
				log_warn("category_auth_change(): auth_admin_whitelist_gid_change(): %s\n", strerror(errno));
				errno = errsv;
				return -1;
			}

			/* All good */
			return 0;
		} else if (!strcasecmp(args[1], USCHED_PROPERTY_UID_STR)) {
			/* set local.use */
			if (auth_admin_whitelist_uid_change(args[2]) < 0) {
				errsv = errno;
				log_warn("category_auth_change(): auth_admin_whitelist_uid_change(): %s\n", strerror(errno));
				errno = errsv;
				return -1;
			}

			/* All good */
			return 0;
		}

		/* Unknown property */
		usage_admin_error_set(USCHED_USAGE_ADMIN_ERR_INVALID_PROPERTY, "change auth whitelist");
		log_warn("category_auth_change(): Invalid 'whitelist' property: %s\n", args[1]);
		errno = EINVAL;

		return -1;
	}

	/* Unknown component */
	usage_admin_error_set(USCHED_USAGE_ADMIN_ERR_INVALID_COMPONENT, "change auth");
	log_warn("category_auth_change(): Invalid 'change auth' component: %s\n", args[0]);
	errno = EINVAL;

	return -1;
}

int category_auth_delete(size_t argc, char **args) {
	int errsv = 0;

	/* Usage: add auth <component> <property> <value> */
	if (argc < 3) {
		usage_admin_error_set(USCHED_USAGE_ADMIN_ERR_INSUFF_ARGS, "delete auth");
		log_warn("category_auth_delete(): Insufficient arguments.\n");
		errno = EINVAL;
		return -1;
	}

	if (!strcasecmp(args[0], USCHED_COMPONENT_BLACKLIST_STR)) {
		if (!strcasecmp(args[1], USCHED_PROPERTY_GID_STR)) {
			/* add blacklist.gid */
			if (auth_admin_blacklist_gid_delete(args[2]) < 0) {
				errsv = errno;
				log_warn("category_auth_delete(): auth_admin_blacklist_gid_delete(): %s\n", strerror(errno));
				errno = errsv;
				return -1;
			}

			/* All good */
			return 0;
		} else if (!strcasecmp(args[1], USCHED_PROPERTY_UID_STR)) {
			/* add blacklist.gid */
			if (auth_admin_blacklist_uid_delete(args[2]) < 0) {
				errsv = errno;
				log_warn("category_auth_delete(): auth_admin_blacklist_uid_delete(): %s\n", strerror(errno));
				errno = errsv;
				return -1;
			}

			/* All good */
			return 0;
		}

		/* Unknown property */
		usage_admin_error_set(USCHED_USAGE_ADMIN_ERR_INVALID_PROPERTY, "delete auth blacklist");
		log_warn("category_auth_delete(): Invalid 'blacklist' property: %s\n", args[1]);
		errno = EINVAL;

		return -1;
	} else if (!strcasecmp(args[0], USCHED_COMPONENT_WHITELIST_STR)) {
		if (!strcasecmp(args[1], USCHED_PROPERTY_GID_STR)) {
			/* add blacklist.gid */
			if (auth_admin_whitelist_gid_delete(args[2]) < 0) {
				errsv = errno;
				log_warn("category_auth_delete(): auth_admin_whitelist_gid_delete(): %s\n", strerror(errno));
				errno = errsv;
				return -1;
			}

			/* All good */
			return 0;
		} else if (!strcasecmp(args[1], USCHED_PROPERTY_UID_STR)) {
			/* add blacklist.gid */
			if (auth_admin_blacklist_uid_delete(args[2]) < 0) {
				errsv = errno;
				log_warn("category_auth_delete(): auth_admin_whitelist_uid_delete(): %s\n", strerror(errno));
				errno = errsv;
				return -1;
			}

			/* All good */
			return 0;
		}

		/* Unknown property */
		usage_admin_error_set(USCHED_USAGE_ADMIN_ERR_INVALID_PROPERTY, "delete auth whitelist");
		log_warn("category_auth_delete(): Invalid 'whitelist' property: %s\n", args[1]);
		errno = EINVAL;

		return -1;
	}

	/* Unknown component */
	usage_admin_error_set(USCHED_USAGE_ADMIN_ERR_INVALID_COMPONENT, "delete auth");
	log_warn("category_auth_delete(): Invalid 'delete auth' component: %s\n", args[0]);
	errno = EINVAL;

	return -1;
}


int category_auth_show(size_t argc, char **args) {
	int errsv = 0;

	/* Usage: show auth <component> <property> */
	if (!argc) {
		auth_admin_show();
		return 0;
	}

	if (argc < 2) {
		usage_admin_error_set(USCHED_USAGE_ADMIN_ERR_INSUFF_ARGS, "show auth");
		log_warn("category_auth_show(): Insufficient arguments.\n");
		errno = EINVAL;
		return -1;
	}

	if (!strcasecmp(args[0], USCHED_COMPONENT_BLACKLIST_STR)) {
		if (!strcasecmp(args[1], USCHED_PROPERTY_GID_STR)) {
			/* show blacklist.gid */
			if (auth_admin_blacklist_gid_show() < 0) {
				errsv = errno;
				log_warn("category_auth_show(): auth_admin_blacklist_gid_show(): %s\n", strerror(errno));
				errno = errsv;
				return -1;
			}

			/* All good */
			return 0;
		} else if (!strcasecmp(args[1], USCHED_PROPERTY_UID_STR)) {
			/* show blacklist.gid */
			if (auth_admin_blacklist_uid_show() < 0) {
				errsv = errno;
				log_warn("category_auth_show(): auth_admin_blacklist_uid_show(): %s\n", strerror(errno));
				errno = errsv;
				return -1;
			}

			/* All good */
			return 0;
		}

		/* Unknown property */
		usage_admin_error_set(USCHED_USAGE_ADMIN_ERR_INVALID_PROPERTY, "show auth blacklist");
		log_warn("category_auth_show(): Invalid 'blacklist' property: %s\n", args[1]);
		errno = EINVAL;

		return -1;
	} else if (!strcasecmp(args[0], USCHED_COMPONENT_LOCAL_STR)) {
		if (!strcasecmp(args[1], USCHED_PROPERTY_USE_STR)) {
			/* show local.use */
			if (auth_admin_local_use_show() < 0) {
				errsv = errno;
				log_warn("category_auth_show(): auth_admin_local_use_show(): %s\n", strerror(errno));
				errno = errsv;
				return -1;
			}

			/* All good */
			return 0;
		}

		/* Unknown property */
		usage_admin_error_set(USCHED_USAGE_ADMIN_ERR_INVALID_PROPERTY, "show auth local");
		log_warn("category_auth_show(): Invalid 'local' property: %s\n", args[1]);
		errno = EINVAL;

		return -1;
	} else if (!strcasecmp(args[0], USCHED_COMPONENT_REMOTE_STR)) {
		if (!strcasecmp(args[1], USCHED_PROPERTY_USERS_STR)) {
			/* show remote.users */
			if (auth_admin_remote_users_show() < 0) {
				errsv = errno;
				log_warn("category_auth_show(): auth_admin_remote_users_show(): %s\n", strerror(errno));
				errno = errsv;
				return -1;
			}

			/* All good */
			return 0;
		}

		/* Unknown property */
		usage_admin_error_set(USCHED_USAGE_ADMIN_ERR_INVALID_PROPERTY, "show auth remote");
		log_warn("category_auth_show(): Invalid 'remote' property: %s\n", args[1]);
		errno = EINVAL;

		return -1;
	} else if (!strcasecmp(args[0], USCHED_COMPONENT_WHITELIST_STR)) {
		if (!strcasecmp(args[1], USCHED_PROPERTY_GID_STR)) {
			/* show blacklist.gid */
			if (auth_admin_blacklist_gid_show() < 0) {
				errsv = errno;
				log_warn("category_auth_show(): auth_admin_whitelist_gid_show(): %s\n", strerror(errno));
				errno = errsv;
				return -1;
			}

			/* All good */
			return 0;
		} else if (!strcasecmp(args[1], USCHED_PROPERTY_UID_STR)) {
			/* show blacklist.gid */
			if (auth_admin_whitelist_uid_show() < 0) {
				errsv = errno;
				log_warn("category_auth_show(): auth_admin_whitelist_uid_show(): %s\n", strerror(errno));
				errno = errsv;
				return -1;
			}

			/* All good */
			return 0;
		}

		/* Unknown property */
		usage_admin_error_set(USCHED_USAGE_ADMIN_ERR_INVALID_PROPERTY, "show auth whitelist");
		log_warn("category_auth_show(): Invalid 'whitelist' property: %s\n", args[1]);
		errno = EINVAL;

		return -1;
	}

	/* Unknown component */
	usage_admin_error_set(USCHED_USAGE_ADMIN_ERR_INVALID_COMPONENT, "show auth");
	log_warn("category_auth_show(): Invalid 'change auth' component: %s\n", args[0]);
	errno = EINVAL;

	return -1;
}

int category_core_commit(size_t argc, char **args) {
	int errsv = 0;

	/* Usage: commit core */
	if (argc) {
		usage_admin_error_set(USCHED_USAGE_ADMIN_ERR_INSUFF_ARGS, "commit core");
		log_warn("category_core_commit(): Too many arguments.\n");
		errno = EINVAL;
		return -1;
	}

	if (core_admin_commit() < 0) {
		errsv = errno;
		log_warn("category_core_commit(): core_admin_commit(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	return 0;
}

int category_core_rollback(size_t argc, char **args) {
	int errsv = 0;

	/* Usage: rollback core */
	if (argc) {
		usage_admin_error_set(USCHED_USAGE_ADMIN_ERR_INSUFF_ARGS, "rollback core");
		log_warn("category_core_rollback(): Too many arguments.\n");
		errno = EINVAL;
		return -1;
	}

	if (core_admin_rollback() < 0) {
		errsv = errno;
		log_warn("category_core_rollback(): core_admin_rollback(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	return 0;
}

int category_core_change(size_t argc, char **args) {
	int errsv = 0;

	/* Usage: change core <component> <property> <value> */
	if (argc < 3) {
		usage_admin_error_set(USCHED_USAGE_ADMIN_ERR_INSUFF_ARGS, "change core");
		log_warn("category_core_change(): Insufficient arguments.\n");
		errno = EINVAL;
		return -1;
	}

	if (!strcasecmp(args[0], USCHED_COMPONENT_DELTA_STR)) {
		if (!strcasecmp(args[1], USCHED_PROPERTY_NOEXEC_STR)) {
			/* set delta.noexec */
			if (core_admin_delta_noexec_change(args[2]) < 0) {
				errsv = errno;
				log_warn("category_core_change(): core_admin_delta_noexec_change(): %s\n", strerror(errno));
				errno = errsv;
				return -1;
			}

			/* All good */
			return 0;
		} else if (!strcasecmp(args[1], USCHED_PROPERTY_RELOAD_STR)) {
			/* set delta.reload */
			if (core_admin_delta_reload_change(args[2]) < 0) {
				errsv = errno;
				log_warn("category_core_change(): core_admin_delta_reload_change(): %s\n", strerror(errno));
				errno = errsv;
				return -1;
			}

			/* All good */
			return 0;
		}

		/* Unknown property */
		usage_admin_error_set(USCHED_USAGE_ADMIN_ERR_INVALID_PROPERTY, "change core delta");
		log_warn("category_core_change(): Invalid 'delta' property: %s\n", args[1]);
		errno = EINVAL;

		return -1;
	} else if (!strcasecmp(args[0], USCHED_COMPONENT_JAIL_STR)) {
		if (!strcasecmp(args[1], USCHED_PROPERTY_DIR_STR)) {
			/* set jail.dir */
			if (core_admin_jail_dir_change(args[2]) < 0) {
				errsv = errno;
				log_warn("category_core_change(): core_admin_jail_dir_change(): %s\n", strerror(errno));
				errno = errsv;
				return -1;
			}

			/* All good */
			return 0;
		}

		/* Unknown property */
		usage_admin_error_set(USCHED_USAGE_ADMIN_ERR_INVALID_PROPERTY, "change core jail");
		log_warn("category_core_change(): Invalid 'jail' property: %s\n", args[1]);
		errno = EINVAL;

		return -1;
	} else if (!strcasecmp(args[0], USCHED_COMPONENT_IPC_STR)) {
		if (!strcasecmp(args[1], USCHED_PROPERTY_MSGMAX_STR)) {
			/* set ipc.msgmax */
			if (core_admin_ipc_msgmax_change(args[2]) < 0) {
				errsv = errno;
				log_warn("category_core_change(): core_admin_ipc_msgmax_change(): %s\n", strerror(errno));
				errno = errsv;
				return -1;
			}

			/* All good */
			return 0;
		} else if (!strcasecmp(args[1], USCHED_PROPERTY_MSGSIZE_STR)) {
			/* set ipc.msgsize */
			if (core_admin_ipc_msgsize_change(args[2]) < 0) {
				errsv = errno;
				log_warn("category_core_change(): core_admin_ipc_msgsize_change(): %s\n", strerror(errno));
				errno = errsv;
				return -1;
			}

			/* All good */
			return 0;
		} else if (!strcasecmp(args[1], USCHED_PROPERTY_NAME_STR)) {
			/* set ipc.name */
			if (core_admin_ipc_name_change(args[2]) < 0) {
				errsv = errno;
				log_warn("category_core_change(): core_admin_ipc_name_change(): %s\n", strerror(errno));
				errno = errsv;
				return -1;
			}

			/* All good */
			return 0;
		}

		/* Unknown property */
		usage_admin_error_set(USCHED_USAGE_ADMIN_ERR_INVALID_PROPERTY, "change core ipc");
		log_warn("category_core_change(): Invalid 'ipc' property: %s\n", args[1]);
		errno = EINVAL;

		return -1;
	} else if (!strcasecmp(args[0], USCHED_COMPONENT_PRIVDROP_STR)) {
		if (!strcasecmp(args[1], USCHED_PROPERTY_GROUP_STR)) {
			/* set privdrop.group */
			if (core_admin_privdrop_group_change(args[2]) < 0) {
				errsv = errno;
				log_warn("category_core_change(): core_admin_privdrop_group_change(): %s\n", strerror(errno));
				errno = errsv;
				return -1;
			}

			/* All good */
			return 0;
		} else if (!strcasecmp(args[1], USCHED_PROPERTY_USER_STR)) {
			/* set privdrop.user */
			if (core_admin_privdrop_user_change(args[2]) < 0) {
				errsv = errno;
				log_warn("category_core_change(): core_admin_privdrop_user_change(): %s\n", strerror(errno));
				errno = errsv;
				return -1;
			}

			/* All good */
			return 0;
		}

		/* Unknown property */
		usage_admin_error_set(USCHED_USAGE_ADMIN_ERR_INVALID_PROPERTY, "change core privdrop");
		log_warn("category_core_change(): Invalid 'privdrop' property: %s\n", args[1]);
		errno = EINVAL;

		return -1;
	} else if (!strcasecmp(args[0], USCHED_COMPONENT_SERIALIZE_STR)) {
		if (!strcasecmp(args[1], USCHED_PROPERTY_FILE_STR)) {
			/* set serialize.file */
			if (core_admin_serialize_file_change(args[2]) < 0) {
				errsv = errno;
				log_warn("category_core_change(): core_admin_serialize_file_change(): %s\n", strerror(errno));
				errno = errsv;
				return -1;
			}

			/* All good */
			return 0;
		}

		/* Unknown property */
		usage_admin_error_set(USCHED_USAGE_ADMIN_ERR_INVALID_PROPERTY, "change core serialize");
		log_warn("category_core_change(): Invalid 'serialize' property: %s\n", args[1]);
		errno = EINVAL;

		return -1;
	} else if (!strcasecmp(args[0], USCHED_COMPONENT_THREAD_STR)) {
		if (!strcasecmp(args[1], USCHED_PROPERTY_PRIORITY_STR)) {
			/* set thread.priority */
			if (core_admin_thread_priority_change(args[2]) < 0) {
				errsv = errno;
				log_warn("category_core_change(): core_admin_thread_priority_change(): %s\n", strerror(errno));
				errno = errsv;
				return -1;
			}

			/* All good */
			return 0;
		} else if (!strcasecmp(args[1], USCHED_PROPERTY_WORKERS_STR)) {
			/* set thread.workers */
			if (core_admin_thread_workers_change(args[2]) < 0) {
				errsv = errno;
				log_warn("category_core_change(): core_admin_thread_workers_change(): %s\n", strerror(errno));
				errno = errsv;
				return -1;
			}

			/* All good */
			return 0;
		}

		/* Unknown property */
		usage_admin_error_set(USCHED_USAGE_ADMIN_ERR_INVALID_PROPERTY, "change core thread");
		log_warn("category_core_change(): Invalid 'thread' property: %s\n", args[1]);
		errno = EINVAL;

		return -1;
	}

	/* Unknown component */
	usage_admin_error_set(USCHED_USAGE_ADMIN_ERR_INVALID_COMPONENT, "change core");
	log_warn("category_core_change(): Invalid 'change core' component: %s\n", args[0]);
	errno = EINVAL;

	return -1;
}

int category_core_show(size_t argc, char **args) {
	int errsv = 0;

	/* Usage: show core <component> <property> */
	if (!argc) {
		core_admin_show();
		return 0;
	}

	if (argc < 2) {
		usage_admin_error_set(USCHED_USAGE_ADMIN_ERR_INSUFF_ARGS, "show core");
		log_warn("category_core_change(): Insufficient arguments.\n");
		errno = EINVAL;
		return -1;
	}

	if (!strcasecmp(args[0], USCHED_COMPONENT_DELTA_STR)) {
		if (!strcasecmp(args[1], USCHED_PROPERTY_NOEXEC_STR)) {
			/* show delta.noexec */
			if (core_admin_delta_noexec_show() < 0) {
				errsv = errno;
				log_warn("category_core_show(): core_admin_delta_noexec_show(): %s\n", strerror(errno));
				errno = errsv;
				return -1;
			}

			/* All good */
			return 0;
		} else if (!strcasecmp(args[1], USCHED_PROPERTY_RELOAD_STR)) {
			/* show delta.reload */
			if (core_admin_delta_reload_show() < 0) {
				errsv = errno;
				log_warn("category_core_show(): core_admin_delta_reload_show(): %s\n", strerror(errno));
				errno = errsv;
				return -1;
			}

			/* All good */
			return 0;
		}

		/* Unknown property */
		usage_admin_error_set(USCHED_USAGE_ADMIN_ERR_INVALID_PROPERTY, "show core delta");
		log_warn("category_core_show(): Invalid 'delta' property: %s\n", args[1]);
		errno = EINVAL;

		return -1;
	} else if (!strcasecmp(args[0], USCHED_COMPONENT_JAIL_STR)) {
		if (!strcasecmp(args[1], USCHED_PROPERTY_DIR_STR)) {
			/* show jail.dir */
			if (core_admin_jail_dir_show() < 0) {
				errsv = errno;
				log_warn("category_core_show(): core_admin_jail_dir_show(): %s\n", strerror(errno));
				errno = errsv;
				return -1;
			}

			/* All good */
			return 0;
		}

		/* Unknown property */
		usage_admin_error_set(USCHED_USAGE_ADMIN_ERR_INVALID_PROPERTY, "show core jail");
		log_warn("category_core_show(): Invalid 'jail' property: %s\n", args[1]);
		errno = EINVAL;

		return -1;
	} else if (!strcasecmp(args[0], USCHED_COMPONENT_IPC_STR)) {
		if (!strcasecmp(args[1], USCHED_PROPERTY_MSGMAX_STR)) {
			/* show ipc.msgmax */
			if (core_admin_ipc_msgmax_show() < 0) {
				errsv = errno;
				log_warn("category_core_show(): core_admin_ipc_msgmax_show(): %s\n", strerror(errno));
				errno = errsv;
				return -1;
			}

			/* All good */
			return 0;
		} else if (!strcasecmp(args[1], USCHED_PROPERTY_MSGSIZE_STR)) {
			/* show ipc.msgsize */
			if (core_admin_ipc_msgsize_show() < 0) {
				errsv = errno;
				log_warn("category_core_show(): core_admin_ipc_msgsize_show(): %s\n", strerror(errno));
				errno = errsv;
				return -1;
			}

			/* All good */
			return 0;
		} else if (!strcasecmp(args[1], USCHED_PROPERTY_NAME_STR)) {
			/* show ipc.name */
			if (core_admin_ipc_name_show() < 0) {
				errsv = errno;
				log_warn("category_core_show(): core_admin_ipc_name_show(): %s\n", strerror(errno));
				errno = errsv;
				return -1;
			}

			/* All good */
			return 0;
		}

		/* Unknown property */
		usage_admin_error_set(USCHED_USAGE_ADMIN_ERR_INVALID_PROPERTY, "show core ipc");
		log_warn("category_core_show(): Invalid 'ipc' property: %s\n", args[1]);
		errno = EINVAL;

		return -1;
	} else if (!strcasecmp(args[0], USCHED_COMPONENT_PRIVDROP_STR)) {
		if (!strcasecmp(args[1], USCHED_PROPERTY_GROUP_STR)) {
			/* show privdrop.group */
			if (core_admin_privdrop_group_show() < 0) {
				errsv = errno;
				log_warn("category_core_show(): core_admin_privdrop_group_show(): %s\n", strerror(errno));
				errno = errsv;
				return -1;
			}

			/* All good */
			return 0;
		} else if (!strcasecmp(args[1], USCHED_PROPERTY_USER_STR)) {
			/* show privdrop.user */
			if (core_admin_privdrop_user_show() < 0) {
				errsv = errno;
				log_warn("category_core_show(): core_admin_privdrop_user_show(): %s\n", strerror(errno));
				errno = errsv;
				return -1;
			}

			/* All good */
			return 0;
		}

		/* Unknown property */
		usage_admin_error_set(USCHED_USAGE_ADMIN_ERR_INVALID_PROPERTY, "show core privdrop");
		log_warn("category_core_show(): Invalid 'privdrop' property: %s\n", args[1]);
		errno = EINVAL;

		return -1;
	} else if (!strcasecmp(args[0], USCHED_COMPONENT_SERIALIZE_STR)) {
		if (!strcasecmp(args[1], USCHED_PROPERTY_FILE_STR)) {
			/* show serialize.file */
			if (core_admin_serialize_file_show() < 0) {
				errsv = errno;
				log_warn("category_core_show(): core_admin_serialize_file_show(): %s\n", strerror(errno));
				errno = errsv;
				return -1;
			}

			/* All good */
			return 0;
		}

		/* Unknown property */
		usage_admin_error_set(USCHED_USAGE_ADMIN_ERR_INVALID_PROPERTY, "show core serialize");
		log_warn("category_core_show(): Invalid 'serialize' property: %s\n", args[1]);
		errno = EINVAL;

		return -1;
	} else if (!strcasecmp(args[0], USCHED_COMPONENT_THREAD_STR)) {
		if (!strcasecmp(args[1], USCHED_PROPERTY_PRIORITY_STR)) {
			/* show thread.priority */
			if (core_admin_thread_priority_show() < 0) {
				errsv = errno;
				log_warn("category_core_show(): core_admin_thread_priority_show(): %s\n", strerror(errno));
				errno = errsv;
				return -1;
			}

			/* All good */
			return 0;
		} else if (!strcasecmp(args[1], USCHED_PROPERTY_WORKERS_STR)) {
			/* show thread.workers */
			if (core_admin_thread_workers_show() < 0) {
				errsv = errno;
				log_warn("category_core_show(): core_admin_thread_workers_show(): %s\n", strerror(errno));
				errno = errsv;
				return -1;
			}

			/* All good */
			return 0;
		}

		/* Unknown property */
		usage_admin_error_set(USCHED_USAGE_ADMIN_ERR_INVALID_PROPERTY, "show core thread");
		log_warn("category_core_show(): Invalid 'thread' property: %s\n", args[1]);
		errno = EINVAL;

		return -1;
	}

	/* Unknown component */
	usage_admin_error_set(USCHED_USAGE_ADMIN_ERR_INVALID_COMPONENT, "show core");
	log_warn("category_core_show(): Invalid 'show core' component: %s\n", args[0]);
	errno = EINVAL;

	return -1;
}

int category_network_commit(size_t argc, char **args) {
	int errsv = 0;

	/* Usage: commit network */
	if (argc) {
		usage_admin_error_set(USCHED_USAGE_ADMIN_ERR_INSUFF_ARGS, "commit network");
		log_warn("category_network_commit(): Too many arguments.\n");
		errno = EINVAL;
		return -1;
	}

	if (network_admin_commit() < 0) {
		errsv = errno;
		log_warn("category_network_commit(): network_admin_commit(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	return 0;
}

int category_network_rollback(size_t argc, char **args) {
	int errsv = 0;

	/* Usage: rollback network */
	if (argc) {
		usage_admin_error_set(USCHED_USAGE_ADMIN_ERR_INSUFF_ARGS, "rollback network");
		log_warn("category_network_rollback(): Too many arguments.\n");
		errno = EINVAL;
		return -1;
	}

	if (network_admin_rollback() < 0) {
		errsv = errno;
		log_warn("category_network_rollback(): network_admin_rollback(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	return 0;
}

int category_network_change(size_t argc, char **args) {
	int errsv = 0;

	/* Usage: change network <component> <property> <value> */
	if (argc < 3) {
		usage_admin_error_set(USCHED_USAGE_ADMIN_ERR_INSUFF_ARGS, "change network");
		log_warn("category_network_change(): Insufficient arguments.\n");
		errno = EINVAL;
		return -1;
	}

	if (!strcasecmp(args[0], USCHED_COMPONENT_BIND_STR)) {
		if (!strcasecmp(args[1], USCHED_PROPERTY_ADDR_STR)) {
			/* set bind.addr */
			if (network_admin_bind_addr_change(args[2]) < 0) {
				errsv = errno;
				log_warn("category_network_change(): network_admin_bind_addr_change(): %s\n", strerror(errno));
				errno = errsv;
				return -1;
			}

			/* All good */
			return 0;
		} else if (!strcasecmp(args[1], USCHED_PROPERTY_PORT_STR)) {
			/* set bind.port */
			if (network_admin_bind_port_change(args[2]) < 0) {
				errsv = errno;
				log_warn("category_network_change(): network_admin_bind_port_change(): %s\n", strerror(errno));
				errno = errsv;
				return -1;
			}

			/* All good */
			return 0;
		}

		/* Unknown property */
		usage_admin_error_set(USCHED_USAGE_ADMIN_ERR_INVALID_PROPERTY, "change network bind");
		log_warn("category_network_change(): Invalid 'bind' property: %s\n", args[1]);
		errno = EINVAL;

		return -1;
	} else if (!strcasecmp(args[0], USCHED_COMPONENT_CONN_STR)) {
		if (!strcasecmp(args[1], USCHED_PROPERTY_LIMIT_STR)) {
			/* set conn.limit */
			if (network_admin_conn_limit_change(args[2]) < 0) {
				errsv = errno;
				log_warn("category_network_change(): network_admin_conn_limit_change(): %s\n", strerror(errno));
				errno = errsv;
				return -1;
			}

			/* All good */
			return 0;
		} else if (!strcasecmp(args[1], USCHED_PROPERTY_TIMEOUT_STR)) {
			/* set conn.timeout */
			if (network_admin_conn_timeout_change(args[2]) < 0) {
				errsv = errno;
				log_warn("category_network_change(): network_admin_conn_timeout_change(): %s\n", strerror(errno));
				errno = errsv;
				return -1;
			}

			/* All good */
			return 0;
		}

		/* Unknown property */
		usage_admin_error_set(USCHED_USAGE_ADMIN_ERR_INVALID_PROPERTY, "change network conn");
		log_warn("category_network_change(): Invalid 'conn' property: %s\n", args[1]);
		errno = EINVAL;

		return -1;
	} else if (!strcasecmp(args[0], USCHED_COMPONENT_SOCK_STR)) {
		if (!strcasecmp(args[1], USCHED_PROPERTY_NAME_STR)) {
			/* set sock.name */
			if (network_admin_sock_name_change(args[2]) < 0) {
				errsv = errno;
				log_warn("category_network_change(): network_admin_sock_name_change(): %s\n", strerror(errno));
				errno = errsv;
				return -1;
			}

			/* All good */
			return 0;
		}

		/* Unknown property */
		usage_admin_error_set(USCHED_USAGE_ADMIN_ERR_INVALID_PROPERTY, "change network sock");
		log_warn("category_network_change(): Invalid 'sock' property: %s\n", args[1]);
		errno = EINVAL;

		return -1;
	}
	
	/* Unknown component */
	usage_admin_error_set(USCHED_USAGE_ADMIN_ERR_INVALID_COMPONENT, "change network");
	log_warn("category_network_change(): Invalid 'change network' component: %s\n", args[0]);
	errno = EINVAL;

	return -1;
}

int category_network_show(size_t argc, char **args) {
	int errsv = 0;

	/* Usage: show network <component> <property> */
	if (!argc) {
		network_admin_show();
		return 0;
	}

	if (argc < 2) {
		usage_admin_error_set(USCHED_USAGE_ADMIN_ERR_INSUFF_ARGS, "show network");
		log_warn("category_network_show(): Insufficient arguments.\n");
		errno = EINVAL;
		return -1;
	}

	if (!strcasecmp(args[0], USCHED_COMPONENT_BIND_STR)) {
		if (!strcasecmp(args[1], USCHED_PROPERTY_ADDR_STR)) {
			/* show bind.addr */
			if (network_admin_bind_addr_show() < 0) {
				errsv = errno;
				log_warn("category_network_show(): network_admin_bind_addr_show(): %s\n", strerror(errno));
				errno = errsv;
				return -1;
			}

			/* All good */
			return 0;
		} else if (!strcasecmp(args[1], USCHED_PROPERTY_PORT_STR)) {
			/* show bind.port */
			if (network_admin_bind_port_show() < 0) {
				errsv = errno;
				log_warn("category_network_show(): network_admin_bind_port_show(): %s\n", strerror(errno));
				errno = errsv;
				return -1;
			}

			/* All good */
			return 0;
		}

		/* Unknown property */
		usage_admin_error_set(USCHED_USAGE_ADMIN_ERR_INVALID_PROPERTY, "show network bind");
		log_warn("category_network_show(): Invalid 'bind' property: %s\n", args[1]);
		errno = EINVAL;

		return -1;
	} else if (!strcasecmp(args[0], USCHED_COMPONENT_CONN_STR)) {
		if (!strcasecmp(args[1], USCHED_PROPERTY_LIMIT_STR)) {
			/* show conn.limit */
			if (network_admin_conn_limit_show() < 0) {
				errsv = errno;
				log_warn("category_network_show(): network_admin_conn_limit_show(): %s\n", strerror(errno));
				errno = errsv;
				return -1;
			}

			/* All good */
			return 0;
		} else if (!strcasecmp(args[1], USCHED_PROPERTY_TIMEOUT_STR)) {
			/* show conn.timeout */
			if (network_admin_conn_timeout_show() < 0) {
				errsv = errno;
				log_warn("category_network_show(): network_admin_conn_timeout_show(): %s\n", strerror(errno));
				errno = errsv;
				return -1;
			}

			/* All good */
			return 0;
		}

		/* Unknown property */
		usage_admin_error_set(USCHED_USAGE_ADMIN_ERR_INVALID_PROPERTY, "show network conn");
		log_warn("category_network_show(): Invalid 'conn' property: %s\n", args[1]);
		errno = EINVAL;

		return -1;
	} else if (!strcasecmp(args[0], USCHED_COMPONENT_SOCK_STR)) {
		if (!strcasecmp(args[1], USCHED_PROPERTY_NAME_STR)) {
			/* show sock.name */
			if (network_admin_sock_name_show() < 0) {
				errsv = errno;
				log_warn("category_network_show(): network_admin_sock_name_show(): %s\n", strerror(errno));
				errno = errsv;
				return -1;
			}

			/* All good */
			return 0;
		}

		/* Unknown property */
		usage_admin_error_set(USCHED_USAGE_ADMIN_ERR_INVALID_PROPERTY, "show network sock");
		log_warn("category_network_show(): Invalid 'sock' property: %s\n", args[1]);
		errno = EINVAL;

		return -1;
	}
	
	/* Unknown component */
	usage_admin_error_set(USCHED_USAGE_ADMIN_ERR_INVALID_COMPONENT, "show network");
	log_warn("category_network_show(): Invalid 'show network' component: %s\n", args[0]);
	errno = EINVAL;

	return -1;
}

int category_users_commit(size_t argc, char **args) {
	int errsv = 0;

	/* Usage: commit users */
	if (argc) {
		usage_admin_error_set(USCHED_USAGE_ADMIN_ERR_INSUFF_ARGS, "commit users");
		log_warn("category_users_commit(): Too many arguments.\n");
		errno = EINVAL;
		return -1;
	}

	if (users_admin_commit() < 0) {
		errsv = errno;
		log_warn("category_users_commit(): users_admin_commit(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	return 0;
}

int category_users_rollback(size_t argc, char **args) {
	int errsv = 0;

	/* Usage: commit users */
	if (argc) {
		usage_admin_error_set(USCHED_USAGE_ADMIN_ERR_INSUFF_ARGS, "rollback users");
		log_warn("category_users_rollback(): Too many arguments.\n");
		errno = EINVAL;
		return -1;
	}

	if (users_admin_rollback() < 0) {
		errsv = errno;
		log_warn("category_users_rollback(): users_admin_rollback(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	return 0;
}

int category_users_add(size_t argc, char **args) {
	int errsv = 0;
	char *endptr = NULL;
	char *username = NULL;
	char *password = NULL;
	char pw_input[CONFIG_USCHED_AUTH_PASSWORD_MAX + 1];
	char pw_input_repeat[CONFIG_USCHED_AUTH_PASSWORD_MAX + 1];
	uid_t uid = (uid_t) -1;
	gid_t gid = (gid_t) -1;

	/* Usage: <username> <uid> <gid> <password> */
	if (argc < 3) {
		usage_admin_error_set(USCHED_USAGE_ADMIN_ERR_INSUFF_ARGS, "add users");
		log_warn("category_users_add(): Insufficient arguments.\n");
		errno = EINVAL;
		return -1;
	} else if (argc > 4) {
		usage_admin_error_set(USCHED_USAGE_ADMIN_ERR_TOOMANY_ARGS, "add users");
		log_warn("category_users_add(): Too many arguments.\n");
		errno = EINVAL;
		return -1;
	} if (argc == 3) {
		printf("Password: ");

		if (input_password(pw_input, sizeof(pw_input)) < 0) {
			errsv = errno;
			log_warn("category_users_add(): input_password(): %s\n", strerror(errno));
			errno = errsv;
			return -1;
		}

		printf("\nRepeat password: ");

		if (input_password(pw_input_repeat, sizeof(pw_input)) < 0) {
			errsv = errno;
			log_warn("category_users_add(): input_password(): %s\n", strerror(errno));
			errno = errsv;
			return -1;
		}

		if (strcmp(pw_input, pw_input_repeat)) {
			puts("\nPasswords do not match.\n");
			log_warn("category_users_add(): Passwords do not match.\n");
			errno = EINVAL;
			return -1;
		}

		puts("");

		password = pw_input;

	} else {
		password = args[3];
	}

	/* Check if password is too short */
	if (strlen(password) < CONFIG_USCHED_AUTH_PASSWORD_MIN) {
		puts("Password is too short (it must be at least 8 characters long).\n");
		log_warn("category_users_add(): Password is too short (it must be at least 8 characters long).\n");
		errno = EINVAL;
		return -1;
	}

	username = args[0];

	uid = strtoul(args[1], &endptr, 0);

	if ((*endptr) || (endptr == args[1]) || (errno == EINVAL) || (errno == ERANGE)) {
		log_warn("category_users_add(): Invalid UID: %s\n", args[1]);
		errno = EINVAL;
		return -1;
	}

	gid = strtoul(args[2], &endptr, 0);

	if ((*endptr) || (endptr == args[1]) || (errno == EINVAL) || (errno == ERANGE)) {
		log_warn("category_users_add(): Invalid GID: %s\n", args[1]);
		errno = EINVAL;
		return -1;
	}

	/* Add the user */
	if (users_admin_add(username, uid, gid, password) < 0) {
		errsv = errno;
		log_warn("category_users_add(): users_admin_add(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Print the result */
	print_admin_config_user_added(username);

	return 0;
}

int category_users_delete(size_t argc, char **args) {
	int errsv = 0;
	char *username = NULL;

	/* Usage: <username> */
	if (argc != 1) {
		usage_admin_error_set(USCHED_USAGE_ADMIN_ERR_TOOMANY_ARGS, "delete users");
		log_warn("category_users_delete(): Too many arguments.\n");
		errno = EINVAL;
		return -1;
	}

	username = args[0];

	/* Delete the user */
	if (users_admin_delete(username) < 0) {
		errsv = errno;
		log_warn("category_users_delete(): users_admin_delete(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Print the result */
	print_admin_config_user_deleted(username);

	return 0;
}

int category_users_change(size_t argc, char **args) {
	int errsv = 0;
	char *endptr = NULL;
	char *username = NULL;
	char *password = NULL;
	char pw_input[CONFIG_USCHED_AUTH_PASSWORD_MAX + 1];
	char pw_input_repeat[CONFIG_USCHED_AUTH_PASSWORD_MAX + 1];
	uid_t uid = (uid_t) -1;
	gid_t gid = (gid_t) -1;

	memset(pw_input, 0, sizeof(pw_input));

	/* Usage: <username> <uid> <gid> <password> */
	if (argc < 3) {
		usage_admin_error_set(USCHED_USAGE_ADMIN_ERR_INSUFF_ARGS, "change users");
		log_warn("category_users_change(): Insufficient arguments.\n");
		errno = EINVAL;
		return -1;
	} else if (argc > 4) {
		usage_admin_error_set(USCHED_USAGE_ADMIN_ERR_TOOMANY_ARGS, "change users");
		log_warn("category_users_change(): Too many arguments.\n");
		errno = EINVAL;
		return -1;
	} else if (argc == 3) {
		printf("Password: ");

		if (input_password(pw_input, sizeof(pw_input)) < 0) {
			errsv = errno;
			log_warn("category_users_change(): input_password(): %s\n", strerror(errno));
			errno = errsv;
			return -1;
		}

		printf("\nRepeat password: ");

		if (input_password(pw_input_repeat, sizeof(pw_input)) < 0) {
			errsv = errno;
			log_warn("category_users_change(): input_password(): %s\n", strerror(errno));
			errno = errsv;
			return -1;
		}

		if (strcmp(pw_input, pw_input_repeat)) {
			puts("\nPasswords do not match.\n");
			log_warn("category_users_change(): Passwords do not match.\n");
			errno = EINVAL;
			return -1;
		}

		puts("");

		password = pw_input;
	} else {
		password = args[3];
	}

	/* Check if password is too short */
	if (strlen(password) < CONFIG_USCHED_AUTH_PASSWORD_MIN) {
		puts("Password is too short (it must be at least 8 characters long).\n");
		log_warn("category_users_add(): Password is too short (it must be at least 8 characters long).\n");
		errno = EINVAL;
		return -1;
	}

	username = args[0];

	uid = strtoul(args[1], &endptr, 0);

	if ((*endptr) || (endptr == args[1]) || (errno == EINVAL) || (errno == ERANGE)) {
		log_warn("category_users_change(): Invalid UID: %s\n", args[1]);
		errno = EINVAL;
		return -1;
	}

	gid = strtoul(args[2], &endptr, 0);

	if ((*endptr) || (endptr == args[1]) || (errno == EINVAL) || (errno == ERANGE)) {
		log_warn("category_users_change(): Invalid GID: %s\n", args[1]);
		errno = EINVAL;
		return -1;
	}

	/* Change the user */
	if (users_admin_change(username, uid, gid, password) < 0) {
		errsv = errno;
		log_warn("category_users_change(): users_admin_change(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	/* Print the result */
	print_admin_config_user_changed(username);

	return 0;
}

int category_users_show(size_t argc, char **args) {
	int errsv = 0;

	/* Usage: no arguments */
	if (argc) {
		usage_admin_error_set(USCHED_USAGE_ADMIN_ERR_TOOMANY_ARGS, "show users");
		log_warn("category_users_change(): Too many arguments.\n");
		errno = EINVAL;
		return -1;
	}

	/* Show the users */
	if (users_admin_show() < 0) {
		errsv = errno;
		log_warn("category_users_show(): users_admin_show(): %s\n", strerror(errno));
		errno = errsv;
		return -1;
	}

	return 0;
}

