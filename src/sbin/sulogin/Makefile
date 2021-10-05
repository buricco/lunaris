# $Owl: Owl/packages/msulogin/msulogin/Makefile,v 1.3 2012/08/15 06:34:45 solar Exp $

CC = gcc
LD = gcc
MKDIR = mkdir -p
INSTALL = install -c
CFLAGS = -c -Wall -W -O2 -fomit-frame-pointer
LDFLAGS = -s
LIBS = -lcrypt

DESTDIR =
SBINDIR = /sbin
MANDIR = /usr/share/man

PROJ = sulogin
OBJS = sulogin.o

all: $(PROJ)

sulogin: $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) $(LIBS) -o sulogin

.c.o:
	$(CC) $(CFLAGS) $*.c

install: $(PROJ)
	$(MKDIR) -m 755 $(DESTDIR)$(SBINDIR) $(DESTDIR)$(MANDIR)/man8
	$(INSTALL) -m 700 sulogin $(DESTDIR)$(SBINDIR)/
	$(INSTALL) -m 644 sulogin.8 $(DESTDIR)$(MANDIR)/man8/

clean:
	$(RM) $(PROJ) $(OBJS)
