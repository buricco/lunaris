all:	nohup

nohup:	nohup.o
	$(CC) -o nohup nohup.o

nohup.o:	nohup.c
	$(CC) -c -o nohup.o nohup.c

clean:
	rm -f nohup nohup.o

install:
	install -d -m 755 nohup $(DESTDIR)/bin
