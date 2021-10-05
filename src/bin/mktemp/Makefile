all:	mktemp

mktemp:	mktemp.o
	$(CC) -o mktemp mktemp.o

mktemp.o:	mktemp.c
	$(CC) -c -o mktemp.o mktemp.c

clean:
	rm -f mktemp mktemp.o

install:
	install -d -m 755 mktemp $(DESTDIR)/bin
