all:	expand

expand:	expand.o
	$(CC) -o expand expand.o

expand.o:	expand.c
	$(CC) -c -o expand.o expand.c

clean:
	rm -f expand expand.o

install:
	install -d -m 755 expand $(DESTDIR)/bin
