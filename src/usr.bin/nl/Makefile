all:	nl

nl:	nl.o
	$(CC) -o nl nl.o

nl.o:	nl.c
	$(CC) -c -o nl.o nl.c

clean:
	rm -f nl nl.o

install:
	install -d -m 755 nl $(DESTDIR)/bin
