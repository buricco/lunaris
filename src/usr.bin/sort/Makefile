all:	sort

sort:	append.o ealloc.o fields.o files.o fsort.o init.o msort.o radix_sort.o sort.o tmp.o
	$(CC) -o sort append.o ealloc.o fields.o files.o fsort.o init.o msort.o radix_sort.o sort.o tmp.o

append.o:	append.c sort.h
	$(CC) -c -o append.o append.c

ealloc.o:	ealloc.c
	$(CC) -c -o ealloc.o ealloc.c

fields.o:	fields.c sort.h
	$(CC) -c -o fields.o fields.c

files.o:	files.c fsort.h sort.h
	$(CC) -c -o files.o files.c

fsort.o:	fsort.c fsort.h sort.h
	$(CC) -c -o fsort.o fsort.c

init.o:	init.c sort.h
	$(CC) -c -o init.o init.c

msort.o:	msort.c fsort.h sort.h
	$(CC) -c -o msort.o msort.c

radix_sort.o:	radix_sort.c
	$(CC) -c -o radix_sort.o radix_sort.c

sort.o:	sort.c pathnames.h fsort.h sort.h
	$(CC) -c -o sort.o sort.c

tmp.o:	tmp.c pathnames.h sort.h
	$(CC) -c -o tmp.o tmp.c

clean:
	rm -f sort append.o ealloc.o fields.o files.o fsort.o init.o msort.o radix_sort.o sort.o tmp.o

install:
	install -d -m 755 sort $(DESTDIR)/bin
