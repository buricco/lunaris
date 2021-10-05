ps:	ps.o
	$(CC) -o ps ps.o

ps.o:	ps.c
	$(CC) -c -o ps.o ps.c

clean:
	rm -f ps ps.o

install:
	install -d -m 755 ps /bin

