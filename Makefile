all:
	make -C src/

install:
	make -C src/ install
	cp scripts/usched /usr/sbin/

clean:
	make -C src/ clean

