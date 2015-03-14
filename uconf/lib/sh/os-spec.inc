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
	libname=${1}

	printf -- " * Checking support for ${1}... "

	# Check if build directory exists
	if [ ! -e ./build ]; then
		mkdir build
	fi

	# check if lib is installed
	echo 'int main(void) { return 0; }' > build/os-spec-lib-test.c && `cat .compiler` -o build/os-spec-lib-test build/os-spec-lib-test.c ${libname} 2> /dev/null

	if [ $? -eq 0 ]; then
		echo "OK"
		rm -f build/os-spec-lib-test
		rm -f build/os-spec-lib-test.c
	else
		echo "Failed"
		rm -f build/os-spec-lib-test.c
		exit 1
	fi
}

## Default options ##
cat ./uconf/params/default/ccflags | tr -d '\n' > .ecflags
printf -- " " >> .ecflags
cat ./uconf/params/default/ldflags | tr -d '\n' > .elflags
printf -- " " >> .elflags

## OS-Specific options ##
if [ -e "./uconf/params/`uname`/ccflags" ]; then
	echo "Target operating system is: `uname`. Configuring..."
	echo `uname` > .osname

	for lib in $(cat ./uconf/params/`uname`/ldflags); do
		check_lib ${lib}
	done

	cat ./uconf/params/`uname`/ccflags | tr -d '\n' >> .ecflags
	cat ./uconf/params/`uname`/ldflags | tr -d '\n' >> .elflags

	echo "Done."
	echo ""
else
	echo "Target operating system is: `uname` (Not supported). Configuring..."
	echo "unknown" > .osname

	for lib in $(cat ./uconf/params/`uname`/ldflags); do
		check_lib ${lib}
	done

	cat ./uconf/params/unknown/ccflags | tr -d '\n' >> .ecflags
	cat ./uconf/params/unknown/ldflags | tr -d '\n' >> .elflags

	echo "Done."
	echo ""
fi

