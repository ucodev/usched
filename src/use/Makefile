CC=`cat ../../.compiler`
ECFLAGS=`cat ../../.ecflags`
CCFLAGS=-DCONFIG_EXEC_SPECIFIC=1
LDFLAGS=-lfsop -lpall -lpanet
ELFLAGS=`cat ../../.elflags`
ARCHFLAGS=`cat ../../.archflags`
INCLUDEDIRS=-I../../include
OBJS_COMMON=../common/bitops.o ../common/config.o ../common/debug.o ../common/ipc.o ../common/local.o ../common/log.o ../common/mm.o ../common/runtime.o ../common/str.o ../common/thread.o
OBJS=config.o exec.o ipc.o runtime.o sig.o thread.o
TARGET=use
SYSSBINDIR=`cat ../../.dirsbin`

all:
	${CC} ${ECFLAGS} ${CCFLAGS} ${ARCHFLAGS} ${INCLUDEDIRS} -c config.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${ARCHFLAGS} ${INCLUDEDIRS} -c exec.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${ARCHFLAGS} ${INCLUDEDIRS} -c ipc.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${ARCHFLAGS} ${INCLUDEDIRS} -c runtime.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${ARCHFLAGS} ${INCLUDEDIRS} -c sig.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${ARCHFLAGS} ${INCLUDEDIRS} -c thread.c
	${CC} -o ${TARGET} ${OBJS} ${OBJS_COMMON} ${LDFLAGS} ${ELFLAGS}

install:
	cp ${TARGET} ${SYSSBINDIR}/

clean:
	rm -f ${OBJS}
	rm -f ${TARGET}

