wall:	wall.o strutils.o ttymsg.o
	$(CC) -o wall wall.o strutils.o ttymsg.o

wall.o:	wall.c strutils.h ttymsg.h
	$(CC) -c -o wall.o wall.c

strutils.o:	strutils.c strutils.h
	$(CC) -c -o strutils.o strutils.c

ttymsg.o:	ttymsg.c ttymsg.h
	$(CC) -c -o ttymsg.o ttymsg.c

clean:
	rm -f wall wall.o strutils.o ttymsg.o
