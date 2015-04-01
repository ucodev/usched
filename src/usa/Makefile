CC=`cat ../../.compiler`
ECFLAGS=`cat ../../.ecflags`
CCFLAGS=-DCONFIG_ADMIN_SPECIFIC=1
LDFLAGS=-lpall -lfsop -lpsec -lpanet `cat ../../.libs`
ELFLAGS=`cat ../../.elflags`
ARCHFLAGS=`cat ../../.archflags`
INCLUDEDIRS=-I../../include
OBJS_COMMON=../common/config.o ../common/debug.o ../common/input.o ../common/log.o ../common/mm.o ../common/pmq.o ../common/runtime.o ../common/str.o ../common/term.o
OBJS=admin.o all.o auth.o category.o config.o core.o exec.o file.o ipc.o logic.o network.o op.o parse.o pmq.o print.o runtime.o stat.o usage.o users.o
TARGET=usa
SYSSBINDIR=`cat ../../.dirsbin`

all:
	${CC} ${ECFLAGS} ${CCFLAGS} ${ARCHFLAGS} ${INCLUDEDIRS} -c admin.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${ARCHFLAGS} ${INCLUDEDIRS} -c all.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${ARCHFLAGS} ${INCLUDEDIRS} -c auth.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${ARCHFLAGS} ${INCLUDEDIRS} -c category.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${ARCHFLAGS} ${INCLUDEDIRS} -c config.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${ARCHFLAGS} ${INCLUDEDIRS} -c core.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${ARCHFLAGS} ${INCLUDEDIRS} -c exec.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${ARCHFLAGS} ${INCLUDEDIRS} -c file.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${ARCHFLAGS} ${INCLUDEDIRS} -c ipc.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${ARCHFLAGS} ${INCLUDEDIRS} -c logic.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${ARCHFLAGS} ${INCLUDEDIRS} -c network.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${ARCHFLAGS} ${INCLUDEDIRS} -c op.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${ARCHFLAGS} ${INCLUDEDIRS} -c parse.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${ARCHFLAGS} ${INCLUDEDIRS} -c pmq.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${ARCHFLAGS} ${INCLUDEDIRS} -c print.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${ARCHFLAGS} ${INCLUDEDIRS} -c runtime.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${ARCHFLAGS} ${INCLUDEDIRS} -c stat.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${ARCHFLAGS} ${INCLUDEDIRS} -c usage.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${ARCHFLAGS} ${INCLUDEDIRS} -c users.c
	${CC} -o ${TARGET} ${OBJS} ${OBJS_COMMON} ${LDFLAGS} ${ELFLAGS}

install:
	cp ${TARGET} ${SYSSBINDIR}/

clean:
	rm -f ${OBJS}
	rm -f ${TARGET}

