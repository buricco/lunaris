all:	paste

paste:	paste.o
	$(CC) -o paste paste.o

paste.o:	paste.c
	$(CC) -c -o paste.o paste.c

clean:
	rm -f paste paste.o

install:
	install -d -m 755 paste $(DESTDIR)/bin
