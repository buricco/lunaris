bc:	y.tab.o lex.yy.o execute.o global.o load.o main.o number.o storage.o util.o warranty.o
	$(CC) -o bc y.tab.o lex.yy.o execute.o global.o load.o main.o number.o storage.o util.o warranty.o

lex.yy.o:	lex.yy.c
	$(CC) -c -o lex.yy.o lex.yy.c

lex.yy.c:	scan.l y.tab.c bcdefs.h config.h global.h proto.h
	lex scan.l

y.tab.o:	y.tab.c
	$(CC) -c -o y.tab.o y.tab.c

y.tab.c:	bc.y bcdefs.h config.h global.h proto.h
	yacc -d bc.y

execute.o:	execute.c bcdefs.h config.h proto.h
	$(CC) -c -o execute.o execute.c

global.o:	global.c bcdefs.h config.h libmath.h
	$(CC) -c -o global.o global.c

load.o:	load.c bcdefs.h config.h proto.h
	$(CC) -c -o load.o load.c

main.o:	main.c bcdefs.h config.h proto.h
	$(CC) -c -o main.o main.c

number.o:	number.c config.h number.h
	$(CC) -c -o number.o number.c

storage.o:	storage.c bcdefs.h config.h proto.h
	$(CC) -c -o storage.o storage.c

util.o:	util.c bcdefs.h config.h proto.h
	$(CC) -c -o util.o util.c

warranty.o:	warranty.c bcdefs.h config.h proto.h
	$(CC) -c -o warranty.o warranty.c

clean:
	rm -f bc y.tab.o lex.yy.o y.tab.c y.tab.h lex.yy.c execute.o global.o load.o main.o number.o storage.o util.o warranty.o
