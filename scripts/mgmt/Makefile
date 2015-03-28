SYSSBINDIR=`cat ../../.dirsbin`

install:
	cp usched_preinit.sh ${SYSSBINDIR}/usched_preinit
	chmod 700 ${SYSSBINDIR}/usched_preinit
	if [ -f "`which python`" ]; then make install_python; else make install_sh; fi
	chmod 700 ${SYSSBINDIR}/usched

install_python:
	cp usched.py ${SYSSBINDIR}/usched

install_sh:
	cp usched.sh ${SYSSBINDIR}/usched

