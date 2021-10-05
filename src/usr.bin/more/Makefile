LDFLAGS+=-ltermcap

more:	more.o xstrncpy.o

more.o:	more.c xstrncpy.h

xstrncpy.o:	xstrncpy.c xstrncpy.h

clean:
	rm -f more *.o

install:
	install -d -m 755 more ${DESTDIR}/usr/bin
