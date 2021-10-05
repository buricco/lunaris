all:	timeout

timeout:	timeout.o signames.o
	$(CC) -o timeout timeout.o signames.o

timeout.o:	timeout.c
	$(CC) -c -o timeout.o timeout.c

signames.o:	signames.c signames.h
	$(CC) -c -o signames.o signames.c

clean:
	rm -f timeout timeout.o signames.o

install:
	install -d -m 755 timeout $(DESTDIR)/bin
