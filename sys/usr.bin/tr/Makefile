all:	tr

tr:	tr.o str.o
	$(CC) -o tr tr.o str.o

tr.o:	tr.c extern.h
	$(CC) -c -o tr.o tr.c

str.o:	str.c extern.h
	$(CC) -c -o str.o str.c

clean:
	rm -f tr tr.o str.o

install:
	install -d -m 755 tr $(DESTDIR)/bin
