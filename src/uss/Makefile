CC=`cat ../../.compiler`
ECFLAGS=`cat ../../.ecflags`
CCFLAGS=-DCONFIG_STAT_SPECIFIC=1
LDFLAGS=-lfsop -lpall -lpanet -lpipc -lpsched
ELFLAGS=`cat ../../.elflags`
ARCHFLAGS=`cat ../../.archflags`
INCLUDEDIRS=-I../../include
OBJS_COMMON=../common/bitops.o ../common/config.o ../common/debug.o ../common/ipc.o ../common/local.o ../common/log.o ../common/mm.o ../common/runtime.o ../common/str.o ../common/thread.o
OBJS=config.o ipc.o pool.o report.o runtime.o sig.o stat.o thread.o
TARGET=uss
SYSSBINDIR=`cat ../../.dirsbin`

all:
	${CC} ${ECFLAGS} ${CCFLAGS} ${ARCHFLAGS} ${INCLUDEDIRS} -c config.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${ARCHFLAGS} ${INCLUDEDIRS} -c ipc.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${ARCHFLAGS} ${INCLUDEDIRS} -c pool.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${ARCHFLAGS} ${INCLUDEDIRS} -c report.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${ARCHFLAGS} ${INCLUDEDIRS} -c runtime.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${ARCHFLAGS} ${INCLUDEDIRS} -c sig.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${ARCHFLAGS} ${INCLUDEDIRS} -c stat.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${ARCHFLAGS} ${INCLUDEDIRS} -c thread.c
	${CC} -o ${TARGET} ${OBJS} ${OBJS_COMMON} ${LDFLAGS} ${ELFLAGS}

install:
	cp ${TARGET} ${SYSSBINDIR}/

clean:
	rm -f ${OBJS}
	rm -f ${TARGET}

