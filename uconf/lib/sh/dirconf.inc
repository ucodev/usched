#
# uConf: System directories configuration shell scripting library
#
# Copyright 2014-2015 Pedro A. Hortas (pah@ucodev.org)
#
# Date: 22/03/2015
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

## Copy directory configuration files from default parameters
cd uconf/params/default/dir

for dirfile in *; do
	cp ${dirfile} ../../../../.dir${dirfile}
done

cd ../../../../

## If there are any non-default directory configurations, copy them (and replace the default ones)
if [ -e uconf/params/`cat .osname`/dir ]; then
	cd uconf/params/`cat .osname`/dir

	for dirfile in *; do
		cp ${dirfile} ../../../../.dir${dirfile}
	done

	cd ../../../../
fi

