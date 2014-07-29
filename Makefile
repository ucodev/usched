all:
	make -C src/
	make -C scripts/

install_all:
	make -C src/ install
	make -C scripts/ install

install_debian:
	make -C scripts/ install_debian

clean:
	make -C src/ clean
	make -C scripts/ clean

