all:	touch

touch:	touch.o
	$(CC) -o touch touch.o

touch.o:	touch.c
	$(CC) -c -o touch.o touch.c

clean:
	rm -f touch touch.o

install:
	install -d -m 755 touch $(DESTDIR)/bin
