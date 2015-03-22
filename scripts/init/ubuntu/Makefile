SYSCONFDIR=`cat ../../../.dirconf`

install:
	mkdir -p ${SYSCONFDIR}/init.d
	cp usched ${SYSCONFDIR}/init.d/usched
	chmod 755 ${SYSCONFDIR}/init.d/usched
	update-rc.d usched defaults

