ls:	cmp.o humanize_number.o ls.o print.o pwcache.o strmode.o util.o vis.o
	$(CC) -o ls cmp.o humanize_number.o ls.o print.o pwcache.o strmode.o util.o vis.o

cmp.o:	cmp.c extern.h ls.h
	$(CC) -c -o cmp.o cmp.c

humanize_number.o:	humanize_number.c humanize_number.h
	$(CC) -c -o humanize_number.o humanize_number.c

ls.o:	ls.c extern.h ls.h pwcache.h
	$(CC) -c -o ls.o ls.c

print.o:	print.c extern.h humanize_number.h ls.h tzfile.h
	$(CC) -c -o print.o print.c

pwcache.o:	pwcache.c pwcache.h
	$(CC) -c -o pwcache.o pwcache.c

strmode.o:	strmode.c
	$(CC) -c -o strmode.o strmode.c

util.o:	util.c extern.h ls.h vis.h
	$(CC) -c -o util.o util.c

vis.o:	vis.c vis.h
	$(CC) -c -o vis.o vis.c

clean:
	rm -f ls cmp.o humanize_number.o ls.o print.o pwcache.o strmode.o util.o vis.o

install:
	install -d -m 755 ls $(DESTDIR)/bin
