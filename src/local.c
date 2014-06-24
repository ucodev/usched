/**
 * @file local.c
 * @brief uSched
 *        Local utilities and handlers interface
 *
 * Date: 24-06-2014
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
#include <errno.h>

#if defined(CONFIG_SYS_LINUX) || defined(CONFIG_SYS_NETBSD)
 #include <sys/types.h>
 #include <sys/socket.h>
 #include <sys/un.h>
 #include <unistd.h>
#elif defined(CONFIG_SYS_BSD)
 #include <sys/types.h>
 #include <unistd.h>
#elif defined(CONFIG_SYS_SOLARIS)
 #include <sys/types.h>
 #include <ucred.h>
#else
 #include <sys/types.h>
#endif

#include "log.h"
#include "local.h"

int local_fd_peer_cred(int fd, uid_t *uid, gid_t *gid) {
#if defined(CONFIG_SYS_LINUX) || defined(CONFIG_SYS_NETBSD)
	socklen_t uc_len = 0;
 #if defined(SO_PEERCRED)
  #define uc_get_uid(uc)		(uc.uid)
  #define uc_get_gid(uc)		(uc.gid)
	struct ucred uc;

	memset(&uc, 0, sizeof(struct ucred));
	uc_len = sizeof(struct ucred);

	if (getsockopt(fd, SOL_SOCKET, SO_PEERCRED, &uc, &uc_len) < 0) {
 #elif defined(LOCAL_PEERCRED)
  #define uc_get_uid(uc)		(uc.cr_uid)
  #define uc_get_gid(uc)		(uc.cr_gid)
	struct xucred uc;

	memset(&uc, 0, sizeof(struct xucred));
	uc_len = sizeof(struct xucred);

	if (getsockopt(fd, 0, LOCAL_PEERCRED, &uc, &uc_len) < 0) {
 #endif
		log_warn("local_fd_peer_cred(): getsockopt(): %s\n", strerror(errno));
		return -1;
	}

	*uid = uc_get_uid(uc);
	*gid = uc_get_gid(uc);

 #undef uc_get_uid
 #undef uc_get_gid

	return 0;
#elif defined(CONFIG_SYS_BSD)
	if (getpeereid(fd, uid, gid) < 0) {
		log_warn("local_fd_peer_cred(): getpeereid(): %s\n", strerror(errno));
		return -1;
	}

	return 0;
#elif defined(CONFIG_SYS_SOLARIS)
	ucred_t *uc = NULL;

	if (getpeerucred(fd, &ucred) < 0) {
		log_warn("local_fd_peer_cred(): getpeerucred(): %s\n", strerror(errno));
		return -1;
	}

	*uid = ucred_geteuid(uc);
	*gid = ucred_getegid(uc);

	ucred_free(uc);

	return 0;
#else
	errno = ENOSYS;
#endif
	return -1;
}

