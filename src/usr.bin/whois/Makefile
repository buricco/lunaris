whois:	whois.o fgetln.o
	$(CC) -o whois whois.o fgetln.o

whois.o:	whois.c
	$(CC) -c -o whois.o -DRIPE whois.c

fgetln.o:	fgetln.c
	$(CC) -c -o fgetln.o -DRIPE fgetln.c

clean:
	rm -f whois whois.o fgetln.o

install:
	install -d -m 755 whois /usr/bin

