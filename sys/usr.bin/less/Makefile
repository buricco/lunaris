PROG=	less
SRCS=	brac.c ch.c charset.c cmdbuf.c command.c decode.c edit.c \
	filename.c forwback.c help.c ifile.c input.c jump.c line.c \
	linenum.c lsystem.c main.c mark.c optfunc.c option.c opttbl.c \
	os.c output.c position.c prompt.c screen.c search.c signal.c \
	tags.c ttyin.c version.c
LDADD+=	-ltermcap

${PROG}:	${SRCS}
	$(CC) -o ${PROG} ${SRCS} ${LDADD}

install:
	install -d -m 755 less ${DESTDIR}/usr/bin
