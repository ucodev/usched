#
# uConf: Variable substitution shell scripting library
#
# Copyright 2014-2015 Pedro A. Hortas (pah@ucodev.org)
#
# Date: 27/03/2015
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

if [ -e uconf/params/default/varsubst ]; then
	for varsubst in `cat uconf/params/default/varsubst`; do
		if [ ! -e ${varsubst}.orig ]; then
			cp ${varsubst} ${varsubst}.orig
		fi

		sed 's/@_SYSBINDIR_@/'$(cat .dirbin | tr -d '\n' | sed 's/\//\\\//g')'/g' ${varsubst} > ${varsubst}.bak 2> /dev/null
		mv -f ${varsubst}.bak ${varsubst}

		sed 's/@_SYSCACHEDIR_@/'$(cat .dircache | tr -d '\n' | sed 's/\//\\\//g')'/g' ${varsubst} > ${varsubst}.bak 2> /dev/null
		mv -f ${varsubst}.bak ${varsubst}

		sed 's/@_SYSCONFDIR_@/'$(cat .dirconf | tr -d '\n' | sed 's/\//\\\//g')'/g' ${varsubst} > ${varsubst}.bak 2> /dev/null
		mv -f ${varsubst}.bak ${varsubst}

		sed 's/@_SYSDOCDIR_@/'$(cat .dirdoc | tr -d '\n' | sed 's/\//\\\//g')'/g' ${varsubst} > ${varsubst}.bak 2> /dev/null
		mv -f ${varsubst}.bak ${varsubst}

		sed 's/@_SYSINCLUDEDIR_@/'$(cat .dirinclude | tr -d '\n' | sed 's/\//\\\//g')'/g' ${varsubst} > ${varsubst}.bak 2> /dev/null
		mv -f ${varsubst}.bak ${varsubst}

		sed 's/@_SYSLIBDIR_@/'$(cat .dirlib | tr -d '\n' | sed 's/\//\\\//g')'/g' ${varsubst} > ${varsubst}.bak 2> /dev/null
		mv -f ${varsubst}.bak ${varsubst}

		sed 's/@_SYSMANDIR_@/'$(cat .dirman | tr -d '\n' | sed 's/\//\\\//g')'/g' ${varsubst} > ${varsubst}.bak 2> /dev/null
		mv -f ${varsubst}.bak ${varsubst}

		sed 's/@_SYSRUNDIR_@/'$(cat .dirrun | tr -d '\n' | sed 's/\//\\\//g')'/g' ${varsubst} > ${varsubst}.bak 2> /dev/null
		mv -f ${varsubst}.bak ${varsubst}

		sed 's/@_SYSSBINDIR_@/'$(cat .dirsbin | tr -d '\n' | sed 's/\//\\\//g')'/g' ${varsubst} > ${varsubst}.bak 2> /dev/null
		mv -f ${varsubst}.bak ${varsubst}

		sed 's/@_SYSSHAREDIR_@/'$(cat .dirshare | tr -d '\n' | sed 's/\//\\\//g')'/g' ${varsubst} > ${varsubst}.bak 2> /dev/null
		mv -f ${varsubst}.bak ${varsubst}

		sed 's/@_SYSTMPDIR_@/'$(cat .dirtmp | tr -d '\n' | sed 's/\//\\\//g')'/g' ${varsubst} > ${varsubst}.bak 2> /dev/null
		mv -f ${varsubst}.bak ${varsubst}

		rm -f ${varsubst}.bak
	done

fi

