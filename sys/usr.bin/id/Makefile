all:	id

id:	id.o
	$(CC) -o id id.o

id.o:	id.c
	$(CC) -c -o id.o id.c

clean:
	rm -f id id.o

install:
	install -d -m 755 id $(DESTDIR)/bin
