#
# uConf: Operating System specific options scripting library
#
# Copyright 2014-2015 Pedro A. Hortas (pah@ucodev.org)
#
# Date: 16/03/2015
#
# URL: http://www.ucodev.org
#
# This file is part of uConf.
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

if [ -e "./uconf/params/`uname`" ]; then
	echo `uname` > .osname
else
	echo "unsupported" > .osname
fi

## OS-Specific options ##
echo "Target operating system is: `cat .osname`. Configuring..."

for lib in $(cat ./uconf/params/`cat .osname`/ldflags); do
	check_lib ${lib}
done

if [ -e "./uconf/params/`cat .osname`/ccflags" ]; then
	cat ./uconf/params/`cat .osname`/ccflags | tr -d '\n' >> .ecflags
fi

if [ -e "./uconf/params/`cat .osname`/ldflags" ]; then
	cat ./uconf/params/`cat .osname`/ldflags | tr -d '\n' >> .elflags
fi

cp ./uconf/params/`cat .osname`/extlib .extlib

echo "Done."
echo ""

