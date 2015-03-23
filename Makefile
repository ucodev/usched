SYSBINDIR=`cat .dirbin`
SYSCACHEDIR=`cat .dircache`
SYSCONFDIR=`cat .dirconf`
SYSDOCDIR=`cat .dirdoc`
SYSINCLUDEDIR=`cat .dirinclude`
SYSLIBDIR=`cat .dirlib`
SYSMANDIR=`cat .dirman`
SYSRUNDIR=`cat .dirrun`
SYSSBINDIR=`cat .dirsbin`
SYSSHAREDIR=`cat .dirshare`
SYSTMPDIR=`cat .dirtmp`

all:
	cd src && make && cd ..
	cd scripts && make && cd ..
	cd tools && make && cd ..

install_all:
	cd src && make install && cd ..
	cd scripts && make install && cd ..
	cd tools && make install && cd ..
	mkdir -p ${SYSCONFDIR}/usched
	cp -r config/* ${SYSCONFDIR}/usched/
	chown -R root ${SYSCONFDIR}/usched
	chmod 700 ${SYSCONFDIR}/usched/auth
	chmod 700 ${SYSCONFDIR}/usched/core
	chmod 755 ${SYSCONFDIR}/usched/network
	chmod 700 ${SYSCONFDIR}/usched/users
	mkdir -p ${SYSINCLUDEDIR}/usched
	cp include/*.h ${SYSINCLUDEDIR}/usched/
	mkdir -p ${SYSCACHEDIR}/usched
	chmod 700 ${SYSCACHEDIR}/usched
	mkdir -p ${SYSCACHEDIR}/usched/jail
	chmod 700 ${SYSCACHEDIR}/usched/jail
	usa commit all

install_bindings_all:
	cd bindings && make install_all && cd ..

install_bindings_csharp:
	cd bindings && make install_csharp && cd ..

install_bindings_java:
	cd bindings && make install_java && cd ..

install_bindings_php:
	cd bindings && make install_php && cd ..

install_bindings_python:
	cd bindings && make isntall_python && cd ..

examples:
	cd example && make && cd ..

install_classic_centos:
	cd scripts && make install_classic_centos && cd ..

install_classic_debian:
	cd scripts && make install_classic_debian && cd ..

install_classic_deepin:
	cd scripts && make install_classic_deepin && cd ..

install_classic_fedora:
	cd scripts && make install_classic_fedora && cd ..

install_classic_redhat:
	cd scripts && make install_classic_redhat && cd ..

install_classic_ubuntu:
	cd scripts && make install_classic_ubuntu && cd ..

install_freebsd:
	cd scripts && make install_freebsd && cd ..

install_systemd:
	cd scripts && make install_systemd && cd ..

install_doc:
	mkdir -p ${SYSDOCDIR}/usched/
	cp README ${SYSDOCDIR}/usched/
	cd doc && make text_install && cd ..
	cd doc && make manpages && make manpages_install && cd ..

doxygen:
	cd doc && make doxygen_files && cd ..

test:
	cd tests && make && cd ..

clean:
	cd doc && make clean && cd ..
	cd src && make clean && cd ..
	cd scripts && make clean && cd ..
	cd example && make clean && cd ..
	cd bindings && make clean && cd ..
	cd tests && make clean && cd ..
	cd tools && make clean && cd ..

