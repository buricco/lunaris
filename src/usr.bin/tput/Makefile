all:	clear tput

clear:	clear.sh
	cp clear.sh clear
	chmod +x clear

tput:	tput.c
	$(CC) -o tput tput.c -ltermcap

clean:
	rm -rf clear tput

install:
	install -d -m 755 tput ${DESTDIR}/usr/bin
