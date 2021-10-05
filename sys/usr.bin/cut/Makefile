all:	cut

cut:   cut.o strtonum.o
	$(CC) -o cut cut.o strtonum.o

cut.o:	cut.c strtonum.h
	$(CC) -c -o cut.o cut.c

strtonum.o:	strtonum.c strtonum.h
	$(CC) -c -o strtonum.o strtonum.c

clean:
	rm -f cut cut.o strtonum.o

install:
	install -d -m 755 cut $(DESTDIR)/bin
