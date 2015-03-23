#
# uConf: Optional features testing shell scripting library
#
# Copyright 2014-2015 Pedro A. Hortas (pah@ucodev.org)
#
# Date: 23/03/2015
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

test_opt_lib()
{
	printf -- " * Checking for lib${lib}... "

	# test lib support
	echo 'int main(void) { return 0; }' > build/${1}.c && `cat .compiler` -o build/${1} build/${1}.c -l${1} 2> /dev/null

	if [ $? -eq 0 ]; then
		touch .l${1}
		printf -- "-l${1} "  >> .libs
		echo "OK"
		rm -f build/${1}
		rm -f build/${1}.c
	else
		echo "Not supported"
		rm -f build/${1}.c
	fi
}

mkdir -p build
rm -f .l*

if [ -e ./uconf/params/default/optlibs ]; then
	echo "Testing default optional libraries..."

	for lib in `cat ./uconf/params/default/optlibs`; do
		test_opt_lib ${lib}
	done

	echo "Done."
	echo ""
fi

if [ -e ./uconf/params/`cat .osname`/optlibs ]; then
	echo "Testing `cat .osname` optional libraries..."

	for lib in $(cat ./uconf/params/`cat .osname`/optlibs); do
		test_opt_lib ${lib}
	done

	echo "Done."
	echo ""
fi

if [ -e ./uconf/params/user/optlibs ]; then
	echo "Testing user-defined optional libraries..."

	for lib in $(cat ./uconf/params/user/optlibs); do
		test_opt_lib ${lib}
	done

	echo "Done."
	echo ""
fi


touch .libs

