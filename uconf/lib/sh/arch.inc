#
# uConf: Architecture detection scripting library
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


