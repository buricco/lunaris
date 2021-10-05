rdate:	rdate.o
	$(CC) -o rdate rdate.o -lutil

rdate.o:	rdate.c
	$(CC) -c -o rdate.o rdate.c

clean:
	rm -f rdate rdate.o

install:
	install -d -m 755 rdate /bin

