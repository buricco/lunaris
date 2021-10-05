all:	seq

seq:	seq.o
	$(CC) -o seq seq.o -lm

seq.o:	seq.c
	$(CC) -c -o seq.o seq.c

clean:
	rm -f seq seq.o

install:
	install -d -m 755 seq $(DESTDIR)/bin
