all:	finger

finger: finger.o lprint.o net.o sprint.o util.o display.o
	$(CC) -o finger finger.o lprint.o net.o sprint.o util.o display.o

finger.o:	finger.c finger.h
	$(CC) -c -o finger.o finger.c

lprint.o:	lprint.c finger.h
	$(CC) -c -o lprint.o lprint.c

net.o:	net.c finger.h
	$(CC) -c -o net.o net.c

sprint.o:	sprint.c finger.h
	$(CC) -c -o sprint.o sprint.c

util.o:	util.c finger.h
	$(CC) -c -o util.o util.c

display.o: display.c finger.h
	$(CC) -c -o display.o display.c

clean:
	rm -f finger finger.o lprint.o net.o sprint.o util.o display.o

install:
	install -d -m 755 finger $(DESTDIR)/bin
