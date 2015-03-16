#
# uConf: Compiler detection scripting library
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

if [ -e "`which clang`" ]; then
	echo "`which clang`" > .compiler
elif [ -e "`which gcc`" ]; then
	echo "`which gcc`" > .compiler
elif [ -e "`which cc`" ]; then
	echo "`which cc`" > .compiler
else
	echo "No suitable compiler found in current PATH."
	exit 1
fi

