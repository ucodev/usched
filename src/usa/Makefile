CC=`cat ../../.compiler`
ECFLAGS=-DCONFIG_SYS_LINUX=1 -D_GNU_SOURCE=1
CCFLAGS=-fPIC -D_POSIX_C_SOURCE=199309L -D_XOPEN_SOURCE -D_REENTRANT -fstrict-aliasing -fstack-protector-all -pedantic -pthread -Wall -Werror -Wstack-protector -g
LDFLAGS=-g -lrt -pthread -lpall -lfsma -lfsop -lpsec `cat ../../.libs`
ARCHFLAGS=`cat ../../.archflags`
INCLUDEDIRS=-I../../include
OBJS_COMMON=../common/config.o ../common/debug.o ../common/input.o ../common/log.o ../common/mm.o ../common/runtime.o ../common/term.o
OBJS=admin.o category.o config.o logic.o op.o parse.o print.o runtime.o usage.o users.o
TARGET=usa

all:
	${CC} ${ECFLAGS} ${CCFLAGS} ${ARCHFLAGS} ${INCLUDEDIRS} -c admin.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${ARCHFLAGS} ${INCLUDEDIRS} -c category.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${ARCHFLAGS} ${INCLUDEDIRS} -c config.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${ARCHFLAGS} ${INCLUDEDIRS} -c logic.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${ARCHFLAGS} ${INCLUDEDIRS} -c op.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${ARCHFLAGS} ${INCLUDEDIRS} -c parse.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${ARCHFLAGS} ${INCLUDEDIRS} -c print.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${ARCHFLAGS} ${INCLUDEDIRS} -c runtime.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${ARCHFLAGS} ${INCLUDEDIRS} -c usage.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${ARCHFLAGS} ${INCLUDEDIRS} -c users.c
	${CC} -o ${TARGET} ${OBJS} ${OBJS_COMMON} ${LDFLAGS}

install:
	cp ${TARGET} /usr/bin/

clean:
	rm -f ${OBJS}
	rm -f ${TARGET}

