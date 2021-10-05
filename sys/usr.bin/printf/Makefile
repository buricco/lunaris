all:	printf

printf:	printf.o
	$(CC) -o printf printf.o

printf.o:	printf.c
	$(CC) -c -o printf.o printf.c

clean:
	rm -f printf printf.o

install:
	install -d -m 755 printf $(DESTDIR)/bin
