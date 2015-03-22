SYSLIBDIR=`cat ../../.dirlib`
SYSSBINDIR=`cat ../../.dirsbin`

all:

install:
	ln -s ${SYSSBINDIR}/usched ${SYSLIBDIR}/systemd/scripts/usched
	cp usched.service ${SYSLIBDIR}/systemd/system/

clean:

