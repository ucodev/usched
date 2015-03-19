CC=`cat ../.compiler`
ECFLAGS=`cat ../.ecflags`
CCFLAGS=
LDFLAGS=-lpsec `cat ../.libs`
ELFLAGS=`cat ../.elflags`
ARCHFLAGS=`cat ../.archflags`
INCLUDEDIRS=-I../include
OBJS_IPC_PWGEN=ipc_pwgen.o
TARGET_IPC_PWGEN=ipc_pwgen

all:
	${CC} ${ECFLAGS} ${CCFLAGS} ${ARCHFLAGS} ${INCLUDEDIRS} -c ipc_pwgen.c
	${CC} -o ${TARGET_IPC_PWGEN} ${OBJS_IPC_PWGEN} ${OBJS_COMMON} ${LDFLAGS} ${ELFLAGS}

clean:
	rm -f ${OBJS_IPC_PWGEN}
	rm -f ${TARGET_IPC_PWGEN}

