all:	pwd

pwd:	pwd.o
	$(CC) -o pwd pwd.o

pwd.o:	pwd.c
	$(CC) -c -o pwd.o pwd.c

clean:
	rm -f pwd pwd.o

install:
	install -d -m 755 pwd $(DESTDIR)/bin
