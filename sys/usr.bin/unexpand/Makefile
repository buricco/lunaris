all:	unexpand

unexpand:	unexpand.o
	$(CC) -o unexpand unexpand.o

unexpand.o:	unexpand.c
	$(CC) -c -o unexpand.o unexpand.c

clean:
	rm -f unexpand unexpand.o

install:
	install -d -m 755 unexpand $(DESTDIR)/bin