if [ -e uconf/params/`cat .osname`/varsubst ]; then
	for varsubst in `cat uconf/params/$(cat .osname)/varsubst`; do
		if [ ! -e ${varsubst}.orig ]; then
			cp ${varsubst} ${varsubst}.orig
		fi

		sed 's/@_SYSBINDIR_@/'$(cat .dirbin | tr -d '\n' | sed 's/\//\\\//g')'/g' ${varsubst} > ${varsubst}.bak 2> /dev/null
		mv -f ${varsubst}.bak ${varsubst}

		sed 's/@_SYSCACHEDIR_@/'$(cat .dircache | tr -d '\n' | sed 's/\//\\\//g')'/g' ${varsubst} > ${varsubst}.bak 2> /dev/null
		mv -f ${varsubst}.bak ${varsubst}

		sed 's/@_SYSCONFDIR_@/'$(cat .dirconf | tr -d '\n' | sed 's/\//\\\//g')'/g' ${varsubst} > ${varsubst}.bak 2> /dev/null
		mv -f ${varsubst}.bak ${varsubst}

		sed 's/@_SYSDOCDIR_@/'$(cat .dirdoc | tr -d '\n' | sed 's/\//\\\//g')'/g' ${varsubst} > ${varsubst}.bak 2> /dev/null
		mv -f ${varsubst}.bak ${varsubst}

		sed 's/@_SYSINCLUDEDIR_@/'$(cat .dirinclude | tr -d '\n' | sed 's/\//\\\//g')'/g' ${varsubst} > ${varsubst}.bak 2> /dev/null
		mv -f ${varsubst}.bak ${varsubst}

		sed 's/@_SYSLIBDIR_@/'$(cat .dirlib | tr -d '\n' | sed 's/\//\\\//g')'/g' ${varsubst} > ${varsubst}.bak 2> /dev/null
		mv -f ${varsubst}.bak ${varsubst}

		sed 's/@_SYSMANDIR_@/'$(cat .dirman | tr -d '\n' | sed 's/\//\\\//g')'/g' ${varsubst} > ${varsubst}.bak 2> /dev/null
		mv -f ${varsubst}.bak ${varsubst}

		sed 's/@_SYSRUNDIR_@/'$(cat .dirrun | tr -d '\n' | sed 's/\//\\\//g')'/g' ${varsubst} > ${varsubst}.bak 2> /dev/null
		mv -f ${varsubst}.bak ${varsubst}

		sed 's/@_SYSSBINDIR_@/'$(cat .dirsbin | tr -d '\n' | sed 's/\//\\\//g')'/g' ${varsubst} > ${varsubst}.bak 2> /dev/null
		mv -f ${varsubst}.bak ${varsubst}

		sed 's/@_SYSSHAREDIR_@/'$(cat .dirshare | tr -d '\n' | sed 's/\//\\\//g')'/g' ${varsubst} > ${varsubst}.bak 2> /dev/null
		mv -f ${varsubst}.bak ${varsubst}

		sed 's/@_SYSTMPDIR_@/'$(cat .dirtmp | tr -d '\n' | sed 's/\//\\\//g')'/g' ${varsubst} > ${varsubst}.bak 2> /dev/null
		mv -f ${varsubst}.bak ${varsubst}

		rm -f ${varsubst}.bak
	done
fi

if [ -e uconf/params/user/varsubst ]; then
	for varsubst in `cat uconf/params/user/varsubst`; do
		if [ ! -e ${varsubst}.orig ]; then
			cp ${varsubst} ${varsubst}.orig
		fi

		sed 's/@_SYSBINDIR_@/'$(cat .dirbin | tr -d '\n' | sed 's/\//\\\//g')'/g' ${varsubst} > ${varsubst}.bak 2> /dev/null
		mv -f ${varsubst}.bak ${varsubst}

		sed 's/@_SYSCACHEDIR_@/'$(cat .dircache | tr -d '\n' | sed 's/\//\\\//g')'/g' ${varsubst} > ${varsubst}.bak 2> /dev/null
		mv -f ${varsubst}.bak ${varsubst}

		sed 's/@_SYSCONFDIR_@/'$(cat .dirconf | tr -d '\n' | sed 's/\//\\\//g')'/g' ${varsubst} > ${varsubst}.bak 2> /dev/null
		mv -f ${varsubst}.bak ${varsubst}

		sed 's/@_SYSDOCDIR_@/'$(cat .dirdoc | tr -d '\n' | sed 's/\//\\\//g')'/g' ${varsubst} > ${varsubst}.bak 2> /dev/null
		mv -f ${varsubst}.bak ${varsubst}

		sed 's/@_SYSINCLUDEDIR_@/'$(cat .dirinclude | tr -d '\n' | sed 's/\//\\\//g')'/g' ${varsubst} > ${varsubst}.bak 2> /dev/null
		mv -f ${varsubst}.bak ${varsubst}

		sed 's/@_SYSLIBDIR_@/'$(cat .dirlib | tr -d '\n' | sed 's/\//\\\//g')'/g' ${varsubst} > ${varsubst}.bak 2> /dev/null
		mv -f ${varsubst}.bak ${varsubst}

		sed 's/@_SYSMANDIR_@/'$(cat .dirman | tr -d '\n' | sed 's/\//\\\//g')'/g' ${varsubst} > ${varsubst}.bak 2> /dev/null
		mv -f ${varsubst}.bak ${varsubst}

		sed 's/@_SYSRUNDIR_@/'$(cat .dirrun | tr -d '\n' | sed 's/\//\\\//g')'/g' ${varsubst} > ${varsubst}.bak 2> /dev/null
		mv -f ${varsubst}.bak ${varsubst}

		sed 's/@_SYSSBINDIR_@/'$(cat .dirsbin | tr -d '\n' | sed 's/\//\\\//g')'/g' ${varsubst} > ${varsubst}.bak 2> /dev/null
		mv -f ${varsubst}.bak ${varsubst}

		sed 's/@_SYSSHAREDIR_@/'$(cat .dirshare | tr -d '\n' | sed 's/\//\\\//g')'/g' ${varsubst} > ${varsubst}.bak 2> /dev/null
		mv -f ${varsubst}.bak ${varsubst}

		sed 's/@_SYSTMPDIR_@/'$(cat .dirtmp | tr -d '\n' | sed 's/\//\\\//g')'/g' ${varsubst} > ${varsubst}.bak 2> /dev/null
		mv -f ${varsubst}.bak ${varsubst}

		rm -f ${varsubst}.bak
	done
fi

