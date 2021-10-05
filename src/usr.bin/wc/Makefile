all:	wc

wc:	wc.o
	$(CC) -o wc wc.o

wc.o:	wc.c
	$(CC) -c -o wc.o wc.c

clean:
	rm -f wc wc.o

install:
	install -d -m 755 wc $(DESTDIR)/bin
