all:	col

col:   col.o strtonum.o
	$(CC) -o col col.o strtonum.o

col.o:	col.c strtonum.h
	$(CC) -c -o col.o col.c

strtonum.o:	strtonum.c strtonum.h
	$(CC) -c -o strtonum.o strtonum.c

clean:
	rm -f col col.o strtonum.o

install:
	install -d -m 755 col $(DESTDIR)/bin
