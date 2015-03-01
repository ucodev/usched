#!/bin/sh

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

exit 0
