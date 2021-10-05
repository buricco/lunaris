all:	time

time:	time.o
	$(CC) -o time time.o

time.o:	time.c
	$(CC) -c -o time.o time.c

clean:
	rm -f time time.o

install:
	install -d -m 755 time $(DESTDIR)/bin
