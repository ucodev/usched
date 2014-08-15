CC=`cat ../../.compiler`
ECFLAGS=-DCONFIG_SYS_LINUX=1 -D_GNU_SOURCE=1
CCFLAGS=-fPIC -DUSE_LIBFSMA -D_POSIX_C_SOURCE=199309L -D_XOPEN_SOURCE -D_REENTRANT -std=c11 -fstrict-aliasing -pedantic -pthread -Wall -Werror -g
LDFLAGS=-g -pthread -lpall -lfsma -lfsop -lpanet -lpsec -lgmp -lcrypt
INCLUDEDIRS=-I../../include
OBJS_COMMON=../common/bitops.o ../common/config.o ../common/conn.o ../common/debug.o ../common/entry.o ../common/input.o ../common/log.o ../common/mm.o ../common/runtime.o ../common/sec.o ../common/sig.o ../common/term.o ../common/thread.o
OBJS_LIB=config.o conn.o entry.o lib.o logic.o op.o opt.o parse.o pool.o print.o process.o runtime.o sec.o usage.o
OBJS_CLIENT=auth.o config.o client.o conn.o entry.o logic.o op.o opt.o parse.o pool.o print.o process.o runtime.o sec.o usage.o
TARGET_LIB=libusc.so
TARGET_CLIENT=usc

all:
	${CC} ${ECFLAGS} ${CCFLAGS} ${INCLUDEDIRS} -c auth.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${INCLUDEDIRS} -c client.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${INCLUDEDIRS} -c config.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${INCLUDEDIRS} -c conn.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${INCLUDEDIRS} -c entry.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${INCLUDEDIRS} -c lib.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${INCLUDEDIRS} -c logic.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${INCLUDEDIRS} -c op.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${INCLUDEDIRS} -c opt.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${INCLUDEDIRS} -c parse.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${INCLUDEDIRS} -c pool.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${INCLUDEDIRS} -c print.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${INCLUDEDIRS} -c process.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${INCLUDEDIRS} -c runtime.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${INCLUDEDIRS} -c sec.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${INCLUDEDIRS} -c usage.c
	${CC} -o ${TARGET_CLIENT} ${OBJS_CLIENT} ${OBJS_COMMON} ${LDFLAGS}
	${CC} -shared -o ${TARGET_LIB} ${OBJS_LIB} ${OBJS_COMMON} ${LDFLAGS}

install:
	cp ${TARGET_CLIENT} /usr/bin/
	cp ${TARGET_LIB} /usr/lib/

clean:
	rm -f ${OBJS_CLIENT}
	rm -f ${OBJS_LIB}
	rm -f ${TARGET_CLIENT}
	rm -f ${TARGET_LIB}

