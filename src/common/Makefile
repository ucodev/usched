CC=`cat ../../.compiler`
ECFLAGS=-DCONFIG_SYS_LINUX=1 -D_GNU_SOURCE=1
CCFLAGS=-fPIC -DUSE_LIBFSMA -D_POSIX_C_SOURCE=199309L -D_XOPEN_SOURCE -D_REENTRANT -std=c11 -fstrict-aliasing -pedantic -pthread -Wall -Werror -g
INCLUDEDIRS=-I../../include
OBJS=bitops.o config.o conn.o debug.o entry.o local.o log.o mm.o pmq.o runtime.o sig.o thread.o

all:
	${CC} ${ECFLAGS} ${CCFLAGS} ${INCLUDEDIRS} -c bitops.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${INCLUDEDIRS} -c config.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${INCLUDEDIRS} -c conn.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${INCLUDEDIRS} -c debug.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${INCLUDEDIRS} -c entry.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${INCLUDEDIRS} -c local.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${INCLUDEDIRS} -c log.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${INCLUDEDIRS} -c mm.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${INCLUDEDIRS} -c pmq.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${INCLUDEDIRS} -c runtime.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${INCLUDEDIRS} -c sig.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${INCLUDEDIRS} -c thread.c

clean:
	rm -f ${OBJS}

