SH = /bin/sh
HOSTCC = /usr/bin/cc

sh:	alias.o arith_yacc.o arith_yylex.o cd.o error.o eval.o exec.o expand.o histedit.o input.o jobs.o mail.o main.o memalloc.o miscbltin.o mystring.o options.o parser.o redir.o show.o trap.o output.o bltin/printf.o system.o bltin/test.o bltin/times.o var.o builtins.o init.o nodes.o signames.o syntax.o
	$(CC) -Wall -g -O2 -o sh alias.o arith_yacc.o arith_yylex.o cd.o error.o eval.o exec.o expand.o histedit.o input.o jobs.o mail.o main.o memalloc.o miscbltin.o mystring.o options.o parser.o redir.o show.o trap.o output.o bltin/printf.o system.o bltin/test.o bltin/times.o var.o builtins.o init.o nodes.o signames.o syntax.o

builtins.c builtins.h:	builtins.def mkbuiltins
	$(SH) mkbuiltins builtins.def

builtins.def:	builtins.def.in
	$(HOSTCC) -E -x c -o builtins.def builtins.def.in

mknodes:	mknodes.c
	$(HOSTCC) -o mknodes mknodes.c

nodes.c nodes.h:	nodetypes nodes.c.pat mknodes
	./mknodes nodetypes nodes.c.pat

token.h token_vars.h:
	$(SH) mktokens

mksyntax:	mksyntax.c parser.h token.h
	$(HOSTCC) -I. -I.. -DBSD=1 -DSHELL -g -O2 -Wall -o mksyntax mksyntax.c

syntax.c syntax.h:	mksyntax
	./mksyntax

alias.o:	alias.c shell.h input.h output.h error.h memalloc.h mystring.h alias.h options.h config.h
	$(CC) -c -o alias.o alias.c

arith_yacc.o:	arith_yacc.c arith_yacc.h expand.h shell.h error.h output.h var.h config.h
	$(CC) -c -o arith_yacc.o arith_yacc.c

arith_yylex.o:	arith_yylex.c arith_yacc.h expand.h error.h shell.h memalloc.h syntax.h system.h config.h
	$(CC) -c -o arith_yylex.o arith_yylex.c

cd.o:	cd.c shell.h var.h nodes.h jobs.h options.h output.h memalloc.h error.h exec.h redir.h main.h mystring.h show.h cd.h config.h
	$(CC) -c -o cd.o cd.c

error.o:	error.c jobs.h shell.h main.h options.h output.h error.h show.h eval.h parser.h system.h config.h
	$(CC) -c -o error.o error.c

eval.o:	eval.c init.h main.h shell.h nodes.h syntax.h expand.h parser.h jobs.h eval.h builtins.h options.h exec.h redir.h input.h output.h trap.h var.h memalloc.h error.h show.h mystring.h myhistedit.h config.h
	$(CC) -c -o eval.o eval.c

exec.o:	exec.c shell.h main.h nodes.h parser.h redir.h eval.h exec.h builtins.h var.h options.h output.h syntax.h memalloc.h error.h init.h mystring.h show.h jobs.h alias.h system.h config.h
	$(CC) -c -o exec.o exec.c

expand.o:	expand.c shell.h main.h nodes.h eval.h expand.h syntax.h parser.h jobs.h options.h var.h output.h memalloc.h error.h mystring.h show.h system.h config.h
	$(CC) -c -o expand.o expand.c

histedit.o:	histedit.c shell.h parser.h var.h options.h main.h output.h mystring.h error.h myhistedit.h eval.h memalloc.h config.h
	$(CC) -c -o histedit.o histedit.c

input.o:	input.c eval.h shell.h redir.h syntax.h input.h output.h options.h memalloc.h error.h alias.h parser.h main.h myhistedit.h config.h
	$(CC) -c -o input.o input.c

jobs.o:	jobs.c shell.h exec.h eval.h init.h redir.h show.h main.h parser.h nodes.h jobs.h options.h trap.h syntax.h input.h output.h memalloc.h error.h mystring.h system.h config.h
	$(CC) -c -o jobs.o jobs.c

mail.o:	mail.c shell.h nodes.h exec.h var.h output.h memalloc.h error.h mail.h mystring.h config.h
	$(CC) -c -o mail.o mail.c

main.o:	main.c shell.h main.h mail.h options.h output.h parser.h nodes.h expand.h eval.h jobs.h input.h trap.h var.h show.h memalloc.h error.h init.h mystring.h exec.h cd.h config.h
	$(CC) -c -o main.o main.c

memalloc.o:	memalloc.c shell.h output.h memalloc.h error.h machdep.h mystring.h system.h config.h
	$(CC) -c -o memalloc.o memalloc.c

