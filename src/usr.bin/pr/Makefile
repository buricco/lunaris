all:	pr

pr:	pr.o egetopt.o
	$(CC) -o pr pr.o egetopt.o

pr.o:	pr.c pr.h extern.h
	$(CC) -c -o pr.o pr.c

egetopt.o:	egetopt.c extern.h
	$(CC) -c -o egetopt.o egetopt.c

clean:
	rm -f pr pr.o egetopt.o

install:
	install -d -m 755 pr $(DESTDIR)/bin
