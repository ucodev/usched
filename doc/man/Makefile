all:
	gzip man1/*.1 2> /dev/null || true
	gzip man3/*.3 2> /dev/null || true
	gzip man7/*.7 2> /dev/null || true

install:
	cp man1/* /usr/share/man/man1/
	cp man3/* /usr/share/man/man3/
	cp man7/* /usr/share/man/man7/

clean:
	gunzip man1/*.gz 2> /dev/null || true
	gunzip man3/*.gz 2> /dev/null || true
	gunzip man7/*.gz 2> /dev/null || true
