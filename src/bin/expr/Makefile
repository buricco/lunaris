all:	expr

expr:   expr.o strtonum.o
	$(CC) -o expr expr.o strtonum.o

expr.o:	expr.c strtonum.h
	$(CC) -c -o expr.o expr.c

strtonum.o:	strtonum.c strtonum.h
	$(CC) -c -o strtonum.o strtonum.c

clean:
	rm -f expr expr.o strtonum.o

install:
	install -d -m 755 expr $(DESTDIR)/bin
