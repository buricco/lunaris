ed:	buf.o glbl.o io.o main.o re.o sub.o undo.o
	$(CC) -o ed buf.o glbl.o io.o main.o re.o sub.o undo.o

buf.o:	buf.c ed.h
	$(CC) -c -o buf.o buf.c

glbl.o:	glbl.c ed.h
	$(CC) -c -o glbl.o glbl.c

io.o:	io.c ed.h
	$(CC) -c -o io.o io.c

main.o:	main.c ed.h
	$(CC) -c -o main.o main.c

re.o:	re.c ed.h
	$(CC) -c -o re.o re.c

sub.o:	sub.c ed.h
	$(CC) -c -o sub.o sub.c

undo.o:	undo.c ed.h
	$(CC) -c -o undo.o undo.c

clean:
	rm -f ed buf.o glbl.o io.o main.o re.o sub.o undo.o
