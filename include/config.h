/**
 * @file config.h
 * @brief uSched
 *        Configuration interface header
 *
 * Date: 24-04-2015
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


#ifndef USCHED_CONFIG_H
#define USCHED_CONFIG_H

#error "WARNING: This is the uSched tree v0.2-xx. This tree is under development and may not even be functional. Please fetch the latest release from the v0.1 tree if you want to test the project. If you really want to compile and develop the v0.2-xx tree, run ./undo and then remove this #error from the include/config.h file. Then, run ./do again."

#define CONFIG_USCHED_DEBUG			1
#define CONFIG_USCHED_MULTIUSER			1
#define CONFIG_USCHED_DROP_PRIVS		1
#define CONFIG_USCHED_JAIL			1
#define CONFIG_USCHED_SERIALIZE_ON_REQ		1
#define CONFIG_USCHED_DELTA_CHECK_INTERVAL	1
#define CONFIG_USCHED_SHELL_BIN_PATH		"/bin/sh"
#define CONFIG_USCHED_DIR_BASE			"@_SYSCONFDIR_@/usched"
#define CONFIG_USCHED_NET_DEFAULT_PORT		"7600"
#define CONFIG_USCHED_DIR_AUTH			"auth"
#define CONFIG_USCHED_DIR_CORE			"core"
#define CONFIG_USCHED_DIR_EXEC			"exec"
#define CONFIG_USCHED_DIR_NETWORK		"network"
#define CONFIG_USCHED_DIR_STAT			"stat"
#define CONFIG_USCHED_DIR_USERS			"users"
#define CONFIG_USCHED_FILE_AUTH_BL_GID		"blacklist.gid"
#define CONFIG_USCHED_FILE_AUTH_BL_UID		"blacklist.uid"
#define CONFIG_USCHED_FILE_AUTH_WL_GID		"whitelist.gid"
#define CONFIG_USCHED_FILE_AUTH_WL_UID		"whitelist.uid"
#define CONFIG_USCHED_FILE_AUTH_LOCAL_USE	"local.use"
#define CONFIG_USCHED_FILE_AUTH_REMOTE_USERS	"remote.users"
#define CONFIG_USCHED_FILE_CORE_DELTA_NOEXEC	"delta.noexec"
#define CONFIG_USCHED_FILE_CORE_DELTA_RELOAD	"delta.reload"
#define CONFIG_USCHED_FILE_CORE_JAIL_DIR	"jail.dir"
#define CONFIG_USCHED_FILE_CORE_IPC_MSGMAX	"ipc.msgmax"
#define CONFIG_USCHED_FILE_CORE_IPC_MSGSIZE	"ipc.msgsize"
#define CONFIG_USCHED_FILE_CORE_IPC_NAME	"ipc.name"
#define CONFIG_USCHED_FILE_CORE_IPC_KEY		"ipc.key"
#define CONFIG_USCHED_FILE_CORE_PRIVDROP_USER	"privdrop.user"
#define CONFIG_USCHED_FILE_CORE_PRIVDROP_GROUP	"privdrop.group"
#define CONFIG_USCHED_FILE_CORE_SERIALIZE_FILE	"serialize.file"
#define CONFIG_USCHED_FILE_CORE_THREAD_PRIORITY	"thread.priority"
#define CONFIG_USCHED_FILE_CORE_THREAD_WORKERS	"thread.workers"
#define CONFIG_USCHED_FILE_EXEC_IPC_MSGMAX	"ipc.msgmax"
#define CONFIG_USCHED_FILE_EXEC_IPC_MSGSIZE	"ipc.msgsize"
#define CONFIG_USCHED_FILE_EXEC_IPC_NAME	"ipc.name"
#define CONFIG_USCHED_FILE_EXEC_IPC_KEY		"ipc.key"
#define CONFIG_USCHED_FILE_NETWORK_BIND_ADDR	"bind.addr"
#define CONFIG_USCHED_FILE_NETWORK_BIND_PORT	"bind.port"
#define CONFIG_USCHED_FILE_NETWORK_CONN_LIMIT	"conn.limit"
#define CONFIG_USCHED_FILE_NETWORK_CONN_TIMEOUT	"conn.timeout"
#define CONFIG_USCHED_FILE_NETWORK_SOCK_NAME	"sock.name"
#define CONFIG_USCHED_FILE_STAT_JAIL_DIR	"jail.dir"
#define CONFIG_USCHED_FILE_STAT_IPC_MSGMAX	"ipc.msgmax"
#define CONFIG_USCHED_FILE_STAT_IPC_MSGSIZE	"ipc.msgsize"
#define CONFIG_USCHED_FILE_STAT_IPC_NAME	"ipc.name"
#define CONFIG_USCHED_FILE_STAT_IPC_KEY		"ipc.key"
#define CONFIG_USCHED_FILE_STAT_PRIVDROP_USER	"privdrop.user"
#define CONFIG_USCHED_FILE_STAT_PRIVDROP_GROUP	"privdrop.group"
#define CONFIG_USCHED_DAEMON_PID_FILE		"@_SYSRUNDIR_@/usched_usd.pid"
#define CONFIG_USCHED_EXEC_PID_FILE		"@_SYSRUNDIR_@/usched_use.pid"
#define CONFIG_USCHED_STAT_PID_FILE		"@_SYSRUNDIR_@/usched_uss.pid"
#define CONFIG_USCHED_ADMIN_PROC_NAME		"usa"
#define CONFIG_USCHED_CLIENT_PROC_NAME		"usc"
#define CONFIG_USCHED_DAEMON_PROC_NAME		"usd"
#define CONFIG_USCHED_EXEC_PROC_NAME		"use"
#define CONFIG_USCHED_MONITOR_PROC_NAME		"usm"
#define CONFIG_USCHED_STAT_PROC_NAME		"uss"
#define CONFIG_USCHED_LOG_MSG_MAX_SIZE		1024
#define CONFIG_USCHED_SEC_KDF_ROUNDS		5000
#define CONFIG_USCHED_AUTH_USERNAME_MAX		32
#define CONFIG_USCHED_AUTH_PASSWORD_MAX		256
#define CONFIG_USCHED_AUTH_PASSWORD_MIN		8
#define CONFIG_USCHED_AUTH_SESSION_MAX		272 /* Current mac: 257 */
#define CONFIG_USCHED_AUTH_IPC_SIZE		128 /* Size of IPC authentication string */
#define CONFIG_USCHED_EXEC_OUTPUT_MAX		4096 /* Max number of bytes to store output data */
#define CONFIG_USCHED_HASH_FNV1A		1
#define CONFIG_USCHED_HASH_DJB2			0
#define CONFIG_USCHED_IPC_TIMEOUT		3   /* Number of seconds before give up on sends */

