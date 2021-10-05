all:	date

date:   date.o strtonum.o
	$(CC) -o date date.o strtonum.o -lutil

date.o:	date.c strtonum.h
	$(CC) -c -o date.o date.c

strtonum.o:	strtonum.c strtonum.h
	$(CC) -c -o strtonum.o strtonum.c

clean:
	rm -f date date.o strtonum.o

install:
	install -d -m 755 date $(DESTDIR)/bin
