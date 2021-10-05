all:	script

script:	script.o
	$(CC) -o script script.o -lutil

script.o:	script.c
	$(CC) -c -o script.o script.c

clean:
	rm -f script script.o

install:
	install -d -m 755 script $(DESTDIR)/bin
