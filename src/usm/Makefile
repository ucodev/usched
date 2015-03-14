CC=`cat ../../.compiler`
ECFLAGS=`cat ../../.ecflags`
CCFLAGS=-std=c99 -D_POSIX_C_SOURCE=199309L -D_REENTRANT -fstrict-aliasing -fstack-protector-all -pedantic -Wall -Werror -Wstack-protector -g
LDFLAGS=-g -lfsma
ELFLAGS=`cat ../../.elflags`
ARCHFLAGS=`cat ../../.archflags`
INCLUDEDIRS=-I../../include
OBJS_COMMON=../common/log.o ../common/debug.o ../common/mm.o ../common/str.o
OBJS=monitor.o
TARGET=usm

all:
	${CC} ${ECFLAGS} ${CCFLAGS} ${ARCHFLAGS} ${INCLUDEDIRS} -c monitor.c
	${CC} -o ${TARGET} ${OBJS} ${OBJS_COMMON} ${LDFLAGS} ${ELFLAGS}

install:
	cp ${TARGET} /usr/sbin/

clean:
	rm -f ${OBJS}
	rm -f ${TARGET}

