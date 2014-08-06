CC=`cat ../../.compiler`
ECFLAGS=-DCONFIG_SYS_LINUX=1 -D_GNU_SOURCE=1
CCFLAGS=-fPIC -DUSE_LIBFSMA -D_POSIX_C_SOURCE=199309L -D_XOPEN_SOURCE -D_REENTRANT -std=c11 -fstrict-aliasing -pedantic -pthread -Wall -Werror -g
LDFLAGS=-g -pthread -lpall -lfsma -lfsop -lpsec
INCLUDEDIRS=-I../../include
OBJS_COMMON=../common/config.o ../common/debug.o ../common/log.o ../common/mm.o ../common/runtime.o
OBJS=admin.o category.o config.o logic.o op.o parse.o print.o process.o runtime.o usage.o users.o
TARGET=usa

all:
	${CC} ${ECFLAGS} ${CCFLAGS} ${INCLUDEDIRS} -c admin.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${INCLUDEDIRS} -c category.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${INCLUDEDIRS} -c config.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${INCLUDEDIRS} -c logic.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${INCLUDEDIRS} -c op.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${INCLUDEDIRS} -c parse.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${INCLUDEDIRS} -c print.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${INCLUDEDIRS} -c process.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${INCLUDEDIRS} -c runtime.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${INCLUDEDIRS} -c usage.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${INCLUDEDIRS} -c users.c
	${CC} -o ${TARGET} ${OBJS} ${OBJS_COMMON} ${LDFLAGS}

install:
	cp ${TARGET} /usr/bin/

clean:
	rm -f ${OBJS}
	rm -f ${TARGET}

