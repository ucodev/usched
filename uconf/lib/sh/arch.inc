#
# Architecture detection scripting library
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

touch .target

if [ `uname -m` = "armv6l" ]; then
	if [ "`cat .target`" = "rpi" ]; then
		if [ "`cat .compiler`" = "`which clang`" ]; then
			echo "-ccc-host-triple armv6-unknown-eabi -march=armv6 -mfpu=vfp -mcpu=arm1176jzf-s -mtune=arm1176jzf-s -mfloat-abi=hard" > .archflags
		else
			echo "-march=armv6zk -mfpu=vfp -mcpu=arm1176jzf-s -mtune=arm1176jzf-s -mfloat-abi=hard" > .archflags
		fi
	else
		echo "-march=armv6" > .archflags
	fi
elif [ "`cat .target`" = "bbb" ]; then
	echo "-march=armv7-a -mtune=cortex-a8 -mfloat-abi=hard -mfpu=neon" > .archflags
else
	echo "" > .archflags
fi


