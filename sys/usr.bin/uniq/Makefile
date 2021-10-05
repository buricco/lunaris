all:	uniq

uniq:	uniq.o
	$(CC) -o uniq uniq.o

uniq.o:	uniq.c
	$(CC) -c -o uniq.o uniq.c

clean:
	rm -f uniq uniq.o

install:
	install -D -m 755 uniq $(DESTDIR)/bin/uniq
