all:	getopt

getopt:	getopt.o
	$(CC) -o getopt getopt.o

getopt.o:	getopt.c
	$(CC) -c -o getopt.o getopt.c

clean:
	rm -f getopt getopt.o

install:
	install -d -m 755 getopt $(DESTDIR)/bin