#define CONFIG_POSIX_STRICT			0

/* #define CONFIG_SYS_LINUX			0 */
/* #define CONFIG_SYS_MINIX			0 */
/* #define CONFIG_SYS_NETBSD			0 */
/* #define CONFIG_SYS_BSD			0 */
/* #define CONFIG_SYS_SOLARIS			0 */
/* #define CONFIG_SYS_WINDOWS			0 */

#define CONFIG_SYS_DEV_ZERO			"/dev/zero"
#define CONFIG_SYS_DEV_NULL			"/dev/null"
#define CONFIG_SYS_EXIT_CODE_CUSTOM_BASE	90

#ifndef CONFIG_COMMON
#define CONFIG_COMMON				0
#endif
#ifndef CONFIG_CLIENT_ONLY
#define CONFIG_CLIENT_ONLY			0
#endif
#ifndef CONFIG_CLIENT_SPECIFIC
#define CONFIG_CLIENT_SPECIFIC			0
#endif
#ifndef CONFIG_DAEMON_SPECIFIC
#define CONFIG_DAEMON_SPECIFIC			0
#endif
#ifndef CONFIG_EXEC_SPECIFIC
#define CONFIG_EXEC_SPECIFIC			0
#endif
#ifndef CONFIG_ADMIN_SPECIFIC
#define CONFIG_ADMIN_SPECIFIC			0
#endif



#ifndef CONFIG_USE_LIBFSMA
 #define CONFIG_USE_LIBFSMA			0
#endif
#ifndef CONFIG_USE_SYNCFS
 #define CONFIG_USE_SYNCFS			0
#endif
#ifndef CONFIG_USE_SELECT
 #define CONFIG_USE_SELECT			0
#endif
#ifndef CONFIG_USE_IPC_SYSVMQ
 #define CONFIG_USE_IPC_SYSVMQ			0
#endif
#ifndef CONFIG_USE_IPC_PMQ
 #define CONFIG_USE_IPC_PMQ			0
#endif
#ifndef CONFIG_USE_IPC_UNIX
 #define CONFIG_USE_IPC_UNIX			0
#endif
#ifndef CONFIG_USE_IPC_INET
 #define CONFIG_USE_IPC_INET			0
#endif
#ifndef CONFIG_USE_IPC_INET_BINDADDR
 #define CONFIG_USE_IPC_INET_BINDADDR		"127.0.0.1"
