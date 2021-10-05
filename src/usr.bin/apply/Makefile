all:	apply

apply:	apply.o
	$(CC) -o apply apply.o

apply.o:	apply.c
	$(CC) -c -o apply.o apply.c

clean:
	rm -f apply apply.o

install:
	install -d -m 755 apply $(DESTDIR)/bin

