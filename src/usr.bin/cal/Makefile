cal:	cal.o
	$(CC) -o cal cal.o

cal.o:	cal.c
	$(CC) -c -o cal.o cal.c

clean:
	rm -f cal cal.o

install:
	install -d -m 755 cal ${DESTDIR}/bin
