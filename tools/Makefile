CC=`cat ../.compiler`
ECFLAGS=`cat ../.ecflags`
CCFLAGS=
LDFLAGS=-lpsec `cat ../.libs`
ELFLAGS=`cat ../.elflags`
ARCHFLAGS=`cat ../.archflags`
INCLUDEDIRS=-I../include
OBJS_IPC_PWGEN=usched_ipcpwgen.o
TARGET_IPC_PWGEN=usched_ipcpwgen

all:
	${CC} ${ECFLAGS} ${CCFLAGS} ${ARCHFLAGS} ${INCLUDEDIRS} -c usched_ipcpwgen.c
	${CC} -o ${TARGET_IPC_PWGEN} ${OBJS_IPC_PWGEN} ${OBJS_COMMON} ${LDFLAGS} ${ELFLAGS}

install:
	cp ${TARGET_IPC_PWGEN} /usr/sbin/

clean:
	rm -f ${OBJS_IPC_PWGEN}
	rm -f ${TARGET_IPC_PWGEN}

