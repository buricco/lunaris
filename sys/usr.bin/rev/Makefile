all:	rev

rev:	rev.o
	$(CC) -o rev rev.o

rev.o:	rev.c
	$(CC) -c -o rev.o rev.c

clean:
	rm -f rev rev.o

install:
	install -d -m 755 rev $(DESTDIR)/bin
