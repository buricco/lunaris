all:	printenv

printenv:	printenv.o
	$(CC) -o printenv printenv.o

printenv.o:	printenv.c
	$(CC) -c -o printenv.o printenv.c

clean:
	rm -f printenv printenv.o

install:
	install -d -m 755 printenv $(DESTDIR)/bin
