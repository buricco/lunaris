all:	dd

dd:	dd.o
	$(CC) -o dd dd.o

dd.o:	dd.c
	$(CC) -c -o dd.o dd.c

clean:
	rm -f dd dd.o

install:
	install -d -m 755 dd $(DESTDIR)/bin
