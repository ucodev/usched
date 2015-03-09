#!/bin/sh

printf "Analyzing uSched source code with scan-build...\n\n"

cd ../../

# Build uSched
printf " * Building uSched... "
./do > /dev/null

if [ ${?} -ne 0 ]; then
	echo "Failed to build uSched."
	exit 1
fi
echo "        OK"

printf " * Analyzing uSched source... "
# Rebuild uSched with scan-build
scan-build make > /dev/null

if [ ${?} -ne 0 ]; then
	echo "Failed."
	exit 1
fi
echo "OK"

# All good
printf "\nDone.\n"

