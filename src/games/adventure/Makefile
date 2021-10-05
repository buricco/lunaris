adventure: crc.o data.o done.o init.o io.o main.o save.o subr.o vocab.o wizard.o
	cc -o adventure crc.o data.o done.o init.o io.o main.o save.o subr.o vocab.o wizard.o

crc.o:	crc.c extern.h
	cc -c -o crc.o crc.c

data.o:	data.c
	cc -c -o data.o data.c

done.o:	done.c extern.h hdr.h
	cc -c -o done.o done.c

init.o:	init.c extern.h hdr.h
	cc -c -o init.o init.c

io.o:	io.c extern.h hdr.h
	cc -c -o io.o io.c

main.o:	main.c extern.h hdr.h
	cc -c -o main.o main.c

save.o:	save.c extern.h hdr.h
	cc -c -o save.o save.c

subr.o:	subr.c extern.h hdr.h
	cc -c -o subr.o subr.c

vocab.o:	vocab.c extern.h hdr.h
	cc -c -o vocab.o vocab.c

wizard.o:	wizard.c extern.h hdr.h
	cc -c -o wizard.o wizard.c

data.c:	setup glorkz
	./setup glorkz > data.c

setup:	setup.o
	cc -o setup setup.o

setup.o:	setup.c extern.h hdr.h
	cc -c -o setup.o setup.c

clean:
	rm -f adventure crc.o data.o done.o init.o io.o main.o save.o setup.o subr.o vocab.o wizard.o data.c setup
