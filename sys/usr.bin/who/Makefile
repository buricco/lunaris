who:	who.o
	$(CC) -o who who.o

who.o:	who.c
	$(CC) -c -o who.o who.c

clean:
	rm -f who who.o

install:
	install -d -m 755 who ${DESTDIR}/bin
