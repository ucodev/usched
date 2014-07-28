CC=/usr/bin/clang
ECFLAGS=-DCONFIG_SYS_LINUX=1 -D_GNU_SOURCE=1
CCFLAGS=-fPIC -DUSE_LIBFSMA -D_POSIX_C_SOURCE=199309L -D_XOPEN_SOURCE -D_REENTRANT -std=c11 -fstrict-aliasing -pedantic -pthread -Wall -Werror -g
LDFLAGS=-g -pthread -lpsched -lpall -lfsma -lrt -lrtsaio -lpanet -lfsop
INCLUDEDIRS=-I../include
SOURCES=auth.c bitops.c client.c conn_client.c conn_daemon.c conn_common.c daemon.c daemonize.c debug.c entry_client.c entry_common.c entry_daemon.c exec.c index.c hash.c lib.c local.c log.c logic.c marshal.c mm.c notify.c op.c parse.c pmq.c pool_client.c pool_daemon.c print.c process.c runtime_client.c runtime_common.c runtime_daemon.c runtime_exec.c schedule.c sig.c thread.c usage.c
LIB_OBJS=bitops.o conn_client.o conn_common.o debug.o entry_client.o entry_common.o lib.o log.o logic.o mm.o op.o parse.o pmq.o pool_client.o print.o runtime_client.o runtime_common.o sig.o thread.o usage.o
CLIENT_OBJS=bitops.o client.o conn_client.o conn_common.o debug.o entry_client.o entry_common.o log.o logic.o mm.o op.o parse.o pmq.o pool_client.o print.o runtime_client.o runtime_common.o sig.o thread.o usage.o
DAEMON_OBJS=auth.o bitops.o conn_daemon.o conn_common.o daemon.o daemonize.o debug.o entry_common.o entry_daemon.o index.o hash.o local.o log.o marshal.o mm.o notify.o pmq.o pool_daemon.o process.o runtime_common.o runtime_daemon.o schedule.o sig.o thread.o
EXEC_OBJS=bitops.o daemonize.o debug.o exec.o log.o mm.o pmq.o runtime_common.o runtime_exec.o sig.o thread.o
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

