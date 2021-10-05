wump:	wump.o
	$(CC) -o wump wump.o

wump.o:	wump.c pathnames.h
	$(CC) -c -o wump.o wump.c

clean:
	rm -f wump wump.o

install:
	install -d -m 755 wump /bin

