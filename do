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

make

if [ $? -ne 0 ]; then
	echo "Build failed."
	exit 1
fi

touch .done

echo "Build completed."

exit 0

