monop:	cards.o execute.o getinp.o houses.o jail.o misc.o monop.o morg.o print.o prop.o rent.o roll.o spec.o trade.o
	cc -o monop cards.o execute.o getinp.o houses.o jail.o misc.o monop.o morg.o print.o prop.o rent.o roll.o spec.o trade.o

cards.o:	cards.c monop.h deck.h
	cc -c -o cards.o cards.c

execute.o:	execute.c monop.h deck.h
	cc -c -o execute.o execute.c

getinp.o:	getinp.c monop.h
	cc -c -o getinp.o getinp.c

houses.o:	houses.c monop.h
	cc -c -o houses.o houses.c

jail.o:	jail.c monop.h
	cc -c -o jail.o jail.c

misc.o:	misc.c monop.h
	cc -c -o misc.o misc.c

monop.o:	monop.c monop.h deck.h
	cc -c -o monop.o monop.c

morg.o:	morg.c monop.h
	cc -c -o morg.o morg.c

print.o:	print.c monop.h
	cc -c -o print.o print.c

prop.o:	prop.c monop.h
	cc -c -o prop.o prop.c

rent.o:	rent.c monop.h
	cc -c -o rent.o rent.c

roll.o:	roll.c monop.h
	cc -c -o roll.o roll.c

spec.o:	spec.c monop.h deck.h
	cc -c -o spec.o spec.c

trade.o:	trade.c monop.h
	cc -c -o trade.o trade.c

clean:
	rm -f monop cards.o execute.o getinp.o houses.o jail.o misc.o monop.o morg.o print.o prop.o rent.o roll.o spec.o trade.o
