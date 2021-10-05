all:	mknod

mknod:	mknod.o
	$(CC) -o mknod mknod.o

mknod.o:	mknod.c
	$(CC) -c -o mknod.o mknod.c

clean:
	rm -f mknod mknod.o

install:
	install -d -m 755 mknod $(DESTDIR)/bin
