CC=`cat ../../.compiler`
ECFLAGS=-DCONFIG_SYS_LINUX=1 -D_GNU_SOURCE=1
CCFLAGS=-fPIC -DUSE_LIBFSMA -D_POSIX_C_SOURCE=199309L -D_XOPEN_SOURCE -D_REENTRANT -std=c11 -fstrict-aliasing -pedantic -pthread -Wall -Werror -g
LDFLAGS=-g -pthread -lpsched -lpall -lfsma -lrt -lrtsaio -lpanet -lfsop -lpsec `cat ../../.libs`
INCLUDEDIRS=-I../../include
OBJS_COMMON=../common/bitops.o ../common/config.o ../common/conn.o ../common/debug.o ../common/entry.o ../common/hash.o ../common/local.o ../common/log.o ../common/mm.o ../common/pmq.o ../common/runtime.o ../common/sig.o ../common/thread.o
OBJS=auth.o config.o conn.o daemon.o entry.o index.o marshal.o notify.o pmq.o pool.o process.o runtime.o schedule.o
TARGET=usd

all:
	${CC} ${ECFLAGS} ${CCFLAGS} ${INCLUDEDIRS} -c auth.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${INCLUDEDIRS} -c config.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${INCLUDEDIRS} -c conn.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${INCLUDEDIRS} -c daemon.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${INCLUDEDIRS} -c entry.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${INCLUDEDIRS} -c index.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${INCLUDEDIRS} -c marshal.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${INCLUDEDIRS} -c notify.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${INCLUDEDIRS} -c pmq.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${INCLUDEDIRS} -c pool.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${INCLUDEDIRS} -c process.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${INCLUDEDIRS} -c runtime.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${INCLUDEDIRS} -c schedule.c
	${CC} -o ${TARGET} ${OBJS} ${OBJS_COMMON} ${LDFLAGS}

install:
	cp ${TARGET} /usr/sbin/

clean:
	rm -f ${OBJS}
	rm -f ${TARGET}

