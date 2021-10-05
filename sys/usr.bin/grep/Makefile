grep:	binary.o fgetln.o file.o grep.o mmfile.o queue.o util.o
	$(CC) -o grep binary.o fgetln.o file.o grep.o mmfile.o queue.o util.o -lz

binary.o:	binary.c grep.h
	$(CC) -c -o binary.o binary.c

fgetln.o:	fgetln.c
	$(CC) -c -o fgetln.o fgetln.c

file.o:	file.c grep.h
	$(CC) -c -o file.o file.c

grep.o:	grep.c grep.h
	$(CC) -c -o grep.o grep.c

mmfile.o:	mmfile.c grep.h
	$(CC) -c -o mmfile.o mmfile.c

queue.o:	queue.c grep.h
	$(CC) -c -o queue.o queue.c

util.o:	util.c grep.h
	$(CC) -c -o util.o util.c

clean:
	rm -f *.o grep

install:
	install -d -m 755 grep ${DESTDIR}/usr/bin
