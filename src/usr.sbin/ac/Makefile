all:	ac accton

ac:	ac.o
	$(CC) -o ac ac.o

ac.o:	ac.c
	$(CC) -c -o ac.o ac.c

accton:	accton.o
	$(CC) -o accton accton.o

accton.o:	accton.c
	$(CC) -c -o accton.o accton.c

clean:
	rm -f ac ac.o accton accton.o

install:
	install -d -m 755 ac /bin
	install -d -m 755 accton /bin
