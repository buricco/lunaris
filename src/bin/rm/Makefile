all:	rm

rm:	rm.o pwcache.o strmode.o
	$(CC) -o rm rm.o pwcache.o strmode.o

rm.o:	rm.c
	$(CC) -c -o rm.o rm.c

pwcache.o:	pwcache.c
	$(CC) -c -o pwcache.o pwcache.c

strmode.o:	strmode.c
	$(CC) -c -o strmode.o strmode.c

clean:
	rm -f rm rm.o pwcache.o strmode.o

install:
	install -d -m 755 rm $(DESTDIR)/bin
