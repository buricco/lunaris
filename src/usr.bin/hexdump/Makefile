CFLAGS= -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64

all:	hexdump

hexdump:	conv.o display.o hexdump.o hexsyntax.o odsyntax.o parse.o
	$(CC) -o hexdump conv.o display.o hexdump.o hexsyntax.o odsyntax.o parse.o

conv.o:	conv.c hexdump.h
	$(CC) $(CFLAGS) -c -o conv.o conv.c

display.o:	display.c hexdump.h
	$(CC) $(CFLAGS) -c -o display.o display.c

hexdump.o:	hexdump.c hexdump.h
	$(CC) $(CFLAGS) -c -o hexdump.o hexdump.c

hexsyntax.o:	hexsyntax.c hexdump.h
	$(CC) $(CFLAGS) -c -o hexsyntax.o hexsyntax.c

odsyntax.o:	odsyntax.c hexdump.h
	$(CC) $(CFLAGS) -c -o odsyntax.o odsyntax.c

parse.o:	parse.c hexdump.h
	$(CC) $(CFLAGS) -c -o parse.o parse.c

clean:
	rm -f hexdump conv.o display.o hexdump.o hexsyntax.o odsyntax.o parse.o

install:
	install -d -m 755 hexdump $(DESTDIR)/bin
	ln -s hexdump $(DESTDIR)/bin/hd
	ln -s hexdump $(DESTDIR)/bin/od
