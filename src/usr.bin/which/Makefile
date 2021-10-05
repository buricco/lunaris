which:	which.o
	$(CC) -o which which.o

which.o:	which.c
	$(CC) -c -o which.o which.c

clean:
	rm -f which which.o

install:
	install -d -m 755 which /bin

