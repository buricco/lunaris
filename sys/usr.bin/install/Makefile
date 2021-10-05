all:	xinstall

xinstall:	xinstall.o pwcache.o setmode.o strtonum.o
	$(CC) -o xinstall xinstall.o pwcache.o setmode.o strtonum.o

xinstall.o:	xinstall.c pathnames.h
	$(CC) -c -o xinstall.o xinstall.c

pwcache.o:	pwcache.c
	$(CC) -c -o pwcache.o pwcache.c

setmode.o:	setmode.c
	$(CC) -c -o setmode.o setmode.c

strtonum.o:	strtonum.c strtonum.h
	$(CC) -c -o strtonum.o strtonum.c

clean:
	rm -f xinstall xinstall.o pwcache.o setmode.o strtonum.o

install:
	./xinstall -D -m 755 xinstall $(DESTDIR)/bin/install
