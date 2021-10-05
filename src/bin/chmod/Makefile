all:	chmod

chmod:	chmod.o pwcache.o setmode.o strtonum.c
	$(CC) -o chmod chmod.o pwcache.o setmode.o strtonum.c

chmod.o:	chmod.c strtonum.h
	$(CC) -c -o chmod.o chmod.c

pwcache.o:	pwcache.c
	$(CC) -c -o pwcache.o pwcache.c

setmode.o:	setmode.c
	$(CC) -c -o setmode.o setmode.c

strtonum.o:	strtonum.c strtonum.h
	$(CC) -c -o strtonum.o strtonum.c

clean:
	rm -f chmod chmod.o pwcache.o setmode.o strtonum.o

install:
	install -d -m 755 chmod $(DESTDIR)/bin
	ln -s chmod $(DESTDIR)/bin/chown
	ln -s chmod $(DESTDIR)/bin/chgrp
