all:	head

head:   head.o strtonum.o
	$(CC) -o head head.o strtonum.o

head.o:	head.c strtonum.h
	$(CC) -c -o head.o head.c

strtonum.o:	strtonum.c strtonum.h
	$(CC) -c -o strtonum.o strtonum.c

clean:
	rm -f head head.o strtonum.o

install:
	install -d -m 755 head $(DESTDIR)/bin
