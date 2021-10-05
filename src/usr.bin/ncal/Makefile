PROG   = ncal
SRC    = ncal.c calendar.c easter.c

# Configuration variables for bsdmainutils build system. These variables
# can be set here or through the environment.
CC      ?= cc
CFLAGS  ?= -O2 -g
DESTDIR ?=

# Each directory in usr.bin has the following targets avaiable: (none),
# install, clean. These targets also exist in the top level Makefile.

# Programs are able to specify additional libraries to link against, and
# are able to specify additional installation rules.
#  * To set additional linker flags, set the LDFLAGS variable in the
#    program Makefile.
#  * To specify a non-standard manpage, set the MAN variable.
#  * To add post-installation commands, define an install-2 target.
#  * To add additional sources, set the SRC variable.

# setup some defaults
SRC ?= $(PROG).c
MAN ?= $(PROG).1

sysconfdir=$(DESTDIR)/etc
datadir=$(DESTDIR)/usr/share
ifneq ($(findstring .6,$(MAN)),)
	bindir=$(DESTDIR)/usr/games
	mandir=$(datadir)/man/man6
else
	bindir=$(DESTDIR)/usr/bin
	mandir=$(datadir)/man/man1
endif

# rule for building the program
ifneq ($(findstring .sh,$(SRC)),)
$(PROG): $(SRC)
	cp $< $@
	chmod 0755 $@
else
objs=$(subst .c,.o,$(SRC))
$(PROG): $(objs)
	$(CC) $(LDFLAGS) -o $@ $^
endif

.c.o:
	$(CC) $(FLAGS) $(CFLAGS) -c -o $@ $<

# normal installation rule
install-1: $(PROG)
	install -o root -g root -m 755 $(PROG) $(bindir)
	install -o root -g root -m 644 $(MAN) $(mandir)

install: install-1 install-2

clean:
	-rm -f $(PROG) *.o

.PHONY: install-1 install-2 install clean

# vim:sw=4:ts=4:

install-2:
	(cd $(bindir); ln -sf ncal cal)
	(cd $(mandir); ln -sf ncal.1 cal.1)

install:
	install -d -m 755 ncal /usr/bin
