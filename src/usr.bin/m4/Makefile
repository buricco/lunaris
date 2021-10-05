m4:	eval.o expr.o gnum4.o look.o main.o misc.o trace.o
	$(CC) -o m4 eval.o expr.o gnum4.o look.o main.o misc.o trace.o

eval.o:	eval.c mdef.h stdd.h extern.h pathnames.h
	$(CC) -o eval.o -c eval.c

expr.o:	expr.c mdef.h extern.h
	$(CC) -o expr.o -c expr.c

gnum4.o:	gnum4.c mdef.h stdd.h extern.h
	$(CC) -o gnum4.o -c gnum4.c

look.o:	look.c mdef.h stdd.h extern.h
	$(CC) -o look.o -c look.c

main.o: main.c mdef.h stdd.h extern.h pathnames.h
	$(CC) -o main.o -c main.c

misc.o:	misc.c mdef.h stdd.h extern.h pathnames.h
	$(CC) -o misc.o -c misc.c

trace.o:	trace.c mdef.h stdd.h extern.h
	$(CC) -o trace.o -c trace.c

clean:
	rm -f m4 eval.o expr.o gnum4.o look.o main.o misc.o trace.o

install:
	install -d -m 755 m4 ${DESTDIR}/bin
