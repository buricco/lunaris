all:	chroot

chroot:	chroot.o
	$(CC) -o chroot chroot.o

chroot.o:	chroot.c
	$(CC) -c -o chroot.o chroot.c

clean:
	rm -f chroot chroot.o

install:
	install -d -m 755 chroot $(DESTDIR)/bin
