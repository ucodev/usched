CC=`cat ../../.compiler`
ECFLAGS=-DCONFIG_SYS_LINUX=1 -D_GNU_SOURCE=1
CCFLAGS=-fPIC -DUSE_LIBFSMA -D_POSIX_C_SOURCE=199309L -D_XOPEN_SOURCE -D_REENTRANT -std=c11 -fstrict-aliasing -pedantic -pthread -Wall -Werror -g
LDFLAGS=-g -pthread -lfsma -lrt
INCLUDEDIRS=-I../../include
OBJS=../common/bitops.o ../common/debug.o exec.o ../common/log.o ../common/mm.o ../common/pmq.o ../common/runtime.o runtime.o ../common/sig.o ../common/thread.o
TARGET=use

all:
	${CC} ${ECFLAGS} ${CCFLAGS} ${INCLUDEDIRS} -c exec.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${INCLUDEDIRS} -c runtime.c
	${CC} -o ${TARGET} ${OBJS} ${LDFLAGS}

install:
	cp use /usr/sbin/

clean:
	rm -f ${OBJS}
	rm -f ${TARGET}

