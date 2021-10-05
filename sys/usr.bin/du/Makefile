all:	du

du:	du.o getdir.o
	$(CC) -o du du.o getdir.o

du.o:	du.c getdir.h
	$(CC) -c -o du.o du.c

getdir.o:	getdir.c getdir.h
	$(CC) -c -o getdir.o getdir.c

clean:
	rm -f du du.o getdir.o

install:
	install -d -m 755 du $(DESTDIR)/bin
