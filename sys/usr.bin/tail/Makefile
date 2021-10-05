all:	tail

tail:	forward.o misc.o read.o reverse.o tail.o
	$(CC) -o tail forward.o misc.o read.o reverse.o tail.o

forward.o:	forward.c extern.h
	$(CC) -c -o forward.o forward.c

misc.o:	misc.c extern.h
	$(CC) -c -o misc.o misc.c

read.o:	read.c extern.h
	$(CC) -c -o read.o read.c

reverse.o:	reverse.c extern.h
	$(CC) -c -o reverse.o reverse.c

tail.o:	tail.c extern.h
	$(CC) -c -o tail.o tail.c

clean:
	rm -f tail forward.o misc.o read.o reverse.o tail.o

install:
	install -d -m 755 tail $(DESTDIR)/bin
