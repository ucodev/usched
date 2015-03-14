#
# Operating System specific options scripting library
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

## Test functions ##
check_lib()
{
	printf -- " * Checking support for ${1}... "
	# remove '-' prefix
	libname=`echo ${1} | cut -d'-' -f2`

	# Check if build directory exists
	if [ ! -e ./build ]; then
		mkdir build
	fi

	# check if lib is installed
	echo 'int main(void) { return 0; }' > build/${libname}.c && `cat .compiler` -o build/${libname} build/${libname}.c -${libname}

	if [ $? -eq 0 ]; then
		echo "OK"
	else
		echo "Failed"
		exit 1
	fi
}

## Default options ##
cat ./uconf/default/ccflags | tr -d '\n' > .ecflags
printf -- " " >> .ecflags
cat ./uconf/default/ldflags | tr -d '\n' > .elflags
printf -- " " >> .elflags

## OS-Specific options ##
if [ -e "./uconf/`uname`/ccflags" ]; then
	echo "Target operating system is: `uname`. Configuring..."

	for lib in $(cat ./uconf/`uname`/ldflags); do
		check_lib ${lib}
	done

	echo "Done."
	echo ""

	cat ./uconf/`uname`/ccflags | tr -d '\n' >> .ecflags
	cat ./uconf/`uname`/ldflags | tr -d '\n' >> .elflags
else
	cat ./uconf/unknown/ccflags | tr -d '\n' >> .ecflags
	cat ./uconf/unknown/ldflags | tr -d '\n' >> .elflags
fi

