#!/bin/sh

printf "Analyzing uSched source code with splint...\n\n"

printf " * Checking src/common... "
for file in ../src/common/*.c; do
	splint -I../include -DCONFIG_COMMON=1 -D_POSIX_C_SOURCE=199309L -D_REENTRANT -Dtimer_t=unsigned -posix-lib -weak -redef -fcnuse -unrecog ${file} 2> .splint_out

	if [ ${?} -ne 0 ]; then
		cat .splint_out
		echo "File ${file} failed splint check."
		rm -f .splint_out
		exit 1
	fi
done
echo "OK"

printf " * Checking src/usa... "
for file in ../src/usa/*.c; do
	splint -I../include -DCONFIG_ADMIN_SPECIFIC=1 -D_POSIX_C_SOURCE=199309L -D_REENTRANT -Dtimer_t=unsigned -posix-lib -weak -redef -fcnuse -unrecog ${file} 2> .splint_out

	if [ ${?} -ne 0 ]; then
		cat .splint_out
		echo "File ${file} failed splint check."
		rm -f .splint_out
		exit 1
	fi
done
echo "   OK"

printf " * Checking src/usc... "
for file in ../src/usc/*.c; do
	splint -I../include -DCONFIG_CLIENT_SPECIFIC=1 -D_POSIX_C_SOURCE=199309L -D_REENTRANT -Dtimer_t=unsigned -posix-lib -weak -redef -fcnuse -unrecog +ignorequals -duplicatequals ${file} 2> .splint_out

	if [ ${?} -ne 0 ]; then
		cat .splint_out
		echo "File ${file} failed splint check."
		rm -f .splint_out
		exit 1
	fi
done
echo "   OK"

printf " * Checking src/usd... "
for file in ../src/usd/*.c; do
	splint -I../include -DCONFIG_DAEMON_SPECIFIC=1 -D_POSIX_C_SOURCE=199309L -D_REENTRANT -Dtimer_t=unsigned -posix-lib -weak -redef -fcnuse -unrecog +ignorequals -duplicatequals ${file} 2> .splint_out

	if [ ${?} -ne 0 ]; then
		cat .splint_out
		echo "File ${file} failed splint check."
		rm -f .splint_out
		exit 1
	fi
done
echo "   OK"

printf " * Checking src/use... "
for file in ../src/use/*.c; do
	splint -I../include -DCONFIG_EXEC_SPECIFIC=1 -D_POSIX_C_SOURCE=199309L -D_REENTRANT -Dtimer_t=unsigned -posix-lib -weak -redef -fcnuse -unrecog +ignorequals -duplicatequals ${file} 2> .splint_out

	if [ ${?} -ne 0 ]; then
		cat .splint_out
		echo "File ${file} failed splint check."
		rm -f .splint_out
		exit 1
	fi
done
echo "   OK"

printf " * Checking src/usm... "
for file in ../src/usm/*.c; do
	splint -I../include -D_POSIX_C_SOURCE=199309L -D_REENTRANT -Dtimer_t=unsigned -posix-lib -weak -redef -fcnuse -unrecog +ignorequals -duplicatequals ${file} 2> .splint_out

	if [ ${?} -ne 0 ]; then
		cat .splint_out
		echo "File ${file} failed splint check."
		rm -f .splint_out
		exit 1
	fi
done
echo "   OK"

printf "\nDone.\n"

rm -f .splint_out

