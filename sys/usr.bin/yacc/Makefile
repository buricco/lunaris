yacc:   closure.o error.o graph.o lalr.o lr0.o main.o mkpar.o mstring.o output.o reader.o yaccpar.o symtab.o verbose.o warshall.o
	$(CC) -o yacc closure.o error.o graph.o lalr.o lr0.o main.o mkpar.o mstring.o output.o reader.o yaccpar.o symtab.o verbose.o warshall.o

closure.o:	closure.c defs.h
	$(CC) -c -o closure.o closure.c

error.o:	error.c defs.h
	$(CC) -c -o error.o error.c

graph.o:	graph.c defs.h
	$(CC) -c -o graph.o graph.c

lalr.o:	lalr.c defs.h
	$(CC) -c -o lalr.o lalr.c

lr0.o:	lr0.c defs.h
	$(CC) -c -o lr0.o lr0.c

main.o:	main.c defs.h
	$(CC) -c -o main.o main.c

mkpar.o:	mkpar.c defs.h
	$(CC) -c -o mkpar.o mkpar.c

mstring.o:	mstring.c defs.h
	$(CC) -c -o mstring.o mstring.c

output.o:	output.c defs.h
	$(CC) -c -o output.o output.c

reader.o:	reader.c defs.h
	$(CC) -c -o reader.o reader.c

yaccpar.o:	yaccpar.c defs.h
	$(CC) -c -o yaccpar.o yaccpar.c

symtab.o:	symtab.c defs.h
	$(CC) -c -o symtab.o symtab.c

verbose.o:	verbose.c defs.h
	$(CC) -c -o verbose.o verbose.c

warshall.o:	warshall.c defs.h
	$(CC) -c -o warshall.o warshall.c

yaccpar.c:	skel2c
	awk -f skel2c < yaccpar.skel > yaccpar.c

clean:
	rm -f yacc closure.o error.o graph.o lalr.o lr0.o main.o mkpar.o mstring.o output.o reader.o yaccpar.o symtab.o verbose.o warshall.o yaccpar.c
