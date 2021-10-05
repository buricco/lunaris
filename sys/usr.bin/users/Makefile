users:	users.o
	$(CC) -o users users.o

users.o:	users.c
	$(CC) -c -o users.o users.c

clean:
	rm -f users users.o

install:
	install -d -m 755 users ${DESTDIR}/bin
