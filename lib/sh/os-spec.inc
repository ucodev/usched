#
# Compiler detection scripting library
#
# Copyright 2014-2015 Pedro A. Hortas (pah@ucodev.org)
#
#
# Double licensed under Apache License Version 2.0:
#
#
# Licensed to the Apache Software Foundation (ASF) under one
# or more contributor license agreements.  See the NOTICE file
# distributed with this work for additional information
# regarding copyright ownership.  The ASF licenses this file
# to you under the Apache License, Version 2.0 (the
# "License"); you may not use this file except in compliance
# with the License.  You may obtain a copy of the License at
#
#  http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing,
# software distributed under the License is distributed on an
# "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
# KIND, either express or implied.  See the License for the
# specific language governing permissions and limitations
# under the License.
#
#
# or under GNU General Public License Version 3:
#
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

## OS-Specific options ##
if [ `uname` = "Linux" ]; then
	echo "-DCONFIG_SYS_LINUX=1 -D_GNU_SOURCE=1 -D_XOPEN_SOURCE=700" > .ecflags
	echo "-pthread -lrt " > .elflags
elif [ `uname` = "Darwin" ]; then
	echo "-DCONFIG_SYS_BSD=1 -D_BSD_SOURCE=1 -D__BSD_VISIBLE=1 -D_XOPEN_SOURCE=700" > .ecflags
	echo "-pthread -lrt " > .elflags
elif [ `uname` = "FreeBSD" ]; then
	echo "-DCONFIG_SYS_BSD=1 -D_BSD_SOURCE=1 -D__BSD_VISIBLE=1 -D_XOPEN_SOURCE=700" > .ecflags
	echo "-pthread -lrt " > .elflags
elif [ `uname` = "OpenBSD" ]; then
	echo "-DCONFIG_SYS_BSD=1 -D_BSD_SOURCE=1 -D__BSD_VISIBLE=1 -D_XOPEN_SOURCE=700 -DPSCHED_INTERNAL_SIGEVENT=1 -DPSCHED_INTERNAL_TIMER_UL=1" > .ecflags
	echo "-pthread -lrt " > .elflags
elif [ `uname` = "Minix" ]; then
	echo "-I/usr/pkg/include -DCONFIG_SYS_MINIX=1 -D_XOPEN_SOURCE=700 -DPSCHED_INTERNAL_TIMER_UL=1" > .ecflags
	echo "/usr/pkg/lib/libpthread.so " > .elflags
elif [ `uname` = "SunOS" ]; then
	echo "-DCONFIG_SYS_SOLARIS=1 -D_XOPEN_SOURCE=700" > .ecflags
	echo "-pthread -lrt " > .elflags
else
	echo "-DCONFIG_SYS_UNKNOWN=1 -D_XOPEN_SOURCE=700" > .ecflags
	echo "-lpthread -lrt " > .elflags
fi

