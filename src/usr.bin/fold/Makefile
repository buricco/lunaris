all:	fold

fold:	fold.o
	$(CC) -o fold fold.o

fold.o:	fold.c
	$(CC) -c -o fold.o fold.c

clean:
	rm -f fold fold.o

install:
	install -d -m 755 fold $(DESTDIR)/bin
