#!/bin/sh
#
# @file usched.sh
# @brief uSched
#        uSched flush/start/stop script - Shell implementation
#
# Date: 22-03-2015
# 
# Copyright 2014-2015 Pedro A. Hortas (pah@ucodev.org)
#
# This file is part of usched.
#
# usched is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# usched is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with usched.  If not, see <http://www.gnu.org/licenses/>.
#

## Arguments data ##
ARGC=${#}
ARG0=${0}; ARG1=${1}; ARG2=${2};

## Config data ##
CONFIG_USCHED_QUIET=0
CONFIG_USCHED_ADMIN_BIN="@_SYSSBINDIR_@/usa"
CONFIG_USCHED_DAEMON_BIN="@_SYSSBINDIR_@/usd"
CONFIG_USCHED_EXEC_BIN="@_SYSSBINDIR_@/use"
CONFIG_USCHED_MONITOR_BIN="@_SYSSBINDIR_@/usm"
CONFIG_USCHED_DAEMON_PID_FILE="@_SYSRUNDIR_@/usched_usd.pid"
CONFIG_USCHED_EXEC_PID_FILE="@_SYSRUNDIR_@/usched_use.pid"

## Status ##
EXIT_SUCCESS=0
EXIT_FAILURE=1

## Implementation ##
print_info() {
	if [ ${CONFIG_USCHED_QUIET} -ne 1 ]; then
		printf "${1}"
	fi
}

usage() {
	print_info "Usage: ${ARG0} flush|reload|start|stop|force_stop [quiet]\n"
	exit ${EXIT_FAILURE}
}

args_check() {
	if [ ${ARGC} -lt 1 ]; then 
		usage
	elif [ ${ARGC} -eq 2 ]; then
		if [ ${ARG2} != "quiet" ]; then
			usage
		fi

		CONFIG_USCHED_QUIET=1
	elif [ ${ARGC} -gt 2 ]; then
		usage
	fi
}

perms_check() {
	if [ `id -u` -ne 0 ]; then
		print_info "You need to be root to perform that operation.\n"
		exit ${EXIT_FAILURE}
	fi
}

op_flush() {
	print_info "Flush operation status: "

	if [ -f "${CONFIG_USCHED_DAEMON_PID_FILE}" ]; then
		kill -USR1 `cat ${CONFIG_USCHED_DAEMON_PID_FILE}`
	else
		return 1
	fi

	return ${?}
}

op_reload() {
	print_info "Reload operation status: "

	if [ -f "${CONFIG_USCHED_DAEMON_PID_FILE}" ]; then
		kill -HUP `cat ${CONFIG_USCHED_DAEMON_PID_FILE}`
	else
		return 1
	fi

	return ${?}
}

op_start() {
	print_info "Start operation status: "

	${CONFIG_USCHED_ADMIN_BIN} commit core

	if [ ${?} -ne 0 ]; then
		ret=${?}
		print_info "Failed to commit uSched Core configuration: "
		return ${ret}
	fi

	${CONFIG_USCHED_MONITOR_BIN} -p ${CONFIG_USCHED_EXEC_PID_FILE} -r -S ${CONFIG_USCHED_EXEC_BIN}

	if [ ${?} -ne 0 ]; then
		return ${?}
	fi

	sleep 1

	while ! [ -f ${CONFIG_USCHED_EXEC_PID_FILE} ]; do sleep 1; done

	${CONFIG_USCHED_MONITOR_BIN} -p ${CONFIG_USCHED_DAEMON_PID_FILE} -r -S ${CONFIG_USCHED_DAEMON_BIN}

	sleep 1

	while ! [ -f ${CONFIG_USCHED_DAEMON_PID_FILE} ]; do sleep 1; done

	return ${?}
}

op_stop() {
	print_info "Stop operation status: "

	if [ -f "${CONFIG_USCHED_DAEMON_PID_FILE}" ]; then
		kill -TERM `cat ${CONFIG_USCHED_DAEMON_PID_FILE}`
	fi

	if [ -f "${CONFIG_USCHED_EXEC_PID_FILE}" ]; then
		kill -TERM `cat ${CONFIG_USCHED_EXEC_PID_FILE}`
	fi

	while [ -f ${CONFIG_USCHED_DAEMON_PID_FILE} ]; do sleep 1; done
	while [ -f ${CONFIG_USCHED_EXEC_PID_FILE} ]; do sleep 1; done

	return 0
}

op_force_stop() {
	print_info "Force stop operation status: "

	(kill -KILL `cat ${CONFIG_USCHED_DAEMON_PID_FILE}`) >& /dev/null
	(kill -KILL `cat ${CONFIG_USCHED_EXEC_PID_FILE}`) >& /dev/null

	rm -f ${CONFIG_USCHED_DAEMON_PID_FILE} >& /dev/null
	rm -f ${CONFIG_USCHED_EXEC_PID_FILE} >& /dev/null

	return 0
}

process_op() {
	case "${ARG1}" in
		flush)
			op_flush
			return ${?}
			;;
		reload)
			op_reload
			return ${?}
			;;
		start)
			op_start
			return ${?}
			;;
		stop)
			op_stop
			return ${?}
			;;
		force_stop)
			op_force_stop
			return ${?}
			;;
		*)
			echo "Invalid operation: ${ARG1}"
			usage
	esac
}

main() {
	args_check

	perms_check

	process_op

	status=${?}

	if [ ${status} -ne ${EXIT_SUCCESS} ]; then
		print_info "Failed.\n"
	else
		print_info "Success.\n"
	fi

	exit ${status}
}

## Entry point ##
main

