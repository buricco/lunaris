all:	mkfifo

mkfifo:	mkfifo.o
	$(CC) -o mkfifo mkfifo.o

mkfifo.o:	mkfifo.c
	$(CC) -c -o mkfifo.o mkfifo.c

clean:
	rm -f mkfifo mkfifo.o

install:
	install -d -m 755 mkfifo $(DESTDIR)/bin
