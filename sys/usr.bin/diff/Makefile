diff:	diff.o
	$(CC) -o diff diff.o

diff.o:	diff.c
	$(CC) -c -o diff.o diff.c

clean:
	rm -f diff diff.o

install:
	install -d -m 755 diff /bin

