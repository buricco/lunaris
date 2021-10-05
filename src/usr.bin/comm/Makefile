all:	comm

comm:	comm.o
	$(CC) -o comm comm.o

comm.o:	comm.c
	$(CC) -c -o comm.o comm.c

clean:
	rm -f comm comm.o

install:
	install -d -m 755 comm $(DESTDIR)/bin
