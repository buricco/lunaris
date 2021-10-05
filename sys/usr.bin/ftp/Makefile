ftp:	cmds.o cmdtab.o domacro.o ftp.o glob.o main.o ruserpass.o
	$(CC) -o ftp cmds.o cmdtab.o domacro.o ftp.o glob.o main.o ruserpass.o

cmds.o:	cmds.c ftp_var.h pathnames.h cmds.h main.h
	$(CC) -c -o cmds.o cmds.c

cmdtab.o:	cmdtab.c ftp_var.h main.h
	$(CC) -c -o cmdtab.o cmdtab.c

domacro.o:	domacro.c ftp_var.h main.h
	$(CC) -c -o domacro.o domacro.c

ftp.o:	ftp.c ftp_var.h cmds.h main.h
	$(CC) -c -o ftp.o ftp.c

glob.o:	glob.c ftp_var.h glob.h
	$(CC) -c -o glob.o glob.c

main.o:	main.c ftp_var.h main.h
	$(CC) -c -o main.o main.c

ruserpass.o:	ruserpass.c ftp_var.h main.h
	$(CC) -c -o ruserpass.o ruserpass.c

clean:
	rm -f ftp cmds.o cmdtab.o domacro.o ftp.o glob.o main.o ruserpass.o
