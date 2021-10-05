rain:	rain.o
	$(CC) -o rain rain.o -lcurses

rain.o:	rain.c
	$(CC) -c -o rain.o rain.c

clean:
	rm -f rain rain.o

install:
	install -d -m 755 rain /bin