miscbltin.o:	miscbltin.c shell.h options.h var.h output.h memalloc.h error.h miscbltin.h mystring.h main.h expand.h parser.h trap.h config.h
	$(CC) -c -o miscbltin.o miscbltin.c

mystring.o:	mystring.c shell.h syntax.h error.h mystring.h memalloc.h parser.h system.h config.h
	$(CC) -c -o mystring.o mystring.c

options.o:	options.c shell.h options.h nodes.h eval.h jobs.h input.h output.h trap.h var.h memalloc.h error.h mystring.h myhistedit.h show.h config.h
	$(CC) -c -o options.o options.c

parser.o:	parser.c shell.h parser.h nodes.h expand.h exec.h syntax.h options.h input.h output.h var.h error.h memalloc.h mystring.h alias.h show.h builtins.h system.h myhistedit.h token_vars.h config.h
	$(CC) -c -o parser.o parser.c

redir.o:	redir.c main.h shell.h nodes.h jobs.h options.h expand.h redir.h output.h memalloc.h error.h config.h
	$(CC) -c -o redir.o redir.c

show.o:	show.c shell.h parser.h nodes.h mystring.h show.h options.h config.h
	$(CC) -c -o show.o show.c

trap.o:	trap.c shell.h main.h nodes.h eval.h init.h jobs.h show.h options.h syntax.h output.h memalloc.h error.h trap.h mystring.h config.h
	$(CC) -c -o trap.o trap.c

output.o:	output.c shell.h syntax.h output.h memalloc.h error.h main.h system.h config.h
	$(CC) -c -o output.o output.c

bltin/printf.o:	bltin/printf.c bltin/bltin.h system.h shell.h mystring.h options.h memalloc.h output.h error.h config.h
	$(CC) -c -o bltin/printf.o bltin/printf.c

system.o:	system.c error.h output.h system.h config.h
	$(CC) -c -o system.o system.c

bltin/test.o:	bltin/test.c bltin/bltin.h config.h
	$(CC) -c -o bltin/test.o bltin/test.c

bltin/times.o:	bltin/times.c bltin/bltin.h system.h config.h
	$(CC) -c -o bltin/times.o bltin/times.c

var.o:	var.c shell.h output.h expand.h nodes.h exec.h syntax.h options.h mail.h var.h memalloc.h error.h mystring.h parser.h show.h myhistedit.h system.h config.h
	$(CC) -c -o var.o var.c

builtins.o:	builtins.c shell.h builtins.h config.h
	$(CC) -c -o builtins.o builtins.c

mkinit:	mkinit.c
	$(CC) -I. -I.. -DBSD=1 -DSHELL -g -O2 -Wall -o mkinit mkinit.c

init.c: mkinit alias.c arith_yacc.c arith_yylex.c cd.c error.c eval.c exec.c expand.c histedit.c input.c jobs.c mail.c main.c memalloc.c miscbltin.c mystring.c options.c parser.c redir.c show.c trap.c output.c bltin/printf.c system.c bltin/test.c bltin/times.c var.c
	./mkinit alias.c arith_yacc.c arith_yylex.c cd.c error.c eval.c exec.c expand.c histedit.c input.c jobs.c mail.c main.c memalloc.c miscbltin.c mystring.c options.c parser.c redir.c show.c trap.c output.c bltin/printf.c system.c bltin/test.c bltin/times.c var.c

init.o:	init.c shell.h mystring.h init.h eval.h expand.h input.h error.h parser.h redir.h memalloc.h trap.h output.h cd.h var.h config.h
	$(CC) -c -o init.o init.c

nodes.o:	nodes.c shell.h nodes.h memalloc.h machdep.h mystring.h system.h config.h
	$(CC) -c -o nodes.o nodes.c

mksignames: mksignames.c
	$(CC) -I. -I.. -DBSD=1 -DSHELL -g -O2 -Wall -o mksignames mksignames.c

signames.o:	signames.c
	$(CC) -c -o signames.o signames.c

syntax.o:	syntax.c shell.h syntax.h config.h
	$(CC) -c -o syntax.o syntax.c

clean:
	rm -f sh alias.o arith_yacc.o arith_yylex.o cd.o error.o eval.o exec.o expand.o histedit.o input.o jobs.o mail.o main.o memalloc.o miscbltin.o mystring.o options.o parser.o redir.o show.o trap.o output.o bltin/printf.o system.o bltin/test.o bltin/times.o var.o builtins.o init.o nodes.o signames.o syntax.o builtins.c builtins.h builtins.def nodes.c nodes.h mknodes token.h token_vars.h mksyntax mkinit init.c syntax.c syntax.h
