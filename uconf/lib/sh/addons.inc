#
# uconf: Add-ons options scripting library
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
	echo 'int main(void) { return 0; }' > build/addons-lib-test.c && `cat .compiler` -o build/addons-lib-test build/addons-lib-test.c ${libname} 2> /dev/null

	if [ $? -eq 0 ]; then
		echo "OK"
		rm -f build/addons-lib-test
		rm -f build/addons-lib-test.c
	else
		echo "Failed"
		rm -f build/addons-lib-test.c
		exit 1
	fi
}

## Addons options ##
if [ -e .addons ]; then
	echo "Testing add-ons support..."

	while read addon; do
		for lib in $(cat ./uconf/addons/${addon}/ldflags); do
			check_lib ${lib}
		done

		printf -- " " >> .ecflags
		if [ -e "./uconf/addons/${addon}/ccflags" ]; then
			cat ./uconf/addons/${addon}/ccflags | tr -d '\n' >> .ecflags
		fi

		printf -- " " >> .elflags
		if [ -e "./uconf/addons/${addon}/ldflags" ]; then
			cat ./uconf/addons/${addon}/ldflags | tr -d '\n' >> .elflags
		fi
	done < .addons

	echo "Done."
	echo ""
fi