#endif


/* Configuration compliance checks */
#if CONFIG_USE_IPC_PMQ == 0 && CONFIG_USE_IPC_UNIX == 0 && CONFIG_USE_IPC_INET == 0
 #error "No communication interface is set for uSched Execution module (CONFIG_USE_IPC_PMQ == 0 and CONFIG_USE_IPC_UNIX == 0 and CONFIG_USE_IPC_INET == 0)."
#endif
#if CONFIG_POSIX_STRICT == 1 && (CONFIG_USCHED_JAIL == 1 || CONFIG_USE_SYNCFS == 1)
 #error "CONFIG_POSIX_STRICT is incompatible with the following options: CONFIG_USCHED_JAIL, CONFIG_USE_SYNCFS"
#endif
#if CONFIG_USCHED_DROP_PRIVS == 0
 #warning "CONFIG_USCHED_DROP_PRIVS is disabled. It's strongly recommended to drop privileges by default."
#endif
#if CONFIG_USCHED_DROP_PRIVS == 1 && CONFIG_USCHED_SERIALIZE_ON_REQ == 0
 #error "CONFIG_USCHED_DROP_PRIVS cannot be enabled if CONFIG_USCHED_SERIALIZE_ON_REQ is disabled."
#endif
#if CONFIG_USCHED_JAIL == 1 && CONFIG_USCHED_DROP_PRIVS == 0
 #error "CONFIG_USCHED_JAIL is enabled while CONFIG_USCHED_DROP_PRIVS is disabled."
#endif
#if CONFIG_USCHED_AUTH_USERNAME_MAX < 8
 #error "CONFIG_USCHED_AUTH_USERNAME_MAX value must be equal or greater than 8"
#endif
#if CONFIG_USCHED_AUTH_PASSWORD_MAX < CONFIG_USCHED_AUTH_PASSWORD_MIN
 #error "CONFIG_USCHED_AUTH_PASSWORD_MAX value is lesser than CONFIG_USCHED_AUTH_PASSWORD_MIN."
#endif
#if CONFIG_USCHED_HASH_FNV1A == 1 && CONFIG_USCHED_HASH_DJB2 == 1
 #error "CONFIG_USCHED_HASH_FNV1A and CONFIG_USCHED_HASH_DJB2 are both enabled. Only one can be selected."
#endif
#if CONFIG_USCHED_HASH_FNV1A == 0 && CONFIG_USCHED_HASH_DJB2 == 0
 #error "CONFIG_USCHED_HASH_FNV1A and CONFIG_USCHED_HASH_DJB2 are both disabled. At least one must be enabled."
#endif
#if CONFIG_SYS_EXIT_CODE_CUSTOM_BASE < 80 || CONFIG_SYS_EXIT_CODE_CUSTOM_BASE > 125
 #error "CONFIG_SYS_EXIT_CODE_CUSTOM_BASE value must stand between 80 and 125"
#endif
#if CONFIG_USCHED_LOG_MSG_MAX_SIZE < 256
 #error "CONFIG_USCHED_LOG_MSG_MAX_SIZE value must be equal or greater than 256"
#endif
#if CONFIG_USCHED_SEC_KDF_ROUNDS < 1000
 #error "CONFIG_USCHED_SEC_KDF_ROUNDS value must be greater than 1000"
#endif


/* Custom exit status offsets (base value is CONFIG_SYS_EXIT_CODE_CUSTOM_BASE) */
enum CHILD_EXIT_STATUS_CUSTOM_OFFSET {
	CHILD_EXIT_STATUS_FAILED_SETSID = 1,
	CHILD_EXIT_STATUS_FAILED_FREOPEN_STDIN,
	CHILD_EXIT_STATUS_FAILED_FREOPEN_STDOUT,	/* Obsolete. Can be replaced. */
	CHILD_EXIT_STATUS_FAILED_FREOPEN_STDERR,	/* Obsolete. Can be replaced. */
	CHILD_EXIT_STATUS_FAILED_SETREGID,
	CHILD_EXIT_STATUS_FAILED_SETREUID,
	CHILD_EXIT_STATUS_FAILED_UID,
	CHILD_EXIT_STATUS_FAILED_GID,
	CHILD_EXIT_STATUS_FAILED_EXECLP
};

