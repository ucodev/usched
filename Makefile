all:
	cd src && make && cd ..
	cd scripts && make && cd ..

install_all:
	cd src && make install && cd ..
	cd scripts && make install && cd ..
	mkdir -p /etc/usched
	cp -r config/* /etc/usched/
	chown -R root /etc/usched
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

install_bindings:
	cd bindings && make install_all && cd ..

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

install_systemd:
	cd scripts && make install_systemd && cd ..

install_doc:
	mkdir -p /usr/share/doc/usched/
	cp README /usr/share/doc/usched/
	cp doc/* /usr/share/doc/usched/

clean:
	cd src && make clean && cd ..
	cd scripts && make clean && cd ..
	cd example && make clean && cd ..
	cd bindings && make clean && cd ..

