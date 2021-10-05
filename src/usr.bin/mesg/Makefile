mesg:	mesg.o
	$(CC) -o mesg mesg.o

mesg.o:	mesg.c
	$(CC) -c -o mesg.o mesg.c

clean:
	rm -f mesg mesg.o

install:
	install -d -m 755 mesg /bin

