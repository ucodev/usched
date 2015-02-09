#!/bin/sh

## Detect compiler ##
. ./lib/sh/compiler.inc

## Detect architecture ##
. ./lib/sh/arch.inc

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

