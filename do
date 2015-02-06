#!/bin/bash

if [ -e "/usr/bin/clang" ]; then
	echo "/usr/bin/clang" > .compiler
elif [ -e "/usr/bin/gcc" ]; then
	echo "/usr/bin/gcc" > .compiler
elif [ -e "/usr/bin/cc" ]; then
	echo "/usr/bin/cc" > .compiler
else
	echo "No suitable compiler found."
	exit 1
fi

if [ `uname -m` = "armv6l" ]; then
	if [ "`cat .target`" == "rpi" ]; then
		if [ "`cat .compiler`" == "/usr/bin/clang" ]; then
			echo "-ccc-host-triple armv6-unknown-eabi -march=armv6 -mfpu=vfp -mcpu=arm1176jzf-s -mtune=arm1176jzf-s -mfloat-abi=hard" > .archflags
		else
			echo "-march=armv6zk -mfpu=vfp -mcpu=arm1176jzf-s -mtune=arm1176jzf-s -mfloat-abi=hard" > .archflags
		fi
	else
		echo "-march=armv6" > .archflags
	fi
else
	echo "" > .archflags
fi

## Test features ##
mkdir -p build
rm -f .l*
. ./lib/sh/test.inc

test_lib "crypt"
test_lib "gmp"

make

if [ $? -ne 0 ]; then
	echo "Build failed."
	exit 1
fi

touch .done

echo "Build completed."

exit 0

