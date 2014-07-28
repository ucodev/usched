CC=/usr/bin/clang
ECFLAGS=-DCONFIG_SYS_LINUX=1 -D_GNU_SOURCE=1
CCFLAGS=-fPIC -DUSE_LIBFSMA -D_POSIX_C_SOURCE=199309L -D_XOPEN_SOURCE -D_REENTRANT -std=c11 -fstrict-aliasing -pedantic -pthread -Wall -Werror -g
LDFLAGS=-g -pthread
LIB_LDFLAGS=${LDFLAGS} -lpall -lfsma -lpanet
CLIENT_LDFLAGS=${LDFLAGS} -lpall -lfsma -lpanet
DAEMON_LDFLAGS=${LDFLAGS} -lpsched -lpall -lfsma -lrt -lrtsaio -lpanet -lfsop
EXEC_LDFLAGS=${LDFLAGS} -lfsma -lrt
MONITOR_LDFLAGS=${LDFLAGS}
INCLUDEDIRS=-I../include
LIB_OBJS=bitops.o conn_client.o conn_common.o debug.o entry_client.o entry_common.o lib.o log.o logic.o mm.o op.o parse.o pool_client.o print.o runtime_client.o runtime_common.o sig.o thread.o usage.o
CLIENT_OBJS=bitops.o client.o conn_client.o conn_common.o debug.o entry_client.o entry_common.o log.o logic.o mm.o op.o parse.o pool_client.o print.o runtime_client.o runtime_common.o sig.o thread.o usage.o
DAEMON_OBJS=auth.o bitops.o conn_daemon.o conn_common.o daemon.o debug.o entry_common.o entry_daemon.o index.o hash.o local.o log.o marshal.o mm.o notify.o pmq.o pool_daemon.o process.o runtime_common.o runtime_daemon.o schedule.o sig.o thread.o
EXEC_OBJS=bitops.o debug.o exec.o log.o mm.o pmq.o runtime_common.o runtime_exec.o sig.o thread.o
MONITOR_OBJS=monitor.o log.o debug.o
LIB_TARGET=libusc.so
CLIENT_TARGET=usc
DAEMON_TARGET=usd
EXEC_TARGET=use
MONITOR_TARGET=usm

all:
	${CC} ${ECFLAGS} ${CCFLAGS} ${INCLUDEDIRS} -c auth.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${INCLUDEDIRS} -c bitops.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${INCLUDEDIRS} -c client.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${INCLUDEDIRS} -c conn_client.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${INCLUDEDIRS} -c conn_daemon.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${INCLUDEDIRS} -c conn_common.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${INCLUDEDIRS} -c daemon.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${INCLUDEDIRS} -c debug.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${INCLUDEDIRS} -c entry_client.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${INCLUDEDIRS} -c entry_common.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${INCLUDEDIRS} -c entry_daemon.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${INCLUDEDIRS} -c exec.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${INCLUDEDIRS} -c index.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${INCLUDEDIRS} -c hash.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${INCLUDEDIRS} -c lib.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${INCLUDEDIRS} -c local.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${INCLUDEDIRS} -c log.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${INCLUDEDIRS} -c logic.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${INCLUDEDIRS} -c marshal.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${INCLUDEDIRS} -c mm.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${INCLUDEDIRS} -c monitor.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${INCLUDEDIRS} -c notify.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${INCLUDEDIRS} -c op.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${INCLUDEDIRS} -c parse.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${INCLUDEDIRS} -c pmq.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${INCLUDEDIRS} -c pool_client.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${INCLUDEDIRS} -c pool_daemon.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${INCLUDEDIRS} -c print.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${INCLUDEDIRS} -c process.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${INCLUDEDIRS} -c runtime_client.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${INCLUDEDIRS} -c runtime_common.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${INCLUDEDIRS} -c runtime_daemon.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${INCLUDEDIRS} -c runtime_exec.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${INCLUDEDIRS} -c schedule.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${INCLUDEDIRS} -c sig.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${INCLUDEDIRS} -c thread.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${INCLUDEDIRS} -c usage.c
	${CC} -shared -o ${LIB_TARGET} ${LIB_OBJS} ${LIB_LDFLAGS}
	${CC} -o ${CLIENT_TARGET} ${CLIENT_OBJS} ${CLIENT_LDFLAGS}
	${CC} -o ${DAEMON_TARGET} ${DAEMON_OBJS} ${DAEMON_LDFLAGS}
	${CC} -o ${EXEC_TARGET} ${EXEC_OBJS} ${EXEC_LDFLAGS}
	${CC} -o ${MONITOR_TARGET} ${MONITOR_OBJS} ${MONITOR_LDFLAGS}

clean:
	rm -f ${LIB_OBJS}
	rm -f ${CLIENT_OBJS}
	rm -f ${DAEMON_OBJS}
	rm -f ${EXEC_OBJS}
	rm -f ${MONITOR_OBJS}
	rm -f ${LIB_TARGET}
	rm -f ${CLIENT_TARGET}
	rm -f ${DAEMON_TARGET}
	rm -f ${EXEC_TARGET}
	rm -f ${MONITOR_TARGET}

