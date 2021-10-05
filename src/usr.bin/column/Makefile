all:	column

column:   column.o strtonum.o
	$(CC) -o column column.o strtonum.o

column.o:	column.c strtonum.h
	$(CC) -c -o column.o column.c

strtonum.o:	strtonum.c strtonum.h
	$(CC) -c -o strtonum.o strtonum.c

clean:
	rm -f column column.o strtonum.o

install:
	install -d -m 755 column $(DESTDIR)/bin
