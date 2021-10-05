cmp:	cmp.o misc.o regular.o special.o
	$(CC) -o cmp cmp.o misc.o regular.o special.o

cmp.o:	cmp.c extern.h
	$(CC) -c -o cmp.o cmp.c

misc.o:	misc.c extern.h
	$(CC) -c -o misc.o misc.c

regular.o:	regular.c extern.h
	$(CC) -c -o regular.o regular.c

special.o:	special.c extern.h
	$(CC) -c -o special.o special.c

clean:
	rm -f cmp cmp.o misc.o regular.o special.o
