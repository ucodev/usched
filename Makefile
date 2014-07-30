all:
	make -C src/
	make -C scripts/

install_all:
	make -C src/ install
	make -C scripts/ install
	make -C bindings/ install_all
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

clean:
	make -C src/ clean
	make -C scripts/ clean
	make -C example/ clean
	make -C bindings/ clean

