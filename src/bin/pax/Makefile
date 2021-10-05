pax:	ar_io.o ar_subs.o buf_subs.o cache.o cpio.o file_subs.o ftree.o gen_subs.o options.o pat_rep.o pax.o sel_subs.o strmode.o tables.o tar.o tty_subs.o
	$(CC) -o pax ar_io.o ar_subs.o buf_subs.o cache.o cpio.o file_subs.o ftree.o gen_subs.o options.o pat_rep.o pax.o sel_subs.o strmode.o tables.o tar.o tty_subs.o

ar_io.o:	ar_io.c pax.h extern.h
	$(CC) -c -o ar_io.o ar_io.c

ar_subs.o:	ar_subs.c pax.h extern.h
	$(CC) -c -o ar_subs.o ar_subs.c

buf_subs.o:	buf_subs.c pax.h extern.h
	$(CC) -c -o buf_subs.o buf_subs.c

cache.o:	cache.c cache.h pax.h extern.h
	$(CC) -c -o cache.o cache.c

cpio.o:	cpio.c cpio.h pax.h extern.h
	$(CC) -c -o cpio.o cpio.c

file_subs.o:	file_subs.c pax.h extern.h
	$(CC) -c -o file_subs.o file_subs.c

ftree.o:	ftree.c ftree.h pax.h extern.h
	$(CC) -c -o ftree.o ftree.c

gen_subs.o:	gen_subs.c pax.h extern.h tzfile.h
	$(CC) -c -o gen_subs.o gen_subs.c

options.o:	options.c options.h cpio.h tar.h pax.h extern.h
	$(CC) -c -o options.o options.c

pat_rep.o:	pat_rep.c pat_rep.h pax.h extern.h
	$(CC) -c -o pat_rep.o pat_rep.c

pax.o:	pax.c pax.h extern.h
	$(CC) -c -o pax.o pax.c

sel_subs.o:	sel_subs.c sel_subs.h pax.h extern.h
	$(CC) -c -o sel_subs.o sel_subs.c

strmode.o:	strmode.c
	$(CC) -c -o strmode.o strmode.c

tables.o:	tables.c tables.h pax.h extern.h
	$(CC) -c -o tables.o tables.c

tar.o:	tar.c tar.h pax.h extern.h
	$(CC) -c -o tar.o tar.c

tty_subs.o:	tty_subs.c pax.h extern.h
	$(CC) -c -o tty_subs.o tty_subs.c

clean:
	rm -f pax ar_io.o ar_subs.o buf_subs.o cache.o cpio.o file_subs.o ftree.o gen_subs.o options.o pat_rep.o pax.o sel_subs.o strmode.o tables.o tar.o tty_subs.o
