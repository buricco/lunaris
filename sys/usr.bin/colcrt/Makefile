all:	colcrt

colcrt:	colcrt.o
	$(CC) -o colcrt colcrt.o

colcrt.o:	colcrt.c
	$(CC) -c -o colcrt.o colcrt.c

clean:
	rm -f colcrt colcrt.o

install:
	install -d -m 755 colcrt $(DESTDIR)/bin
