#!/bin/sh
#
# @file usched_preinit.sh
# @brief uSched
#        uSched pre initialization script
#
# Date: 14-05-2015
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

USCHED_USA_BIN="@_SYSSBINDIR_@/usa"
USCHED_IPCPWGEN_BIN="@_SYSSBINDIR_@/usched_ipcpwgen"

ipc_key_regen()
{
	MODULE=${1}

	# Get current IPC key
	ipc_key_last="`${USCHED_USA_BIN} show ${MODULE} auth key | cut -d' ' -f3`"

	# Change IPC key
	${USCHED_USA_BIN} change ${MODULE} auth key $(${USCHED_IPCPWGEN_BIN} 128) > /dev/null
	${USCHED_USA_BIN} commit ${MODULE}

	# Get new IPC key
	ipc_key_new="`${USCHED_USA_BIN} show ${MODULE} auth key | cut -d' ' -f3`"

	# Check if IPC key is valid
	if [ -z "${ipc_key_new}" ]; then
		echo "Fatal: IPC authentication key is empty"
		exit 1
	fi

	# Check if IPC key was changed
	if [ "${ipc_key_last}" = "${ipc_key_new}" ]; then
		echo "Fatal: IPC authentication key wasn't changed"
		exit 1
	fi
}

# Regen IPC keys
ipc_key_regen "ipc"

# All good
exit 0

