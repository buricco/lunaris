tftp:	main.o tftp.o tftpsubs.o
	cc -o tftp main.o tftp.o tftpsubs.o

main.o:	main.c extern.h
	cc -c -o main.o main.c

tftp.o:	tftp.c extern.h tftpsubs.h
	cc -c -o tftp.o tftp.c

tftpsubs.o:	tftpsubs.c tftpsubs.h
	cc -c -o tftpsubs.o tftpsubs.c

clean:
	rm -f tftp main.o tftp.o tftpsubs.o

install:
	install -d -m 755 tftp ${DESTDIR}/bin
