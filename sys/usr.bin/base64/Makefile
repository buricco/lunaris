all:	base64

base64:	base64.o
	$(CC) -o base64 base64.o

base64.o:	base64.c
	$(CC) -c -o base64.o base64.c

clean:
	rm -f base64 base64.o

install:
	install -d -m 755 base64 $(DESTDIR)/bin
