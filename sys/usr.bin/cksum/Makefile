all:	cksum

cksum:	cksum.o crc.o md2.o md4.o md5.o print.o rmd160.o sha1.o sha256.o sha384.o sha512.o sum1.o sum2.o libcksum/libcksum.a
	$(CC) -o cksum cksum.o crc.o md2.o md4.o md5.o print.o rmd160.o sha1.o sha256.o sha384.o sha512.o sum1.o sum2.o -Llibcksum -lcrypto -lcksum

libcksum/libcksum.a:
	make -C libcksum

cksum.o:	cksum.c extern.h crc_extern.h
	$(CC) -Ilibcksum -c -o cksum.o cksum.c

crc.o:	crc.c extern.h crc_extern.h
	$(CC) -Ilibcksum -c -o crc.o crc.c

md2.o:	md2.c md5.c
	$(CC) -Ilibcksum -c -o md2.o md2.c

md4.o:	md4.c md5.c
	$(CC) -Ilibcksum -c -o md4.o md4.c

md5.o:	md5.c
	$(CC) -Ilibcksum -c -o md5.o md5.c

print.o:	print.c extern.h crc_extern.h
	$(CC) -Ilibcksum -c -o print.o print.c

rmd160.o:	rmd160.c md5.c
	$(CC) -Ilibcksum -c -o rmd160.o rmd160.c

sha1.o:	sha1.c md5.c
	$(CC) -Ilibcksum -c -o sha1.o sha1.c

sha256.o:	sha256.c md5.c
	$(CC) -Ilibcksum -c -o sha256.o sha256.c

sha384.o:	sha384.c md5.c
	$(CC) -Ilibcksum -c -o sha384.o sha384.c

sha512.o:	sha512.c md5.c
	$(CC) -Ilibcksum -c -o sha512.o sha512.c

sum1.o:	sum1.c extern.h crc_extern.h
	$(CC) -Ilibcksum -c -o sum1.o sum1.c

sum2.o:	sum2.c extern.h crc_extern.h
	$(CC) -Ilibcksum -c -o sum2.o sum2.c

clean:
	rm -f cksum cksum.o crc.o md2.o md4.o md5.o print.o rmd160.o sha1.o sha256.o sha384.o sha512.o sum1.o sum2.o
	make -C libcksum clean

install:
	install -d -m 755 cksum $(DESTDIR)/bin
	ln -s cksum $(DESTDIR)/bin/sum
	ln -s cksum $(DESTDIR)/bin/md2
	ln -s cksum $(DESTDIR)/bin/md4
	ln -s cksum $(DESTDIR)/bin/md5
	ln -s cksum $(DESTDIR)/bin/rmd160
	ln -s cksum $(DESTDIR)/bin/sha1
	ln -s cksum $(DESTDIR)/bin/sha256
	ln -s cksum $(DESTDIR)/bin/sha384
	ln -s cksum $(DESTDIR)/bin/sha512
