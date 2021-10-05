all:	cp

cp:	cp.o sfile.o memalign.o
	$(CC) -o cp cp.o sfile.o memalign.o

cp.o:	cp.c sfile.h memalign.h
	$(CC) -c -o cp.o cp.c

sfile.o:	sfile.c sfile.h
	$(CC) -c -o sfile.o sfile.c

memalign.o:	memalign.c memalign.h
	$(CC) -c -o memalign.o memalign.c

clean:
	rm -f cp cp.o sfile.o memalign.o

install:
	install -d -m 755 cp $(DESTDIR)/bin
	ln -s cp $(DESTDIR)/bin/mv
	ln -s cp $(DESTDIR)/bin/ln
