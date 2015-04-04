#!/bin/sh

# Globals
PSCAN_OUT=.pscan_out

printf "Analyzing uSched source code with pscan...\n\n"

# Analyze src/common
printf " * Checking src/common... "
pscan -p usched.pscan ../../src/common/*.c 2> ${PSCAN_OUT}
if [ ${?} -ne 0 ]; then
	echo "Failed."
	cat ${PSCAN_OUT}
	rm -f ${PSCAN_OUT}
	exit 1
fi
echo "OK"

# Analyze src/usa
printf " * Checking src/usa... "
pscan -p usched.pscan ../../src/usa/*.c 2> ${PSCAN_OUT}
if [ ${?} -ne 0 ]; then
	echo "Failed."
	cat ${PSCAN_OUT}
	rm -f ${PSCAN_OUT}
	exit 1
fi
echo "   OK"

# Analyze src/usc
printf " * Checking src/usc... "
pscan -p usched.pscan ../../src/usc/*.c 2> ${PSCAN_OUT}
if [ ${?} -ne 0 ]; then
	echo "Failed."
	cat ${PSCAN_OUT}
	rm -f ${PSCAN_OUT}
	exit 1
fi
echo "   OK"

# Analyze src/usd
printf " * Checking src/usd... "
pscan -p usched.pscan ../../src/usd/*.c 2> ${PSCAN_OUT}
if [ ${?} -ne 0 ]; then
	echo "Failed."
	cat ${PSCAN_OUT}
	rm -f ${PSCAN_OUT}
	exit 1
fi
echo "   OK"

# Analyze src/use
printf " * Checking src/use... "
pscan -p usched.pscan ../../src/use/*.c 2> ${PSCAN_OUT}
if [ ${?} -ne 0 ]; then
	echo "Failed."
	cat ${PSCAN_OUT}
	rm -f ${PSCAN_OUT}
	exit 1
fi
echo "   OK"

# Analyze src/usm
printf " * Checking src/usm... "
pscan -p usched.pscan ../../src/usm/*.c 2> ${PSCAN_OUT}
if [ ${?} -ne 0 ]; then
	echo "Failed."
	cat ${PSCAN_OUT}
	rm -f ${PSCAN_OUT}
	exit 1
fi
echo "   OK"

# Analyze src/uss
printf " * Checking src/uss... "
pscan -p usched.pscan ../../src/uss/*.c 2> ${PSCAN_OUT}
if [ ${?} -ne 0 ]; then
	echo "Failed."
	cat ${PSCAN_OUT}
	rm -f ${PSCAN_OUT}
	exit 1
fi
echo "   OK"

# We're done
printf "\nDone.\n"

# Unlink output file
rm -f ${PSCAN_OUT}

# ALl good
exit 0

