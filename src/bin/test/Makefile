all:	test

test:   test.o strtonum.o
	$(CC) -o test test.o strtonum.o

test.o:	test.c strtonum.h
	$(CC) -c -o test.o test.c

strtonum.o:	strtonum.c strtonum.h
	$(CC) -c -o strtonum.o strtonum.c

clean:
	rm -f test test.o strtonum.o

install:
	install -d -m 755 test $(DESTDIR)/bin
