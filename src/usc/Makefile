CC=/usr/bin/clang
ECFLAGS=-DCONFIG_SYS_LINUX=1 -D_GNU_SOURCE=1
CCFLAGS=-fPIC -DUSE_LIBFSMA -D_POSIX_C_SOURCE=199309L -D_XOPEN_SOURCE -D_REENTRANT -std=c11 -fstrict-aliasing -pedantic -pthread -Wall -Werror -g
LDFLAGS=-g -pthread -lpall -lfsma -lpanet
INCLUDEDIRS=-I../../include
LIB_OBJS=../common/bitops.o conn.o ../common/conn.o ../common/debug.o entry.o ../common/entry.o lib.o ../common/log.o logic.o ../common/mm.o op.o parse.o pool.o print.o runtime.o ../common/runtime.o ../common/sig.o ../common/thread.o usage.o
CLIENT_OBJS=../common/bitops.o client.o conn.o ../common/conn.o ../common/debug.o entry.o ../common/entry.o ../common/log.o logic.o ../common/mm.o op.o parse.o pool.o print.o runtime.o ../common/runtime.o ../common/sig.o ../common/thread.o usage.o
LIB_TARGET=libusc.so
CLIENT_TARGET=usc

all:
	${CC} ${ECFLAGS} ${CCFLAGS} ${INCLUDEDIRS} -c client.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${INCLUDEDIRS} -c conn.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${INCLUDEDIRS} -c entry.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${INCLUDEDIRS} -c lib.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${INCLUDEDIRS} -c logic.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${INCLUDEDIRS} -c op.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${INCLUDEDIRS} -c parse.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${INCLUDEDIRS} -c pool.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${INCLUDEDIRS} -c print.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${INCLUDEDIRS} -c runtime.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${INCLUDEDIRS} -c usage.c
	${CC} -shared -o ${LIB_TARGET} ${LIB_OBJS} ${LDFLAGS}
	${CC} -o ${CLIENT_TARGET} ${CLIENT_OBJS} ${LDFLAGS}

install:
	cp usc /usr/bin/

clean:
	rm -f ${LIB_OBJS}
	rm -f ${CLIENT_OBJS}
	rm -f ${LIB_TARGET}
	rm -f ${CLIENT_TARGET}

