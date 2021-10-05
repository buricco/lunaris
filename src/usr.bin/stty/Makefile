all:	stty

stty:	stty.o
	$(CC) -o stty stty.o

stty.o:	stty.c
	$(CC) -c -o stty.o stty.c

clean:
	rm -f stty stty.o

install:
	install -D -m 755 stty $(DESTDIR)/bin/stty
