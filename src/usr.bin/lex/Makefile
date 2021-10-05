lex:	parse.o ccl.o dfa.o ecs.o gen.o scan.o main.o misc.o nfa.o sym.o tblcmp.o yylex.o
	$(CC) -o lex parse.o ccl.o dfa.o ecs.o gen.o scan.o main.o misc.o nfa.o sym.o tblcmp.o yylex.o

scan.o: scan.c flexdef.h parse.h
	$(CC) -c -o scan.o scan.c

scan.c:	scan.l lex1 lex.skel
	./lex1 -Slex.skel scan.l
	mv lex.yy.c scan.c

lex1:	parse.o ccl.o dfa.o ecs.o gen.o initscan.o main.o misc.o nfa.o sym.o tblcmp.o yylex.o
	$(CC) -o lex1 parse.o ccl.o dfa.o ecs.o gen.o initscan.o main.o misc.o nfa.o sym.o tblcmp.o yylex.o

ccl.o:	ccl.c flexdef.h
	$(CC) -c -o ccl.o ccl.c

dfa.o:	dfa.c flexdef.h
	$(CC) -c -o dfa.o dfa.c

ecs.o:	ecs.c flexdef.h
	$(CC) -c -o ecs.o ecs.c

gen.o:	gen.c flexdef.h
	$(CC) -c -o gen.o gen.c

initscan.o:	initscan.c flexdef.h
	$(CC) -c -o initscan.o initscan.c

main.o:	main.c flexdef.h
	$(CC) -c -o main.o main.c

misc.o:	misc.c flexdef.h
	$(CC) -c -o misc.o misc.c

nfa.o:	nfa.c flexdef.h
	$(CC) -c -o nfa.o nfa.c

sym.o:	sym.c flexdef.h
	$(CC) -c -o sym.o sym.c

tblcmp.o:	tblcmp.c flexdef.h
	$(CC) -c -o tblcmp.o tblcmp.c

yylex.o:	yylex.c flexdef.h
	$(CC) -c -o yylex.o yylex.c

parse.o:	parse.c parse.h flexdef.h
	$(CC) -c -o parse.o parse.c

parse.c parse.h:	parse.y
	yacc -d parse.y
	mv y.tab.c parse.c
	mv y.tab.h parse.h

clean:
	rm -f parse.c parse.h scan.c lex lex1 *.o

install:
	install -d -m 755 lex ${DESTDIR}/usr/bin
