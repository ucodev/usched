CC=/usr/bin/clang
ECFLAGS=-DCONFIG_SYS_LINUX=1 -D_GNU_SOURCE=1
CCFLAGS=-fPIC -DUSE_LIBFSMA -D_POSIX_C_SOURCE=199309L -D_XOPEN_SOURCE -D_REENTRANT -std=c11 -fstrict-aliasing -pedantic -pthread -Wall -Werror -g
LDFLAGS=-g -pthread -lpsched -lpall -lfsma -lrt -lrtsaio -lpanet -lfsop
INCLUDEDIRS=-I../include
SOURCES=auth.c bitops.c client.c conn.c daemon.c daemonize.c debug.c entry.c exec.c index.c hash.c lib.c local.c log.c logic.c marshal.c mm.c notify.c op.c parse.c pmq.c pool.c print.c process.c runtime.c schedule.c sig.c thread.c usage.c
LIB_OBJS=auth.o bitops.o conn.o debug.o entry.o index.o hash.o lib.o local.o log.o logic.o marshal.o mm.o op.o notify.o parse.o pmq.o pool.o print.o process.o runtime.o schedule.o sig.o thread.o usage.o
CLIENT_OBJS=auth.o bitops.o client.o conn.o debug.o entry.o index.o hash.o local.o log.o logic.o marshal.o mm.o notify.o op.o parse.o pmq.o pool.o print.o process.o runtime.o schedule.o sig.o thread.o usage.o
DAEMON_OBJS=auth.o bitops.o conn.o daemon.o daemonize.o debug.o entry.o index.o hash.o local.o log.o logic.o marshal.o mm.o notify.o op.o parse.o pmq.o pool.o print.o process.o runtime.o schedule.o sig.o thread.o usage.o
EXEC_OBJS=auth.o bitops.o conn.o daemonize.o debug.o entry.o exec.o index.o hash.o local.o log.o logic.o marshal.o mm.o notify.o op.o parse.o pmq.o pool.o print.o process.o runtime.o schedule.o sig.o thread.o usage.o
LIB_TARGET=libusc.so
CLIENT_TARGET=usc
DAEMON_TARGET=usd
EXEC_TARGET=use

all:
	${CC} ${ECFLAGS} ${CCFLAGS} ${INCLUDEDIRS} -c ${SOURCES}
	${CC} -shared -o ${LIB_TARGET} ${LIB_OBJS} ${LDFLAGS}
	${CC} -o ${CLIENT_TARGET} ${CLIENT_OBJS} ${LDFLAGS}
	${CC} -o ${DAEMON_TARGET} ${DAEMON_OBJS} ${LDFLAGS}
	${CC} -o ${EXEC_TARGET} ${EXEC_OBJS} ${LDFLAGS}

clean:
	rm -f ${LIB_OBJS}
	rm -f ${CLIENT_OBJS}
	rm -f ${DAEMON_OBJS}
	rm -f ${EXEC_OBJS}
	rm -f ${LIB_TARGET}
	rm -f ${CLIENT_TARGET}
	rm -f ${DAEMON_TARGET}
	rm -f ${EXEC_TARGET}

