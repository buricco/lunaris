all:	primes factor

primes:	primes.o pattern.o pr_tbl.o
	$(CC) -o primes primes.o pattern.o pr_tbl.o -lm

factor:	factor.o pr_tbl.o
	$(CC) -o factor factor.o pr_tbl.o -lm

primes.o:	primes.c
	$(CC) -c -o primes.o primes.c

pattern.o:	pattern.c
	$(CC) -c -o pattern.o pattern.c

factor.o:	factor.c primes.h
	$(CC) -c -o factor.o factor.c

pr_tbl.o:	pr_tbl.c primes.h
	$(CC) -c -o pr_tbl.o pr_tbl.c

clean:
	rm -f factor primes primes.o pattern.o pr_tbl.o factor.o

install:
	install -d -m 755 factor $(DESTDIR)/bin
	install -d -m 755 primes $(DESTDIR)/bin