/* Custom exit codes for uSched processes (used to inform uSched monitor of some special states) */
enum PROCESS_EXIT_STATUS_CUSTOM_CODES {
	PROCESS_EXIT_STATUS_CUSTOM_RELOAD_NOPRIV = 77,
	PROCESS_EXIT_STATUS_CUSTOM_BAD_RUNTIME_OR_CONFIG,
	PROCESS_EXIT_STATUS_CUSTOM_UNKNOWN_ABORT
	/* NOTE: If more options are added here, make sure that CONFIG_SYS_EXIT_CODE_CUSTOM_BASE
	 *       value is greater than the maximum value of this enumeration.
	 */
};

/* Windows specific parameters */
#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64) || CONFIG_SYS_WINDOWS == 1
 #undef CONFIG_SYS_WINDOWS
 #define CONFIG_SYS_WINDOWS 1
 #ifndef COMPILE_WIN32
  #define COMPILE_WIN32 1
 #endif
 #undef CONFIG_CLIENT_ONLY
 #define CONFIG_CLIENT_ONLY 1
#endif

#ifdef COMPILE_WIN32
 #include <winsock2.h>
 #include <windows.h>

 #if BUILDING_DLL
  #define DLLIMPORT __declspec(dllexport)
 #else
  #define DLLIMPORT __declspec(dllimport)
 #endif
 
 typedef UINT32	uid_t, gid_t;
 typedef INT8	int8_t;
 typedef INT16	int16_t;
 typedef INT32	int32_t;
 typedef INT64	int64_t;
 typedef UINT8	uint8_t;
 typedef UINT16	uint16_t;
 typedef UINT32	uint32_t;
 typedef UINT64	uint64_t;
 
 #define strtok_r strtok_s
 #define getuid() 0
 #define getgid() 0

 enum {
	LOG_INFO = 1,
	LOG_WARNING,
	LOG_CRIT
 };
#endif

#include <sys/types.h>

#include <pall/cll.h>

#include <psec/ke.h>

/* Configuration structures */
struct usched_config_userinfo {
	char *username;
	char *password;
	char *salt;
	uid_t uid;
	gid_t gid;
};

struct usched_config_users {
	struct cll_handler *list;
};

struct usched_config_auth {
	struct cll_handler *blacklist_gid;
	struct cll_handler *whitelist_gid;
	struct cll_handler *blacklist_uid;
	struct cll_handler *whitelist_uid;
	unsigned int local_use;
	unsigned int pam_use;
	unsigned int remote_users;
};

struct usched_config_core {
	unsigned int delta_noexec;
	unsigned int delta_reload;
	char *serialize_file;
	char *jail_dir;
	long ipc_msgmax;
	long ipc_msgsize;
	char *ipc_name;
	char *ipc_key;
	char *privdrop_user;
	char *privdrop_group;
	uid_t privdrop_uid;
	gid_t privdrop_gid;
	long thread_priority;
	unsigned int thread_workers;
};

struct usched_config_exec {
	long ipc_msgmax;
	long ipc_msgsize;
	char *ipc_name;
	char *ipc_key;
};

struct usched_config_network {
	char *bind_addr;
	char *bind_port;
	unsigned int conn_limit;
	unsigned int conn_timeout;
	char *sock_name;
};

struct usched_config_stat {
	char *jail_dir;
	long ipc_msgmax;
	long ipc_msgsize;
	char *ipc_name;
	char *ipc_key;
	char *privdrop_user;
	char *privdrop_group;
	uid_t privdrop_uid;
	gid_t privdrop_gid;
};

struct usched_config {
	struct usched_config_auth auth;
	struct usched_config_core core;
	struct usched_config_exec exec;
	struct usched_config_network network;
	struct usched_config_stat stat;
	struct usched_config_users users;
};


/* Prototypes */
int config_init_auth(struct usched_config_auth *auth);
int config_init_core(struct usched_config_core *core);
int config_init_exec(struct usched_config_exec *exec);
int config_init_network(struct usched_config_network *network);
int config_init_stat(struct usched_config_stat *stat);
int config_init_users(struct usched_config_users *users);
void config_destroy_auth(struct usched_config_auth *auth);
void config_destroy_core(struct usched_config_core *core);
void config_destroy_exec(struct usched_config_exec *exec);
void config_destroy_network(struct usched_config_network *network);
void config_destroy_stat(struct usched_config_stat *stat);
void config_destroy_users(struct usched_config_users *users);
int config_admin_init(void);
void config_admin_destroy(void);
int config_client_init(void);
void config_client_destroy(void);
int config_daemon_init(void);
void config_daemon_destroy(void);
int config_exec_init(void);
void config_exec_destroy(void);
int config_ipc_init(void);
void config_ipc_destroy(void);
int config_stat_init(void);
void config_stat_destroy(void);

#endif

