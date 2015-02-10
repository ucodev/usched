#!/bin/sh

## Detect compiler ##
. ./lib/sh/compiler.inc

## Detect architecture ##
. ./lib/sh/arch.inc

## Target options ##
if [ `uname` = "Linux" ]; then
	echo "-DCONFIG_SYS_LINUX=1 -D_GNU_SOURCE=1 -D_XOPEN_SOURCE=700" > .defines
elif [ `uname` = "Darwin" ]; then
	echo "-DCONFIG_SYS_BSD=1 -D_XOPEN_SOURCE=700" > .defines
elif [ `uname` = "FreeBSD" ]; then
	echo "-DCONFIG_SYS_BSD=1 -D_XOPEN_SOURCE=700" > .defines
elif [ `uname` = "OpenBSD" ]; then
	echo "-DCONFIG_SYS_BSD=1 -D_XOPEN_SOURCE=700" > .defines
elif [ `uname` = "SunOS" ]; then
	echo "-DCONFIG_SYS_SOLARIS=1 -D_XOPEN_SOURCE=700" > .defines
fi

## Test features ##
mkdir -p build
rm -f .l*
. ./lib/sh/test.inc

test_lib "crypt"
test_lib "gmp"

## Build ##
make

if [ $? -ne 0 ]; then
	echo "Build failed."
	exit 1
fi

touch .done

echo "Build completed."

exit 0

