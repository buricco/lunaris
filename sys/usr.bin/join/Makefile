all:	join

join:	join.o fgetln.o
	$(CC) -o join join.o fgetln.o

join.o:	join.c
	$(CC) -c -o join.o join.c

fgetln.o: fgetln.c
	$(CC) -c -o fgetln.o fgetln.c

clean:
	rm -f join join.o fgetln.o

install:
	install -d -m 755 join $(DESTDIR)/bin
