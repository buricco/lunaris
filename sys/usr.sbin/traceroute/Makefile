traceroute:	traceroute.o
	$(CC) -o traceroute traceroute.o

traceroute.o:	traceroute.c
	$(CC) -c -o traceroute.o traceroute.c

clean:
	rm -f traceroute traceroute.o

install:
	install -d -m 755 traceroute /bin

