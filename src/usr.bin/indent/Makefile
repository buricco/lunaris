indent:	args.o indent.o io.o lexi.o parse.o pr_comment.o
	$(CC) -o indent args.o indent.o io.o lexi.o parse.o pr_comment.o

args.o:	args.c indent_globs.h
	$(CC) -c -o args.o args.c

indent.o:	indent.c indent_codes.h indent_globs.h
	$(CC) -c -o indent.o indent.c

io.o:	io.c indent_globs.h
	$(CC) -c -o io.o io.c

lexi.o:	lexi.c indent_codes.h indent_globs.h
	$(CC) -c -o lexi.o lexi.c

parse.o:	parse.c indent_codes.h indent_globs.h
	$(CC) -c -o parse.o parse.c

pr_comment.o:	pr_comment.c indent_globs.h
	$(CC) -c -o pr_comment.o pr_comment.c

clean:
	rm -f *.o indent

install:
	install -d -m 755 indent ${DESTDIR}/usr/bin
