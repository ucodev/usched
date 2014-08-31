all:
	make -C src/
	make -C scripts/

install_all:
	make -C src/ install
	make -C scripts/ install
	make -C bindings/ install_all
	mkdir -p /etc/usched
	cp -r config/* /etc/usched/
	chown -R root:root /etc/usched
	chmod 700 /etc/usched/auth
	chmod 700 /etc/usched/core
	chmod 700 /etc/usched/keys
	chmod 755 /etc/usched/network
	chmod 700 /etc/usched/users
	mkdir -p /usr/include/usched
	cp include/*.h /usr/include/usched/

examples:
	make -C example/

install_centos:
	make -C scripts/ install_centos

install_debian:
	make -C scripts/ install_debian

install_deepin:
	make -C scripts/ install_deepin

install_fedora:
	make -C scripts/ install_fedora

install_redhat:
	make -C scripts/ install_redhat

install_ubuntu:
	make -C scripts/ install_ubuntu

install_doc:
	mkdir -p /usr/share/doc/usched/doc
	cp AUTHOR /usr/share/doc/usched/
	cp COPYING /usr/share/doc/usched/
	cp INSTALL /usr/share/doc/usched/
	cp README /usr/share/doc/usched/
	cp TODO /usr/share/doc/usched/
	cp VERSION /usr/share/doc/usched/
	cp doc/* /usr/share/doc/usched/doc/

clean:
	make -C src/ clean
	make -C scripts/ clean
	make -C example/ clean
	make -C bindings/ clean

