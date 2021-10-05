fish:	fish.o
	$(CC) -o fish fish.o

fish.o:	fish.c pathnames.h
	$(CC) -c -o fish.o fish.c

clean:
	rm -f fish fish.o

install:
	install -d -m 755 fish /bin

