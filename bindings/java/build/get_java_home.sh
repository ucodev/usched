#!/bin/sh

JAVA_HOME_TRY=$(readlink -f `which java`)

if [ ! -d "${JAVA_HOME_TRY:0:-12}/include" ]; then
	echo "Cannot determine JAVA_HOME."
	exit 1
fi

echo ${JAVA_HOME_TRY:0:-12}

exit 0
