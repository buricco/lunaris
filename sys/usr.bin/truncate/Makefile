all:	truncate

truncate:	truncate.o
	$(CC) -o truncate truncate.o

truncate.o:	truncate.c
	$(CC) -c -o truncate.o truncate.c

clean:
	rm -f truncate truncate.o

install:
	install -d -m 755 truncate $(DESTDIR)/bin

