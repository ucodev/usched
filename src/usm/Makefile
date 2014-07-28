CC=/usr/bin/clang
ECFLAGS=-DCONFIG_SYS_LINUX=1 -D_GNU_SOURCE=1
CCFLAGS=-fPIC -DUSE_LIBFSMA -D_POSIX_C_SOURCE=199309L -D_XOPEN_SOURCE -D_REENTRANT -std=c11 -fstrict-aliasing -pedantic -pthread -Wall -Werror -g
LDFLAGS=-g -pthread
INCLUDEDIRS=-I../../include
OBJS=monitor.o ../common/log.o ../common/debug.o
TARGET=usm

all:
	${CC} ${ECFLAGS} ${CCFLAGS} ${INCLUDEDIRS} -c monitor.c
	${CC} -o ${TARGET} ${OBJS} ${LDFLAGS}

install:
	cp usm /usr/sbin/

clean:
	rm -f ${OBJS}
	rm -f ${TARGET}

