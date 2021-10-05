all:	colrm

colrm:	colrm.o
	$(CC) -o colrm colrm.o

colrm.o:	colrm.c
	$(CC) -c -o colrm.o colrm.c

clean:
	rm -f colrm colrm.o

install:
	install -d -m 755 colrm $(DESTDIR)/bin
