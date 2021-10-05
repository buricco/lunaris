all:	tee

tee:	tee.o
	$(CC) -o tee tee.o

tee.o:	tee.c
	$(CC) -c -o tee.o tee.c

clean:
	rm -f tee tee.o

install:
	install -d -m 755 tee $(DESTDIR)/bin
