ctags:	c.o ctags.o fortran.o lisp.o print.o tree.o yacc.o
	$(CC) -o ctags c.o ctags.o fortran.o lisp.o print.o tree.o yacc.o

c.o: c.c ctags.h
	$(CC) -c -o c.o c.c

ctags.o: ctags.c ctags.h
	$(CC) -c -o ctags.o ctags.c

fortran.o: fortran.c ctags.h
	$(CC) -c -o fortran.o fortran.c

lisp.o: lisp.c ctags.h
	$(CC) -c -o lisp.o lisp.c

print.o: print.c ctags.h
	$(CC) -c -o print.o print.c

tree.o: tree.c ctags.h
	$(CC) -c -o tree.o tree.c

yacc.o: yacc.c ctags.h
	$(CC) -c -o yacc.o yacc.c

clean:
	rm -f *.o ctags c.o ctags.o fortran.o lisp.o print.o tree.o yacc.o

install:
	install -d -m 755 ctags ${DESTDIR}/bin
