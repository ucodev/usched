CC=`cat ../../.compiler`
ECFLAGS=`cat ../../.ecflags`
CCFLAGS=-DCONFIG_CLIENT_SPECIFIC=1
LDFLAGS=-lpall -lfsma -lfsop -lpanet -lpsec `cat ../../.libs`
ELFLAGS=`cat ../../.elflags`
ARCHFLAGS=`cat ../../.archflags`
INCLUDEDIRS=-I../../include
OBJS_COMMON=../common/bitops.o ../common/config.o ../common/conn.o ../common/debug.o ../common/entry.o ../common/input.o ../common/log.o ../common/mm.o ../common/runtime.o ../common/str.o ../common/term.o
OBJS_LIB=auth.o config.o conn.o entry.o lib.o logic.o op.o opt.o parse.o pool.o print.o process.o runtime.o sig.o usage.o
OBJS_CLIENT=auth.o config.o client.o conn.o entry.o logic.o op.o opt.o parse.o pool.o print.o process.o runtime.o sig.o usage.o
TARGET_LIB=libusc.`cat ../../.extlib`
TARGET_CLIENT=usc

all:
	${CC} ${ECFLAGS} ${CCFLAGS} ${ARCHFLAGS} ${INCLUDEDIRS} -c auth.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${ARCHFLAGS} ${INCLUDEDIRS} -c client.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${ARCHFLAGS} ${INCLUDEDIRS} -c config.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${ARCHFLAGS} ${INCLUDEDIRS} -c conn.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${ARCHFLAGS} ${INCLUDEDIRS} -c entry.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${ARCHFLAGS} ${INCLUDEDIRS} -c lib.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${ARCHFLAGS} ${INCLUDEDIRS} -c logic.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${ARCHFLAGS} ${INCLUDEDIRS} -c op.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${ARCHFLAGS} ${INCLUDEDIRS} -c opt.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${ARCHFLAGS} ${INCLUDEDIRS} -c parse.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${ARCHFLAGS} ${INCLUDEDIRS} -c pool.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${ARCHFLAGS} ${INCLUDEDIRS} -c print.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${ARCHFLAGS} ${INCLUDEDIRS} -c process.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${ARCHFLAGS} ${INCLUDEDIRS} -c runtime.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${ARCHFLAGS} ${INCLUDEDIRS} -c sig.c
	${CC} ${ECFLAGS} ${CCFLAGS} ${ARCHFLAGS} ${INCLUDEDIRS} -c usage.c
	${CC} -o ${TARGET_CLIENT} ${OBJS_CLIENT} ${OBJS_COMMON} ${LDFLAGS} ${ELFLAGS}
	${CC} -shared -o ${TARGET_LIB} ${OBJS_LIB} ${OBJS_COMMON} ${LDFLAGS} ${ELFLAGS}

install:
	cp ${TARGET_CLIENT} /usr/bin/
	cp ${TARGET_LIB} /usr/lib/

clean:
	rm -f ${OBJS_CLIENT}
	rm -f ${OBJS_LIB}
	rm -f ${TARGET_CLIENT}
	rm -f ${TARGET_LIB}

