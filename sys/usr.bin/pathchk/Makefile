all:	pathchk

pathchk:	pathchk.o
	$(CC) -o pathchk pathchk.o

pathchk.o:	pathchk.c
	$(CC) -c -o pathchk.o pathchk.c

clean:
	rm -f pathchk pathchk.o

install:
	install -d -m 755 pathchk $(DESTDIR)/bin
