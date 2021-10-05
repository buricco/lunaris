compress:	compress.o gzopen.o nullopen.o zopen.o bzopen.o
	$(CC) -o compress compress.o gzopen.o nullopen.o zopen.o bzopen.o -lz -lbz2

compress.o:	compress.c compress.h bzopen.h gzopen.h nullopen.h zopen.h
	$(CC) -c -o compress.o compress.c

gzopen.o:	gzopen.c compress.h gzopen.h
	$(CC) -c -o gzopen.o gzopen.c

nullopen.o:	nullopen.c compress.h nullopen.h
	$(CC) -c -o nullopen.o nullopen.c

zopen.o:	zopen.c compress.h zopen.h
	$(CC) -c -o zopen.o zopen.c

bzopen.o:	bzopen.c compress.h bzopen.h
	$(CC) -c -o bzopen.o bzopen.c

clean:
	rm -f compress compress.o gzopen.o nullopen.o zopen.o bzopen.o
