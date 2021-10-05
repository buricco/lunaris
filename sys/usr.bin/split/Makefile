all:	split

split:	split.o
	$(CC) -o split split.o

split.o:	split.c
	$(CC) -c -o split.o split.c

clean:
	rm -f split split.o

install:
	install -d -m 755 split $(DESTDIR)/bin
