#!/bin/sh

# Check deps
if ! [ -f `which scan-build` ]; then
	echo "scan-build not found."
	exit 1
fi

if ! [ -f `which pscan` ]; then
	echo "pscan not found."
	exit 1
fi

if ! [ -f `which splint` ]; then
	echo "splint not found."
	exit 1
fi


# Do tests
./test_scan-build.sh

if [ ${?} -ne 0 ]; then
	exit 1;
fi

echo ""

./test_pscan.sh

if [ ${?} -ne 0 ]; then
	exit 1;
fi

echo ""

./test_splint.sh

if [ ${?} -ne 0 ]; then
	exit 1;
fi

# All good
exit 0
