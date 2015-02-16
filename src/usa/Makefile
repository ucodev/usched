CC=`cat ../../.compiler`
ECFLAGS=`cat ../../.defines`
CCFLAGS=-std=c1x -D_POSIX_C_SOURCE=199309L -DCONFIG_ADMIN_SPECIFIC=1 -D_REENTRANT -fstrict-aliasing -fstack-protector-all -pedantic -pthread -Wall -Werror -Wstack-protector -g
LDFLAGS=-g -lrt -pthread -lpall -lfsma -lfsop -lpsec `cat ../../.libs`
ARCHFLAGS=`cat ../../.archflags`
INCLUDEDIRS=-I../../include
OBJS_COMMON=../common/config.o ../common/debug.o ../common/input.o ../common/log.o ../common/mm.o ../common/runtime.o ../common/str.o ../common/term.o
OBJS=admin.o auth.o category.o config.o core.o file.o logic.o network.o op.o parse.o print.o runtime.o usage.o users.o
TARGET=usa

all:
	${CC} ${ECFLAGS} ${CCFLAGS} ${ARCHFLAGS} ${INCLUDEDIRS} -c admin.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${ARCHFLAGS} ${INCLUDEDIRS} -c auth.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${ARCHFLAGS} ${INCLUDEDIRS} -c category.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${ARCHFLAGS} ${INCLUDEDIRS} -c config.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${ARCHFLAGS} ${INCLUDEDIRS} -c core.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${ARCHFLAGS} ${INCLUDEDIRS} -c file.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${ARCHFLAGS} ${INCLUDEDIRS} -c logic.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${ARCHFLAGS} ${INCLUDEDIRS} -c network.c
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

