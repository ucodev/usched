#!/bin/sh

echo "Performing security checks..."
echo ""

# Test PMQ
printf " * Checking message queues... "

./check_pmq 2> /dev/null

if [ ${?} -ne 0 ]; then
	echo "Failed"
	exit 1;
fi


# Test Config
printf " * Checking config perms...   "

./check_config 2> /dev/null

if [ ${?} -ne 0 ]; then
	echo "Failed"
	exit 1;
fi


# Test cache
printf " * Checking cache perms...    "

./check_cache 2> /dev/null

if [ ${?} -ne 0 ]; then
	echo "Failed"
	exit 1;
fi


# All good
echo ""
echo "Security checks done."

exit 0

