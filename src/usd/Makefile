CC=/usr/bin/clang
ECFLAGS=-DCONFIG_SYS_LINUX=1 -D_GNU_SOURCE=1
CCFLAGS=-fPIC -DUSE_LIBFSMA -D_POSIX_C_SOURCE=199309L -D_XOPEN_SOURCE -D_REENTRANT -std=c11 -fstrict-aliasing -pedantic -pthread -Wall -Werror -g
LDFLAGS=-g -pthread -lpsched -lpall -lfsma -lrt -lrtsaio -lpanet -lfsop
INCLUDEDIRS=-I../../include
OBJS=auth.o ../common/bitops.o ../common/conn.o conn.o daemon.o ../common/debug.o ../common/entry.o entry.o index.o hash.o local.o ../common/log.o marshal.o ../common/mm.o notify.o ../common/pmq.o pool.o process.o ../common/runtime.o runtime.o schedule.o ../common/sig.o ../common/thread.o
TARGET=usd

all:
	${CC} ${ECFLAGS} ${CCFLAGS} ${INCLUDEDIRS} -c auth.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${INCLUDEDIRS} -c conn.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${INCLUDEDIRS} -c daemon.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${INCLUDEDIRS} -c entry.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${INCLUDEDIRS} -c index.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${INCLUDEDIRS} -c hash.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${INCLUDEDIRS} -c local.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${INCLUDEDIRS} -c marshal.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${INCLUDEDIRS} -c notify.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${INCLUDEDIRS} -c pool.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${INCLUDEDIRS} -c process.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${INCLUDEDIRS} -c runtime.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${INCLUDEDIRS} -c schedule.c
	${CC} -o ${TARGET} ${OBJS} ${LDFLAGS}

install:
	cp usd /usr/sbin/

clean:
	rm -f ${OBJS}
	rm -f ${TARGET}

