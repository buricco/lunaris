all:	libcksum.a

libcksum.a:	md2.o md2hl.o md4c.o md4hl.o md5c.o md5hl.o rmd160.o rmd160hl.o sha1.o sha1hl.o sha224hl.o sha256hl.o sha384hl.o sha512hl.o
	ar rv libcksum.a md2.o md2hl.o md4c.o md4hl.o md5c.o md5hl.o rmd160.o rmd160hl.o sha1.o sha1hl.o sha224hl.o sha256hl.o sha384hl.o sha512hl.o

md2.o:	md2.c md2.h
	$(CC) -c -o md2.o md2.c

md2hl.o:	md2hl.c md2.h hashhl.c
	$(CC) -c -o md2hl.o md2hl.c

md4c.o:	md4c.c md4.h
	$(CC) -c -o md4c.o md4c.c

md4hl.o:	md4hl.c md4.h mdXhl.c
	$(CC) -c -o md4hl.o md4hl.c

md5c.o:	md5c.c md5.h
	$(CC) -c -o md5c.o md5c.c

md5hl.o:	md5hl.c md5.h mdXhl.c
	$(CC) -c -o md5hl.o md5hl.c

rmd160.o:	rmd160.c rmd160.h
	$(CC) -c -o rmd160.o rmd160.c

rmd160hl.o:	rmd160hl.c rmd160.h hashhl.c
	$(CC) -c -o rmd160hl.o rmd160hl.c

sha1.o:	sha1.c sha1.h
	$(CC) -c -o sha1.o sha1.c

sha1hl.o:	sha1hl.c sha1.h hashhl.c
	$(CC) -c -o sha1hl.o sha1hl.c

sha224hl.o:	sha224hl.c sha2.h hashhl.c
	$(CC) -c -o sha224hl.o sha224hl.c

sha256hl.o:	sha256hl.c sha2.h hashhl.c
	$(CC) -c -o sha256hl.o sha256hl.c

sha384hl.o:	sha384hl.c sha2.h hashhl.c
	$(CC) -c -o sha384hl.o sha384hl.c

sha512hl.o:	sha512hl.c sha2.h hashhl.c
	$(CC) -c -o sha512hl.o sha512hl.c

clean:
	rm -f libcksum.a md2.o md2hl.o md4c.o md4hl.o md5c.o md5hl.o rmd160.o rmd160hl.o sha1.o sha1hl.o sha224hl.o sha256hl.o sha384hl.o sha512hl.o

install:
	true
