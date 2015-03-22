SYSCONFDIR=`cat ../../../.dirconf`

install:
	mkdir -p ${SYSCONFDIR}/rc.d/init.d
	cp usched ${SYSCONFDIR}/rc.d/init.d/usched
	chmod 755 ${SYSCONFDIR}/rc.d/init.d/usched
	chkconfig usched on

