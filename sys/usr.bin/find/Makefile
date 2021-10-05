find:	find.o function.o ls.o main.o misc.o operator.o option.o strmode.o pwcache.o setmode.o
	$(CC) -o find find.o function.o ls.o main.o misc.o operator.o option.o strmode.o pwcache.o setmode.o

find.o:	find.c find.h extern.h
	$(CC) -c -o find.o find.c

function.o:	function.c find.h extern.h tzfile.h setmode.h
	$(CC) -c -o function.o function.c

ls.o:	ls.c tzfile.h
	$(CC) -c -o ls.o ls.c

main.o:	main.c find.h extern.h
	$(CC) -c -o main.o main.c

misc.o:	misc.c find.h extern.h
	$(CC) -c -o misc.o misc.c

operator.o:	operator.c find.h extern.h
	$(CC) -c -o operator.o operator.c

option.o:	option.c find.h extern.h
	$(CC) -c -o option.o option.c

strmode.o:	strmode.c
	$(CC) -c -o strmode.o strmode.c

pwcache.o:	pwcache.c
	$(CC) -c -o pwcache.o pwcache.c

setmode.o:	setmode.c setmode.h
	$(CC) -c -o setmode.o setmode.c

clean:
	rm -f find find.o function.o ls.o main.o misc.o operator.o option.o strmode.o pwcache.o setmode.o

install:
	install -d -m 755 find ${DESTDIR}/bin
