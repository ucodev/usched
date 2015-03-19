#!/bin/sh

# Globals
SPLINT_OPTS="-posix-lib -weak -redef -fcnuse -unrecog"
SPLINT_OPTS_EXTRA="+ignorequals -duplicatequals"
SPLINT_OUT=".splint_out"

DEFINES="-D_POSIX_C_SOURCE=199309L -D_REENTRANT -Dtimer_t=unsigned -Dclockid_t=unsigned -DCONFIG_USE_IPC_PMQ=1"
INCLUDES="-I../../include"

printf "Analyzing uSched source code with splint...\n\n"

# Analyze src/common
printf " * Checking src/common... "
for file in ../../src/common/*.c; do
	splint ${INCLUDES} ${DEFINES} -DCONFIG_COMMON=1 ${SPLINT_OPTS} ${file} 2> ${SPLINT_OUT}

	if [ ${?} -ne 0 ]; then
		cat ${SPLINT_OUT}
		echo "File ${file} failed splint check."
		rm -f ${SPLINT_OUT}
		exit 1
	fi
done
echo "OK"

# Analyze src/usa
printf " * Checking src/usa... "
for file in ../../src/usa/*.c; do
	splint ${INCLUDES} ${DEFINES} -DCONFIG_ADMIN_SPECIFIC=1 ${SPLINT_OPTS} ${file} 2> ${SPLINT_OUT}

	if [ ${?} -ne 0 ]; then
		cat ${SPLINT_OUT}
		echo "File ${file} failed splint check."
		rm -f ${SPLINT_OUT}
		exit 1
	fi
done
echo "   OK"

# Analyze src/usc
printf " * Checking src/usc... "
for file in ../../src/usc/*.c; do
	splint ${INCLUDES} ${DEFINES} -DCONFIG_CLIENT_SPECIFIC=1 ${SPLINT_OPTS} ${SPLINT_OPTS_EXTRA} ${file} 2> ${SPLINT_OUT}

	if [ ${?} -ne 0 ]; then
		cat ${SPLINT_OUT}
		echo "File ${file} failed splint check."
		rm -f ${SPLINT_OUT}
		exit 1
	fi
done
echo "   OK"

# Analyze src/usd
printf " * Checking src/usd... "
for file in ../../src/usd/*.c; do
	splint ${INCLUDES} ${DEFINES} -DCONFIG_DAEMON_SPECIFIC=1 ${SPLINT_OPTS} ${SPLINT_OPTS_EXTRA} ${file} 2> ${SPLINT_OUT}

	if [ ${?} -ne 0 ]; then
		cat ${SPLINT_OUT}
		echo "File ${file} failed splint check."
		rm -f ${SPLINT_OUT}
		exit 1
	fi
done
echo "   OK"

# Analyze src/use
printf " * Checking src/use... "
for file in ../../src/use/*.c; do
	splint ${INCLUDES} ${DEFINES} -DCONFIG_EXEC_SPECIFIC=1 ${SPLINT_OPTS} ${SPLINT_OPTS_EXTRA} ${file} 2> ${SPLINT_OUT}

	if [ ${?} -ne 0 ]; then
		cat ${SPLINT_OUT}
		echo "File ${file} failed splint check."
		rm -f ${SPLINT_OUT}
		exit 1
	fi
done
echo "   OK"

# Analyze src/usm
printf " * Checking src/usm... "
for file in ../../src/usm/*.c; do
	splint ${INCLUDES} ${DEFINES} ${SPLINT_OPTS} ${SPLINT_OPTS_EXTRA} ${file} 2> ${SPLINT_OUT}

	if [ ${?} -ne 0 ]; then
		cat ${SPLINT_OUT}
		echo "File ${file} failed splint check."
		rm -f ${SPLINT_OUT}
		exit 1
	fi
done
echo "   OK"

printf "\nDone.\n"

# Unlink output file
rm -f ${SPLINT_OUT}

# All good
exit 0
