# Use -ltermcap with your usual gcc-glibc-ncurses setup.
# With clang-musl-bsdcurses you will need -lterminfo instead.

all:	ex expreserve exrecover

ex:	ex_blk.o ex_cmd1.o ex_cmd2.o ex_ctype.o ex_curses.o ex_cut.o ex_ex.o  \
	ex_input.o ex_main.o ex_misc.o ex_modify.o ex_move1.o ex_move2.o      \
	ex_move3.o ex_move4.o ex_move5.o ex_opts.o ex_recycle.o ex_redraw.o   \
	ex_regexp.o ex_regsub.o ex_system.o ex_tio.o ex_tmp.o ex_unix.o       \
	ex_vars.o ex_vcmd.o ex_vi.o
	$(CC) -o ex ex_blk.o ex_cmd1.o ex_cmd2.o ex_ctype.o ex_curses.o       \
	      ex_cut.o ex_ex.o ex_input.o ex_main.o ex_misc.o ex_modify.o     \
	      ex_move1.o ex_move2.o ex_move3.o ex_move4.o ex_move5.o          \
	      ex_opts.o ex_recycle.o ex_redraw.o ex_regexp.o ex_regsub.o      \
	      ex_system.o ex_tio.o ex_tmp.o ex_unix.o ex_vars.o ex_vcmd.o     \
	      ex_vi.o  -ltermcap

expreserve:	expreserve.o ex_ctype.o
	$(CC) -o expreserve expreserve.o ex_ctype.o

exrecover:	exrecover.o
	$(CC) -o exrecover exrecover.o

expreserve.o:	expreserve.c ex_ctype.h
	$(CC) -c -o expreserve.o expreserve.c

exrecover.o:	exrecover.c
	$(CC) -c -o exrecover.o exrecover.c

ex_blk.o:	ex_blk.c config.h ex_common.h ex_curses.h
	$(CC) -c -o ex_blk.o ex_blk.c

ex_cmd1.o:	ex_cmd1.c config.h ex_common.h ex_curses.h ex_regexp.h \
		ex_ctype.h
	$(CC) -c -o ex_cmd1.o ex_cmd1.c

ex_cmd2.o:	ex_cmd2.c config.h ex_common.h ex_curses.h ex_regexp.h \
		ex_ctype.h
	$(CC) -c -o ex_cmd2.o ex_cmd2.c

ex_ctype.o:	ex_ctype.c config.h ex_ctype.h
	$(CC) -c -o ex_ctype.o ex_ctype.c

ex_curses.o:	ex_curses.c config.h ex_common.h ex_curses.h
	$(CC) -c -o ex_curses.o ex_curses.c

ex_cut.o:	ex_cut.c config.h ex_common.h ex_curses.h
	$(CC) -c -o ex_cut.o ex_cut.c

ex_ex.o:	ex_ex.c config.h ex_common.h ex_curses.h ex_ctype.h
	$(CC) -c -o ex_ex.o ex_ex.c

ex_input.o:	ex_input.c config.h ex_common.h ex_curses.h ex_ctype.h
	$(CC) -c -o ex_input.o ex_input.c

ex_main.o:	ex_main.c config.h ex_common.h ex_curses.h
	$(CC) -c -o ex_main.o ex_main.c

ex_misc.o:	ex_misc.c config.h ex_common.h ex_curses.h
	$(CC) -c -o ex_misc.o ex_misc.c

ex_modify.o:	ex_modify.c config.h ex_common.h ex_curses.h
	$(CC) -c -o ex_modify.o ex_modify.c

ex_move1.o:	ex_move1.c config.h ex_common.h ex_curses.h ex_ctype.h
	$(CC) -c -o ex_move1.o ex_move1.c

ex_move2.o:	ex_move2.c config.h ex_common.h ex_curses.h ex_regexp.h
	$(CC) -c -o ex_move2.o ex_move2.c

ex_move3.o:	ex_move3.c config.h ex_common.h ex_curses.h
	$(CC) -c -o ex_move3.o ex_move3.c

ex_move4.o:	ex_move4.c config.h ex_common.h ex_curses.h
	$(CC) -c -o ex_move4.o ex_move4.c

ex_move5.o:	ex_move5.c config.h ex_common.h ex_curses.h ex_ctype.h
	$(CC) -c -o ex_move5.o ex_move5.c

ex_opts.o:	ex_opts.c config.h ex_common.h ex_curses.h ex_ctype.h
	$(CC) -c -o ex_opts.o ex_opts.c

ex_recycle.o:	ex_recycle.c config.h ex_common.h ex_curses.h
	$(CC) -c -o ex_recycle.o ex_recycle.c

ex_redraw.o:	ex_redraw.c config.h ex_common.h ex_curses.h
	$(CC) -c -o ex_redraw.o ex_redraw.c

ex_regexp.o:	ex_regexp.c config.h ex_common.h ex_curses.h ex_regexp.h \
		ex_ctype.h
	$(CC) -c -o ex_regexp.o ex_regexp.c

ex_regsub.o:	ex_regsub.c config.h ex_common.h ex_curses.h ex_regexp.h \
		ex_ctype.h
	$(CC) -c -o ex_regsub.o ex_regsub.c

ex_system.o:	ex_system.c config.h ex_common.h ex_curses.h
	$(CC) -c -o ex_system.o ex_system.c

ex_tio.o:	ex_tio.c config.h ex_common.h ex_curses.h ex_ctype.h
	$(CC) -c -o ex_tio.o ex_tio.c

ex_tmp.o:	ex_tmp.c config.h ex_common.h ex_curses.h
	$(CC) -c -o ex_tmp.o ex_tmp.c

ex_unix.o:	ex_unix.c config.h ex_common.h ex_curses.h
	$(CC) -c -o ex_unix.o ex_unix.c

ex_vars.o:	ex_vars.c config.h ex_common.h ex_curses.h
	$(CC) -c -o ex_vars.o ex_vars.c

ex_vcmd.o:	ex_vcmd.c config.h ex_common.h ex_curses.h ex_ctype.h
	$(CC) -c -o ex_vcmd.o ex_vcmd.c

ex_vi.o:	ex_vi.c config.h ex_common.h ex_curses.h ex_ctype.h
	$(CC) -c -o ex_vi.o ex_vi.c

clean:
	rm -f ex expreserve exrecover ex_blk.o ex_cmd1.o ex_cmd2.o            \
	      ex_ctype.o ex_curses.o ex_cut.o ex_ex.o ex_input.o ex_main.o    \
	      ex_misc.o ex_modify.o ex_move1.o ex_move2.o ex_move3.o          \
	      ex_move4.o ex_move5.o ex_opts.o ex_recycle.o ex_redraw.o        \
	      ex_regexp.o ex_regsub.o ex_system.o ex_tio.o ex_tmp.o ex_unix.o \
	      ex_vars.o ex_vcmd.o ex_vi.o expreserve.o exrecover.o

install:
	install -d -m 755 ex /usr/local/bin
	install -d -m 755 expreserve /usr/local/bin
	install -d -m 755 exrecover /usr/local/bin
	ln -s ex /usr/local/bin/edit
	ln -s ex /usr/local/bin/vi
	ln -s ex /usr/local/bin/view
	ln -s ex /usr/local/bin/input
