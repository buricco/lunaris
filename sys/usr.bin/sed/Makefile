sed:    compile.o fgetln.o main.o misc.o process.o strtonum.o
	$(CC) -o sed compile.o fgetln.o main.o misc.o process.o strtonum.o

compile.o:	compile.c defs.h extern.h
	$(CC) -c compile.c

fgetln.o:	fgetln.c
	$(CC) -c fgetln.c

main.o:	main.c defs.h extern.h strtonum.h
	$(CC) -c main.c

misc.o:	misc.c defs.h extern.h
	$(CC) -c misc.c

process.o:	process.c defs.h extern.h
	$(CC) -c process.c

strtonum.o:	strtonum.c
	$(CC) -c strtonum.c

clean:
	rm -f compile.o fgetln.o main.o misc.o process.o strtonum.o sed
