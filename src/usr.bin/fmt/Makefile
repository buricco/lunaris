all:	fmt

fmt:	fmt.o head.o
	$(CC) -o fmt fmt.o head.o

fmt.o:	fmt.c
	$(CC) -c -o fmt.o fmt.c

head.o:	head.c def.h
	$(CC) -c -o head.o head.c

clean:
	rm -f fmt fmt.o head.o

install:
	install -d -m 755 fmt $(DESTDIR)/bin
