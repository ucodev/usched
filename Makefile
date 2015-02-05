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
	chmod 755 /etc/usched/network
	chmod 700 /etc/usched/users
	mkdir -p /usr/include/usched
	cp include/*.h /usr/include/usched/
	mkdir -p /var/cache/usched
	chmod 700 /var/cache/usched
	mkdir -p /var/cache/usched/jail
	chmod 700 /var/cache/usched/jail

examples:
	make -C example/

install_classic_centos:
	make -C scripts/ install_classic_centos

install_classic_debian:
	make -C scripts/ install_classic_debian

install_classic_deepin:
	make -C scripts/ install_classic_deepin

install_classic_fedora:
	make -C scripts/ install_classic_fedora

install_classic_redhat:
	make -C scripts/ install_classic_redhat

install_classic_ubuntu:
	make -C scripts/ install_classic_ubuntu

install_systemd:
	make -C scripts/ install_systemd

install_doc:
	mkdir -p /usr/share/doc/usched/doc
	cp AUTHOR /usr/share/doc/usched/
	cp COPYING /usr/share/doc/usched/
	cp INSTALL.txt /usr/share/doc/usched/
	cp README /usr/share/doc/usched/
	cp TODO /usr/share/doc/usched/
	cp VERSION /usr/share/doc/usched/
	cp doc/* /usr/share/doc/usched/doc/

clean:
	make -C src/ clean
	make -C scripts/ clean
	make -C example/ clean
	make -C bindings/ clean

