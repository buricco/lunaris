login:	login.o
	$(CC) -o login login.o -lcrypt -lutil

login.o:	login.c pathnames.h
	$(CC) -c -o login.o login.c

clean:
	rm -f login login.o

install:
	install -d -m 755 login ${DESTDIR}/usr/bin
