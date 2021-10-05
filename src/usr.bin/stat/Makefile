all:	stat

stat:	stat.o strmode.o vis.o
	$(CC) -o stat stat.o strmode.o vis.o

stat.o:	stat.c vis.h
	$(CC) -c -o stat.o stat.c

strmode.o:	strmode.c
	$(CC) -c -o strmode.o strmode.c

vis.o:	vis.c vis.h
	$(CC) -c -o vis.o vis.c

clean:
	rm -f stat stat.o strmode.o vis.o

install:
	install -d -m 755 stat $(DESTDIR)/bin
	ln -s stat $(DESTDIR)/bin/readlink
