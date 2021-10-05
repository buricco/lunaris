last:	last.o
	$(CC) -o last last.o

last.o:	last.c tzfile.h
	$(CC) -c -o last.o last.c

clean:
	rm -f last last.o

install:
	install -d -m 755 last /bin

