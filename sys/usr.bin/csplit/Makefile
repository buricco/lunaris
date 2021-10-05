all:	csplit

csplit:	csplit.o
	$(CC) -o csplit csplit.o

csplit.o:	csplit.c
	$(CC) -c -o csplit.o csplit.c

clean:
	rm -f csplit csplit.o

install:
	install -d -m 755 csplit $(DESTDIR)/bin
