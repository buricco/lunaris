/*
 * Copyright (c) 1985, 1989 Regents of the University of California.
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/* 
 * From: @(#)ftp.c	5.38 (Berkeley) 4/22/91
 */
char ftp_rcsid[] = 
  "$Id: ftp.c,v 1.25 1999/12/13 20:33:20 dholland Exp $";

#include <sys/param.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/file.h>

#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/ftp.h>
#include <arpa/inet.h>
#include <arpa/telnet.h>

#include <ctype.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <pwd.h>
#include <stdarg.h>
#include <stdint.h>

#include "ftp_var.h"
#include "cmds.h"
#include "main.h"

int data = -1;
off_t restart_point = 0;

/* Increment for hash markings. Can be overridden by a command. */
size_t hashstep = 1024;

static char ipstring[INET6_ADDRSTRLEN]; /* Scribble area for resolver. */

static struct sockaddr_storage hisctladdr;
static struct sockaddr_storage data_addr;
static struct sockaddr_storage myctladdr;
static int ptflag = 0;
static int ptabflg = 0;

void lostpeer(int);
extern int connected;

static char *gunique(char *);
static void proxtrans(const char *cmd, char *local, char *remote);
static int initconn(void);
static void ptransfer(const char *direction, off_t bytes, 
		      const struct timeval *t0, 
		      const struct timeval *t1);
static void tvsub(struct timeval *tdiff, 
		  const struct timeval *t1, 
		  const struct timeval *t0);
static void abort_remote(FILE *din);

FILE *cin, *cout;
static FILE *dataconn(const char *);
static void printbytes(off_t);

#if ! defined(FTP_CONNECT_TIMEOUT) || FTP_CONNECT_TIMEOUT < 1
# define FTP_CONNECT_TIMEOUT 10
#endif

static void
trivial_alarm(int sig)
{
	/* Only used to generate an EINTR error. */
	return;
}

char *
hookup(char *host, int port)
{
	struct addrinfo hints, *ai = NULL, *aiptr = NULL;
	struct sigaction sigact, oldsigact;
	int status;
	volatile int s = -1;
	int tos, af_in_use;
	socklen_t len;
	static char hostnamebuf[256];
	sigjmp_buf jmploc;
	sigjmp_buf *volatile oldtoplevel;
	int dupfd;
	struct sockaddr_in *hisctl_sa4 = (struct sockaddr_in *) &hisctladdr;
	struct sockaddr_in6 *hisctl_sa6 = (struct sockaddr_in6 *) &hisctladdr;

	memset(&hisctladdr, 0, sizeof(hisctladdr));

	sigact.sa_handler = trivial_alarm;
	sigemptyset(&sigact.sa_mask);
	sigact.sa_flags = 0;

	memset(&hints, 0, sizeof(hints));
	hints.ai_flags = AI_ADDRCONFIG | AI_CANONNAME;
	hints.ai_family = usefamily;
	hints.ai_socktype = SOCK_STREAM;

	if ( (status = getaddrinfo(host, NULL, &hints, &ai)) ) {
		fprintf(stderr, "ftp: %s: %s\n", host,
				gai_strerror(status));
		code = -1;
		return((char *) 0);
	}

	aiptr = ai;
	memcpy(&hisctladdr, aiptr->ai_addr, aiptr->ai_addrlen);
	(void) strncpy(hostnamebuf, aiptr->ai_canonname,
			sizeof(hostnamebuf));
	hostnamebuf[sizeof(hostnamebuf)-1] = 0;
	hostname = hostnamebuf;

	oldtoplevel = toplevel;
	if (sigsetjmp(jmploc, 0)) {
		if (s >= 0)
			close(s);
		toplevel = oldtoplevel;
		siglongjmp(*toplevel, 1);
	}
	toplevel = &jmploc;

	INTOFF;
	s = socket(hisctladdr.ss_family, SOCK_STREAM, 0);
	af_in_use = hisctladdr.ss_family;
	INTON;
	if (s < 0) {
		perror("ftp: socket");
		freeaddrinfo(ai);
		code = -1;
		goto out;
	}
	switch (hisctladdr.ss_family) {
		case AF_INET:
			hisctl_sa4->sin_port = port;
			break;
		case AF_INET6:
			hisctl_sa6->sin6_port = port;
	}

	sigaction(SIGALRM, &sigact, &oldsigact);
	alarm(FTP_CONNECT_TIMEOUT);

	while (connect(s, (struct sockaddr *)&hisctladdr,
				(hisctladdr.ss_family == AF_INET)
				? sizeof(struct sockaddr_in)
				: sizeof(struct sockaddr_in6))
			< 0)
	{
		alarm(0);
		sigaction(SIGALRM, &oldsigact, NULL);
		if (errno == EINTR)
			errno = ETIMEDOUT;

		if (aiptr && aiptr->ai_next)
		{
			int oerrno = errno;
			struct in_addr *ctladdr4 = &hisctl_sa4->sin_addr;
			struct in6_addr *ctladdr6 = &hisctl_sa6->sin6_addr;

			switch (aiptr->ai_family) {
			    case AF_INET:
				fprintf(stderr, "ftp: connect to address %s: ",
					inet_ntop(aiptr->ai_family,
							ctladdr4,
							ipstring,
							sizeof(ipstring)));
				break;
			    case AF_INET6:
				fprintf(stderr, "ftp: connect to address %s: ",
					inet_ntop(aiptr->ai_family,
							ctladdr6,
							ipstring,
							sizeof(ipstring)));
			}
			errno = oerrno;
			perror((char *) 0);

			aiptr = aiptr->ai_next;
			memcpy(&hisctladdr, aiptr->ai_addr,
					aiptr->ai_addrlen);
			switch (hisctladdr.ss_family) {
				case AF_INET:
					hisctl_sa4->sin_port = port;
					break;
				case AF_INET6:
					hisctl_sa6->sin6_port = port;
			}

			switch (aiptr->ai_family) {
			    case AF_INET:
				fprintf(stdout, "Trying %s...\n",
					inet_ntop(aiptr->ai_family,
							ctladdr4,
							ipstring,
							sizeof(ipstring)));
				break;
			    case AF_INET6:
				fprintf(stdout, "Trying %s...\n",
					inet_ntop(aiptr->ai_family,
							ctladdr6,
							ipstring,
							sizeof(ipstring)));
			}
			INTOFF;
			(void) close(s);
			s = socket(aiptr->ai_family, SOCK_STREAM, 0);
			af_in_use = aiptr->ai_family;
			INTON;
			if (s < 0) {
				perror("ftp: socket");
				freeaddrinfo(ai);
				code = -1;
				goto out;
			}
			/* Try next server candidate. */
			continue;
		}
		/* No answer to any call. */
		perror("ftp: connect");
		freeaddrinfo(ai);
		code = -1;
		goto bad;
	}
	alarm(0);
	sigaction(SIGALRM, &oldsigact, NULL);

	len = sizeof (myctladdr);
	if (getsockname(s, (struct sockaddr *)&myctladdr, &len) < 0) {
		perror("ftp: getsockname");
		if (ai)
			freeaddrinfo(ai);
		code = -1;
		goto bad;
	}
	if (ai)
		freeaddrinfo(ai);
#ifdef IP_TOS
	tos = IPTOS_LOWDELAY;
	if ( (af_in_use == AF_INET) &&
		(setsockopt(s, IPPROTO_IP, IP_TOS, (char *)&tos,
	  		  sizeof(tos)) < 0) )
		perror("ftp: setsockopt TOS (ignored)");
#endif /* IP_TOS */
	INTOFF;
	if (cin)
		fclose(cin);
	if (cout)
		fclose(cout);
	cin = fdopen(s, "r");
	if (cin == NULL)
		close(s);
	dupfd = dup(s);
	cout = fdopen(dup(s), "w");
	if (cout == NULL && dupfd >= 0)
		close(dupfd);
	s = -1;
	toplevel = oldtoplevel;
	INTON;
	if (cin == NULL || cout == NULL) {
		fprintf(stderr, "ftp: fdopen failed.\n");
		if (cin) {
			INTOFF;
			(void) fclose(cin);
			cin = NULL;
			INTON;
		}
		if (cout) {
			INTOFF;
			(void) fclose(cout);
			cout = NULL;
			INTON;
		}
		code = -1;
		goto out;
	}
	if (verbose)
		printf("Connected to %s.\n", hostname);
	if (getreply(0) > 2) { 	/* read startup message from server */
		INTOFF;
		fclose(cin);
		fclose(cout);
		cin = NULL;
		cout = NULL;
		INTON;
		code = -1;
		goto out;
	}
#ifdef SO_OOBINLINE
	{
	int on = 1;

	if (setsockopt(s, SOL_SOCKET, SO_OOBINLINE, (char *)&on, sizeof(on))
		< 0 && debug) {
			perror("ftp: setsockopt");
		}
	}
#endif /* SO_OOBINLINE */

	return (hostname);
bad:
	INTOFF;
	(void) close(s);
	s = -1;
	INTON;
out:
	toplevel = oldtoplevel;
	return ((char *)0);
}

int
dologin(const char *host)
{
	char tmp[80];
	char *luser, *pass, *zacct;
	int n, aflag = 0;

	luser = pass = zacct = 0;
	INTOFF;
	if (xruserpass(host, &luser, &pass, &zacct) < 0) {
		INTON;
		code = -1;
		return(0);
	}
	INTON;
	while (luser == NULL) {
		char *myname = getlogin();

		if (myname == NULL) {
			struct passwd *pp = getpwuid(getuid());

			if (pp != NULL)
				myname = pp->pw_name;
		}
		if (myname)
			printf("Name (%s:%s): ", host, myname);
		else
			printf("Name (%s): ", host);
		if (fgets(tmp, sizeof(tmp), stdin)==NULL) {
			fprintf(stderr, "\nLogin failed.\n");
			return 0;
		}
		n = strlen(tmp);
		if (tmp[n - 1] == '\n')
			tmp[n - 1] = 0;
		if (*tmp == '\0')
			luser = myname;
		else
			luser = tmp;
	}
	n = command("USER %s", luser);
	if (n == CONTINUE) {
		if (pass == NULL) {
			/* fflush(stdout); */
			pass = getpass("Password:");
		}
		n = command("PASS %s", pass);
	}
	if (n == CONTINUE) {
		aflag++;
		/* fflush(stdout); */
		zacct = getpass("Account:");
		n = command("ACCT %s", zacct);
	}
	if (n != COMPLETE) {
		fprintf(stderr, "Login failed.\n");
		return (0);
	}
	if (!aflag && zacct != NULL)
		(void) command("ACCT %s", zacct);
	if (proxy)
		return(1);
	for (n = 0; n < macnum; ++n) {
		if (!strcmp("init", macros[n].mac_name)) {
			int margc;
			char **margv;
			char *oldline = line;
			INTOFF;
			line = obstack_copy(&lineobstack, "$init", 6);
			INTON;
			margv = makeargv(&margc, NULL);
			domacro(margc, margv);
			INTOFF;
			obstack_free(&lineobstack, line);
			INTON;
			line = oldline;
			break;
		}
	}
	return (1);
}

int
command(const char *fmt, ...)
{
	va_list ap;
	int r;

	if (debug) {
		printf("---> ");
		va_start(ap, fmt);
		if (strncmp("PASS ", fmt, 5) == 0)
			printf("PASS XXXX");
		else 
			vprintf(fmt, ap);
		va_end(ap);
		printf("\n");
		(void) fflush(stdout);
	}
	if (cout == NULL) {
		perror ("No control connection for command");
		code = -1;
		return (0);
	}
	INTOFF;
	intrnewline++;
	va_start(ap, fmt);
	vfprintf(cout, fmt, ap);
	va_end(ap);
	fputs("\r\n", cout);
	if (fflush(cout) == EOF)
		goto outerr;
	cpend = 1;
	r = getreply(!strcmp(fmt, "QUIT"));
	intrnewline--;
	INTON;
	return(r);
outerr:
	lostpeer(0);
	INTON;
	if (verbose) {
		printf("421 Service not available, remote server has closed connection\n");
		fflush(stdout);
	}
	code = 421;
	return 4;
}

char reply_string[BUFSIZ];		/* last line of previous reply */

#include <ctype.h>

int
getreply(int expecteof)
{
	register int c, n;
	register int dig;
	register char *cp;
	int originalcode = 0, continuation = 0;
	int pflag = 0;
	size_t px = 0;
	size_t psize = sizeof(pasv);

	if (!cin || !cout) {
		cpend = 0;
		return 4;
	}
	INTOFF;
	intrnewline++;
	for (;;) {
		dig = n = code = 0;
		cp = reply_string;
		while ((c = getc(cin)) != '\n') {
			if (c == IAC) {     /* handle telnet commands */
				switch (c = getc(cin)) {
				case WILL:
				case WONT:
					if ((c = getc(cin)) == EOF)
						goto goteof;
					fprintf(cout, "%c%c%c", IAC, DONT, c);
					if (fflush(cout) == EOF)
						goto goteof;
					break;
				case DO:
				case DONT:
					if ((c = getc(cin)) == EOF)
						goto goteof;
					fprintf(cout, "%c%c%c", IAC, WONT, c);
					if (fflush(cout) == EOF)
						goto goteof;
					break;
				default:
					break;
				}
				continue;
			}
			dig++;
			if (c == EOF) {
				if (expecteof) {
					intrnewline--;
					INTON;
					code = 221;
					return (0);
				}
goteof:
				lostpeer(0);
				intrnewline--;
				INTON;
				if (verbose) {
					printf("421 Service not available, remote server has closed connection\n");
					(void) fflush(stdout);
				}
				code = 421;
				return(4);
			}
			if (c != '\r' && (verbose > 0 ||
			    (verbose > -1 && n == '5' && dig > 4))) {
				if (proxflag &&
				   (dig == 1 || (dig == 5 && verbose == 0)))
					printf("%s:",hostname);
				(void) putchar(c);
			}
			if (dig < 4 && isdigit(c))
				code = code * 10 + (c - '0');
			if (!pflag && (code == 227 || code == 229))
				pflag = 1;
			if (dig > 4 && pflag == 1 && isdigit(c))
				pflag = 2;
			if (pflag == 2) {
				if (c != '\r' && c != ')') {
					if (px < psize-1) pasv[px++] = c;
				}
				else {
					pasv[px] = '\0';
					pflag = 3;
				}
			}
			if (dig == 4 && c == '-') {
				if (continuation)
					code = 0;
				continuation++;
			}
			if (n == 0)
				n = c;
			if (cp < &reply_string[sizeof(reply_string) - 1])
				*cp++ = c;
		}
		if (verbose > 0 || (verbose > -1 && n == '5')) {
			(void) putchar(c);
			(void) fflush (stdout);
		}
		if (continuation && code != originalcode) {
			if (originalcode == 0)
				originalcode = code;
			continue;
		}
		*cp = '\0';
		if (n != '1')
			cpend = 0;
		intrnewline--;
		INTON;
		if (code == 421 || originalcode == 421) {
			INTOFF;
			lostpeer(0);
			INTON;
		}
		return (n - '0');
	}
}

static int
empty(fd_set *mask, int hifd, int sec)
{
	struct timeval t;

	t.tv_sec = (long) sec;
	t.tv_usec = 0;
	return(select(hifd+1, mask, (fd_set *) 0, (fd_set *) 0, &t));
}

static void
abortsend(int ignore)
{
	(void)ignore;

	mflag = 0;
	printf("\nsend aborted\nwaiting for remote to finish abort\n");
	(void) fflush(stdout);
}

#define HASHBYTES 1024

void
sendrequest(const char *cmd, char *local, char *remote, int printnames)
{
	struct stat st;
	struct timeval start, stop;
	register int c, d;
	FILE *volatile fin = 0, *volatile dout = 0;
	int (*volatile closefunc)(FILE *);
	volatile off_t bytes = 0, hashbytes = hashstep;
	char buf[BUFSIZ], *bufp;
	const char *volatile lmode;
	sigjmp_buf jmploc;
	sigjmp_buf *volatile oldtoplevel;

	if (verbose && printnames) {
		if (local && *local != '-')
			printf("local: %s ", local);
		if (remote)
			printf("remote: %s\n", remote);
	}
	if (proxy) {
		proxtrans(cmd, local, remote);
		return;
	}
	if (curtype != type)
		changetype(type, 0);
	closefunc = NULL;
	lmode = "w";
	oldtoplevel = toplevel;
	if (sigsetjmp(jmploc, 1)) {
		abortsend(SIGINT);
		while (cpend) {
			(void) getreply(0);
		}
		if (data >= 0) {
			INTOFF;
			(void) close(data);
			data = -1;
			INTON;
		}
		if (fin != NULL && closefunc != NULL)
			(*closefunc)(fin);
		toplevel = oldtoplevel;
		code = -1;
		return;
	}
	toplevel = &jmploc;
	if (strcmp(local, "-") == 0)
		fin = stdin;
	else if (*local == '|') {
		closefunc = pclose;
		INTOFF;
		fin = popen(local + 1, "r");
		INTON;
		if (fin == NULL) {
			perror(local + 1);
			toplevel = oldtoplevel;
			code = -1;
			return;
		}
	} else {
		closefunc = fclose;
		INTOFF;
		fin = fopen(local, "r");
		INTON;
		if (fin == NULL) {
			fprintf(stderr, "local: %s: %s\n", local,
				strerror(errno));
			toplevel = oldtoplevel;
			code = -1;
			return;
		}
		if (fstat(fileno(fin), &st) < 0 ||
		    (st.st_mode&S_IFMT) != S_IFREG) {
			fprintf(stdout, "%s: not a plain file.\n", local);
			INTOFF;
			fclose(fin);
			fin = NULL;
			INTON;
			toplevel = oldtoplevel;
			code = -1;
			return;
		}
	}
	if (initconn()) {
		code = -1;
		if (closefunc != NULL) {
			INTOFF;
			(*closefunc)(fin);
			fin = NULL;
			INTON;
		}
		toplevel = oldtoplevel;
		return;
	}
	if (sigsetjmp(jmploc, 1)) {
		abortsend(SIGINT);
		goto abort;
	}

	if (restart_point &&
	    (strcmp(cmd, "STOR") == 0 || strcmp(cmd, "APPE") == 0)) {
		if (fseeko(fin, restart_point, SEEK_SET) < 0) {
			fprintf(stderr, "local: %s: %s\n", local,
				strerror(errno));
			restart_point = 0;
			if (closefunc != NULL) {
				INTOFF;
				(*closefunc)(fin);
				fin = NULL;
				INTON;
			}
			toplevel = oldtoplevel;
			return;
		}
		if (command("REST %jd", (intmax_t) restart_point)
			!= CONTINUE) {
			restart_point = 0;
			if (closefunc != NULL) {
				INTOFF;
				(*closefunc)(fin);
				fin = NULL;
				INTON;
			}
			toplevel = oldtoplevel;
			return;
		}
		restart_point = 0;
		lmode = "r+w";
	}
	if (remote) {
		if (command("%s %s", cmd, remote) != PRELIM) {
			if (closefunc != NULL) {
				INTOFF;
				(*closefunc)(fin);
				fin = NULL;
				INTON;
			}
			toplevel = oldtoplevel;
			return;
		}
	} else
		if (command("%s", cmd) != PRELIM) {
			if (closefunc != NULL) {
				INTOFF;
				(*closefunc)(fin);
				fin = NULL;
				INTON;
			}
			toplevel = oldtoplevel;
			return;
		}
	INTOFF;
	dout = dataconn(lmode);
	INTON;
	if (dout == NULL)
		goto abort;
	(void) gettimeofday(&start, (struct timezone *)0);
	switch (curtype) {

	case TYPE_I:
	case TYPE_L:
		errno = d = 0;
		while ((c = read(fileno(fin), buf, sizeof (buf))) > 0) {
			bytes += c;
			for (bufp = buf; c > 0; c -= d, bufp += d)
				if ((d = write(fileno(dout), bufp, c)) <= 0) {
					perror("netout");
					break;
				}
			if (hash) {
				while (bytes >= hashbytes) {
					(void) putchar('#');
					hashbytes += hashstep;
				}
				(void) fflush(stdout);
			}
			if (tick && (bytes >= hashbytes)) {
				printbytes(bytes);
				while (bytes >= hashbytes)
					hashbytes += 10 * hashstep;
			}
			if (d <= 0)
				break;
		}
		if (hash && (bytes > 0)) {
			if (bytes < hashstep)
				(void) putchar('#');
			(void) putchar('\n');
			(void) fflush(stdout);
		}
		if (tick) {
			printbytes(bytes);
			putchar('\n');
		}
		if (c < 0)
			fprintf(stderr, "local: %s: %s\n", local,
				strerror(errno));
		if (d <= 0)
			bytes = -1;
		break;

	case TYPE_A:
		while ((c = getc(fin)) != EOF) {
			if (c == '\n') {
				while (hash && (bytes >= hashbytes)) {
					(void) putchar('#');
					(void) fflush(stdout);
					hashbytes += hashstep;
				}
				if (tick && (bytes >= hashbytes)) {
					printbytes(bytes);
					while (bytes >= hashbytes)
						hashbytes += 10 * hashstep;
				}
				if (putc('\r', dout) == EOF) {
					perror("netout");
					break;
				}
				bytes++;
			}
			if (putc(c, dout) == EOF) {
				perror("netout");
				break;
			}
			bytes++;
	/*		if (c == '\r') {			  	*/
	/*		(void)	putc('\0', dout);  (* this violates rfc */
	/*			bytes++;				*/
	/*		}                          			*/     
		}
		if (hash) {
			if (bytes < hashbytes)
				(void) putchar('#');
			(void) putchar('\n');
			(void) fflush(stdout);
		}
		if (tick) {
			printbytes(bytes);
			putchar('\n');
		}
		if (ferror(fin))
			fprintf(stderr, "local: %s: %s\n", local,
				strerror(errno));
		if (ferror(dout))
			bytes = -1;
		break;
	}
	(void) gettimeofday(&stop, (struct timezone *)0);
	if (closefunc != NULL) {
		INTOFF;
		(*closefunc)(fin);
		fin = NULL;
		INTON;
	}
	INTOFF;
	(void) fclose(dout);
	dout = NULL;
	/* closes data as well, so discard it */
	data = -1;
	INTON;
	(void) getreply(0);
	toplevel = oldtoplevel;
	if (bytes > 0)
		ptransfer("sent", bytes, &start, &stop);
	return;
abort:
	(void) gettimeofday(&stop, (struct timezone *)0);
	if (!cpend) {
		code = -1;
		toplevel = oldtoplevel;
		return;
	}
	if (dout) {
		INTOFF;
		if (data == fileno(dout))
			data = -1;
		(void) fclose(dout);
		dout = NULL;
		INTON;
	}
	if (data >= 0) {
		INTOFF;
		(void) close(data);
		data = -1;
		INTON;
	}
	(void) getreply(0);
	code = -1;
	if (closefunc != NULL && fin != NULL) {
		INTOFF;
		(*closefunc)(fin);
		fin = NULL;
		INTON;
	}
	toplevel = oldtoplevel;
	if (bytes > 0)
		ptransfer("sent", bytes, &start, &stop);
}

static void
abortrecv(int ignore)
{
	(void)ignore;

	mflag = 0;
	printf("\nreceive aborted\nwaiting for remote to finish abort\n");
	(void) fflush(stdout);
}

void
recvrequest(const char *cmd, 
	    char *volatile local, char *remote, 
	    const char *lmode, int printnames)
{
	FILE *volatile fout = 0, *volatile din = 0;
	int (*volatile closefunc)(FILE *);
	volatile int is_retr, tcrflag, bare_lfs = 0;
	int tqcflag = 0;
	unsigned bufsize;
	char *buf;
	volatile off_t bytes = 0, hashbytes = hashstep;
	register int c, d;
	struct timeval start, stop;
	struct stat st;
	sigjmp_buf jmploc;
	sigjmp_buf *volatile oldtoplevel;

	is_retr = strcmp(cmd, "RETR") == 0;
	if (is_retr && verbose && printnames) {
		if (local && *local != '-')
			printf("local: %s ", local);
		if (remote)
			printf("remote: %s\n", remote);
	}
	if (proxy && is_retr) {
		proxtrans(cmd, local, remote);
		return;
	}
	closefunc = NULL;
	tcrflag = !crflag && is_retr;
	oldtoplevel = toplevel;
	if (sigsetjmp(jmploc, 1)) {
		abortrecv(SIGINT);
		while (cpend) {
			(void) getreply(0);
		}
		if (data >= 0) {
			INTOFF;
			(void) close(data);
			data = -1;
			INTON;
		}
		toplevel = oldtoplevel;
		code = -1;
		return;
	}
	toplevel = &jmploc;
	if (strcmp(local, "-") && *local != '|') {
		if (access(local, W_OK) < 0) {
			char *dir = rindex(local, '/');

			if (errno != ENOENT && errno != EACCES) {
				fprintf(stderr, "local: %s: %s\n", local,
					strerror(errno));
				toplevel = oldtoplevel;
				code = -1;
				return;
			}
			if (dir != NULL)
				*dir = 0;
			d = access(dir ? local : ".", W_OK);
			if (dir != NULL)
				*dir = '/';
			if (d < 0) {
				fprintf(stderr, "local: %s: %s\n", local,
					strerror(errno));
				toplevel = oldtoplevel;
				code = -1;
				return;
			}
			if (!runique && errno == EACCES &&
			    chmod(local, 0600) < 0) {
				fprintf(stderr, "local: %s: %s\n", local,
					strerror(errno));
				toplevel = oldtoplevel;
				code = -1;
				return;
			}
			if (runique && errno == EACCES &&
			   (local = gunique(local)) == NULL) {
				toplevel = oldtoplevel;
				code = -1;
				return;
			}
		}
		else if (runique && (strcmp(cmd,"NLST") != 0) &&
			 (local = gunique(local)) == NULL) {
			toplevel = oldtoplevel;
			code = -1;
			return;
		}
	}
	if (!is_retr) {
		if (curtype != TYPE_A)
			changetype(TYPE_A, 0);
	} 
	else if (curtype != type) {
		changetype(type, 0);
	}
	if (initconn()) {
		toplevel = oldtoplevel;
		code = -1;
		return;
	}
	if (sigsetjmp(jmploc, 1)) {
		abortrecv(SIGINT);
		goto abort;
	}
	if (is_retr && restart_point &&
	    command("REST %jd", (intmax_t) restart_point) != CONTINUE) {
		toplevel = oldtoplevel;
		return;
	}
	if (remote) {
		if (command("%s %s", cmd, remote) != PRELIM) {
			toplevel = oldtoplevel;
			return;
		}
	} 
	else {
		if (command("%s", cmd) != PRELIM) {
			toplevel = oldtoplevel;
			return;
		}
	}
	INTOFF;
	din = dataconn("r");
	INTON;
	if (din == NULL)
		goto abort;
	if (strcmp(local, "-") == 0) {
		fout = stdout;
		tqcflag = qcflag;
	}
	else if (*local == '|') {
		closefunc = pclose;
		INTOFF;
		fout = popen(local + 1, "w");
		INTON;
		if (fout == NULL) {
			perror(local+1);
			goto abort;
		}
	} 
	else {
		closefunc = fclose;
		INTOFF;
		fout = fopen(local, lmode);
		INTON;
		if (fout == NULL) {
			fprintf(stderr, "local: %s: %s\n", local,
				strerror(errno));
			goto abort;
		}
	}
	if (fstat(fileno(fout), &st) < 0 || st.st_blksize == 0)
		bufsize = BUFSIZ;
	else
		bufsize = st.st_blksize;
	INTOFF;
	buf = obstack_alloc(&mainobstack, bufsize);
	INTON;
	(void) gettimeofday(&start, (struct timezone *)0);
	switch (curtype) {

	case TYPE_I:
	case TYPE_L:
		if (restart_point &&
		    lseek(fileno(fout), restart_point, SEEK_SET) < 0) {
			fprintf(stderr, "local: %s: %s\n", local,
				strerror(errno));
			if (closefunc != NULL) {
				INTOFF;
				(*closefunc)(fout);
				fout = NULL;
				INTON;
			}
			toplevel = oldtoplevel;
			return;
		}
		errno = d = 0;
		while ((c = read(fileno(din), buf, bufsize)) > 0) {
			if ((d = write(fileno(fout), buf, c)) != c)
				break;
			bytes += c;
			if (hash && is_retr) {
				while (bytes >= hashbytes) {
					(void) putchar('#');
					hashbytes += hashstep;
				}
				(void) fflush(stdout);
			}
			if (tick && (bytes >= hashbytes) && is_retr) {
				printbytes(bytes);
				while (bytes >= hashbytes)
					hashbytes += 10 * hashstep;
			}
		}
		if (hash && bytes > 0) {
			if (bytes < hashstep)
				(void) putchar('#');
			(void) putchar('\n');
			(void) fflush(stdout);
		}
		if (tick && is_retr) {
			printbytes(bytes);
			putchar('\n');
		}
		if (c < 0) {
			perror("netin");
			bytes = -1;
		}
		if (d < c) {
			if (d < 0)
				fprintf(stderr, "local: %s: %s\n", local,
					strerror(errno));
			else
				fprintf(stderr, "%s: short write\n", local);
		}
		break;

	case TYPE_A:
		if (restart_point) {
			register off_t i, n;
			register int ch;

			if (fseeko(fout, 0L, SEEK_SET) < 0)
				goto done;
			n = restart_point;
			for (i = 0; i++ < n;) {
				if ((ch = getc(fout)) == EOF)
					goto done;
				if (ch == '\n')
					i++;
			}
			if (fseeko(fout, 0L, SEEK_CUR) < 0) {
done:
				fprintf(stderr, "local: %s: %s\n", local,
					strerror(errno));
				if (closefunc != NULL) {
					INTOFF;
					(*closefunc)(fout);
					fout = NULL;
					INTON;
				}
				toplevel = oldtoplevel;
				return;
			}
		}
		while ((c = getc(din)) != EOF) {
			if (c == '\n')
				bare_lfs++;
			while (c == '\r') {
				while (hash && (bytes >= hashbytes)
					&& is_retr) {
					(void) putchar('#');
					(void) fflush(stdout);
					hashbytes += hashstep;
				}
				if (tick && (bytes >= hashbytes) && is_retr) {
					printbytes(bytes);
					while (bytes >= hashbytes)
						hashbytes += 10 * hashstep;
				}
				bytes++;
				if ((c = getc(din)) != '\n' || tcrflag) {
					if (ferror(fout))
						goto break2;
					(void) putc('\r', fout);
					if (c == '\0') {
						bytes++;
						goto contin2;
					}
					if (c == EOF)
						goto contin2;
				}
			}
			if (tqcflag && !isprint(c) && !isspace(c))
				c = '?';
			(void) putc(c, fout);
			bytes++;
	contin2:	;
		}
break2:
		if (hash && is_retr) {
			if (bytes < hashbytes)
				(void) putchar('#');
			(void) putchar('\n');
			(void) fflush(stdout);
		}
		if (tick && is_retr) {
			printbytes(bytes);
			putchar('\n');
		}
		if (bare_lfs) {
			printf("WARNING! %d bare linefeeds received in ASCII mode\n", bare_lfs);
			printf("File may not have transferred correctly.\n");
		}
		if (ferror(din)) {
			perror("netin");
			bytes = -1;
		}
		if (ferror(fout))
			fprintf(stderr, "local: %s: %s\n", local,
				strerror(errno));
		break;
	}
	if (closefunc != NULL) {
		INTOFF;
		(*closefunc)(fout);
		fout = NULL;
		INTON;
	}
	INTOFF;
	(void) fclose(din);
	/* closes data as well, so discard it */
	data = -1;
	INTON;
	toplevel = oldtoplevel;
	(void) gettimeofday(&stop, (struct timezone *)0);
	(void) getreply(0);
	if (bytes > 0 && is_retr)
		ptransfer("received", bytes, &start, &stop);
	return;
abort:

/* abort using RFC959 recommended IP,SYNC sequence  */

	(void) gettimeofday(&stop, (struct timezone *)0);
	INTOFF;
	if (!cpend) {
		code = -1;
		INTON;
		toplevel = oldtoplevel;
		return;
	}

	abort_remote(din);
	code = -1;
	if (closefunc != NULL && fout != NULL) {
		(*closefunc)(fout);
		fout = NULL;
	}
	if (din) {
		if (data == fileno(din))
			data = -1;
		(void) fclose(din);
		din = NULL;
	}
	if (data >= 0) {
		(void) close(data);
		data = -1;
	}
	if (bytes > 0)
		ptransfer("received", bytes, &start, &stop);
	toplevel = oldtoplevel;
	INTON;
}

/*
 * Need to start a listen on the data channel before we send the command,
 * otherwise the server's connect may fail.
 */
static int
initconn(void)
{
	register char *p = NULL, *a = NULL;
	int result, tmpno = 0;
	socklen_t len;
	int on = 1;
	int tos;
	u_long a1,a2,a3,a4,p1,p2;
	unsigned short int port;
	struct sockaddr_in *data_addr_sa4 = (struct sockaddr_in *) &data_addr;
	struct sockaddr_in6 *data_addr_sa6 = (struct sockaddr_in6 *) &data_addr;

	if (passivemode) {
		INTOFF;
		if (data >= 0)
			close(data);
		data = socket(hisctladdr.ss_family, SOCK_STREAM, 0);
		INTON;
		if (data < 0) {
			perror("ftp: socket");
			return(1);
		}
		if (options & SO_DEBUG &&
		    setsockopt(data, SOL_SOCKET, SO_DEBUG, (char *)&on,
			       sizeof (on)) < 0)
			perror("ftp: setsockopt (ignored)");
		switch (hisctladdr.ss_family) {
			case AF_INET:
				if (command("PASV") != COMPLETE) {
					printf("Passive mode refused.\n");
					return(1);
				}
				break;
			case AF_INET6:
				if (command("EPSV 2") != COMPLETE) {
					printf("Passive mode refused.\n");
					return(1);
				}
		}

		if (hisctladdr.ss_family == AF_INET) {
			/*
			 * IPv4
			 *
			 * What we've got at this point is a string of
			 * comma separated one-byte unsigned integer
			 * values, separated by commas. The first four
			 * are the an IP address. The fifth is the MSB
			 * of the port number, the sixth is the LSB.
			 * From that we will prepare a sockaddr_in.
			 */

			if (sscanf(pasv,"%ld,%ld,%ld,%ld,%ld,%ld",
				   &a1,&a2,&a3,&a4,&p1,&p2)
			    != 6) 
			{
				printf("Passive mode address scan failure."
					"Shouldn't happen!\n");
				return(1);
			}

			data_addr.ss_family = AF_INET;
			data_addr_sa4->sin_addr.s_addr =
				htonl((a1 << 24) | (a2 << 16) |
						(a3 << 8) | a4);
			data_addr_sa4->sin_port = htons((p1 << 8) | p2);
		} /* Old IPv4 command PASV */
		else {
			/* EPSV for IPv6
			 *
			 * Expected: pasv =~ "%u|"
			 *
			 * This is a shortcut based on the old code
			 * for getreply(), only altered to accept
			 * return code "229" for ESPV, in addition
			 * to "227" which goes with PASV.
			 */
			if (sscanf(pasv, "%hu", &port) != 1) {
				printf("Extended passive mode address "
					"scan failure. Unfortunate!\n");
				return(1);
			}
			data_addr = hisctladdr;
			data_addr.ss_family = AF_INET6;
			data_addr_sa6->sin6_port = htons(port);
		} /* EPSV for IPv6 */
	
		if (connect(data, (struct sockaddr *) &data_addr,
		    sizeof(data_addr))<0) {
			perror("ftp: connect");
			return(1);
		}
#ifdef IP_TOS
		tos = IPTOS_THROUGHPUT;
		if ( (hisctladdr.ss_family == AF_INET) &&
			(setsockopt(data, IPPROTO_IP, IP_TOS, (char *)&tos,
		  		  sizeof(tos)) < 0) )
			perror("ftp: setsockopt TOS (ignored)");
#endif /* IP_TOS */
		return(0);
	}
noport:
	data_addr = myctladdr;
	if (sendport)
		/* let the system pick a port */ 
		switch (data_addr.ss_family) {
			case AF_INET:
				data_addr_sa4->sin_port = 0;
				break;
			case AF_INET6:
				data_addr_sa6->sin6_port = 0;
		}
	INTOFF;
	if (data != -1)
		(void) close(data);
	data = socket(data_addr.ss_family, SOCK_STREAM, 0);
	INTON;
	if (data < 0) {
		perror("ftp: socket");
		if (tmpno)
			sendport = 1;
		return (1);
	}
	if (!sendport)
		if (setsockopt(data, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof (on)) < 0) {
			perror("ftp: setsockopt (reuse address)");
			goto bad;
		}
	if (bind(data, (struct sockaddr *)&data_addr, sizeof (data_addr)) < 0) {
		perror("ftp: bind");
		goto bad;
	}
	if (options & SO_DEBUG &&
	    setsockopt(data, SOL_SOCKET, SO_DEBUG, (char *)&on, sizeof (on)) < 0)
		perror("ftp: setsockopt (ignored)");
	len = sizeof (data_addr);
	if (getsockname(data, (struct sockaddr *)&data_addr, &len) < 0) {
		perror("ftp: getsockname");
		goto bad;
	}
	if (listen(data, 1) < 0)
		perror("ftp: listen");
	if (sendport) {
#define	UC(b)	(((int)b)&0xff)
		switch (data_addr.ss_family) {
			case AF_INET:
				a = (char *)&data_addr_sa4->sin_addr;
				p = (char *)&data_addr_sa4->sin_port;
				result = command("PORT %d,%d,%d,%d,%d,%d",
					    UC(a[0]), UC(a[1]), UC(a[2]),
					    UC(a[3]), UC(p[0]), UC(p[1]));
				break;
			case AF_INET6:
				result = command("EPRT |2|%s|%d|",
						inet_ntop(data_addr.ss_family,
							&data_addr_sa6->sin6_addr,
							ipstring,
							sizeof(ipstring)),
						ntohs(data_addr_sa6->sin6_port));
		}
		if (result == ERROR && sendport == -1) {
			sendport = 0;
			tmpno = 1;
			goto noport;
		}
		return (result != COMPLETE);
	}
	if (tmpno)
		sendport = 1;
#ifdef IP_TOS
	on = IPTOS_THROUGHPUT;
	if ( (data_addr.ss_family == AF_INET) &&
		(setsockopt(data, IPPROTO_IP, IP_TOS, (char *)&tos,
	  		  sizeof(tos)) < 0) )
		perror("ftp: setsockopt TOS (ignored)");
#endif /* IP_TOS */
	return (0);
bad:
	INTOFF;
	(void) close(data), data = -1;
	INTON;
	if (tmpno)
		sendport = 1;
	return (1);
}

static FILE *
dataconn(const char *lmode)
{
	struct sockaddr_storage from;
	int s, tos;
	socklen_t fromlen = sizeof(from);

        if (passivemode)
            return (fdopen(data, lmode));

	s = accept(data, (struct sockaddr *) &from, &fromlen);
	if (s < 0) {
		perror("ftp: accept");
		(void) close(data), data = -1;
		return (NULL);
	}
	(void) close(data);
	data = s;
#ifdef IP_TOS
	tos = IPTOS_THROUGHPUT;
	if ( (from.ss_family == AF_INET) &&
		(setsockopt(s, IPPROTO_IP, IP_TOS, (char *)&tos,
	  		  sizeof(tos)) < 0) )
		perror("ftp: setsockopt TOS (ignored)");
#endif /* IP_TOS */
	return (fdopen(data, lmode));
}

static void
ptransfer(const char *direction, off_t bytes, 
	  const struct timeval *t0, 
	  const struct timeval *t1)
{
	struct timeval td;
	float s, bs;

	if (verbose) {
		tvsub(&td, t1, t0);
		s = td.tv_sec + (td.tv_usec / 1000000.);
#define	nz(x)	((x) == 0 ? 1 : (x))
		bs = bytes / nz(s);
		if (bs > 1048576.)	/* 1024^2 */
			printf("%jd bytes %s in %.2f secs (%.4f MB/s)\n",
			       (intmax_t) bytes, direction, s,
			       bs / 1048576.);
		else
			printf("%jd bytes %s in %.2f secs (%.4f kB/s)\n",
			       (intmax_t) bytes, direction, s,
			       bs / 1024.);
	}
}

#if 0
tvadd(tsum, t0)
	struct timeval *tsum, *t0;
{

	tsum->tv_sec += t0->tv_sec;
	tsum->tv_usec += t0->tv_usec;
	if (tsum->tv_usec > 1000000)
		tsum->tv_sec++, tsum->tv_usec -= 1000000;
}
#endif

static void
tvsub(struct timeval *tdiff, 
      const struct timeval *t1, 
      const struct timeval *t0)
{

	tdiff->tv_sec = t1->tv_sec - t0->tv_sec;
	tdiff->tv_usec = t1->tv_usec - t0->tv_usec;
	if (tdiff->tv_usec < 0)
		tdiff->tv_sec--, tdiff->tv_usec += 1000000;
}

void
pswitch(int flag)
{
	static struct comvars {
		int connect;
		char name[MAXHOSTNAMELEN];
		struct sockaddr_storage mctl;
		struct sockaddr_storage hctl;
		FILE *in;
		FILE *out;
		int tpe;
		int curtpe;
		int cpnd;
		int sunqe;
		int runqe;
		int mcse;
		int ntflg;
		char nti[17];
		char nto[17];
		int mapflg;
		char mi[MAXPATHLEN];
		char mo[MAXPATHLEN];
	} proxstruct, tmpstruct;
	struct comvars *ip, *op;

	if (flag) {
		if (proxy)
			return;
		INTOFF;
		ip = &tmpstruct;
		op = &proxstruct;
		proxy++;
	} 
	else {
		if (!proxy)
			return;
		INTOFF;
		ip = &proxstruct;
		op = &tmpstruct;
		proxy = 0;
	}
	ip->connect = connected;
	connected = op->connect;
	if (hostname) {
		(void) strncpy(ip->name, hostname, sizeof(ip->name) - 1);
		ip->name[sizeof(ip->name) - 1] = '\0';
	} 
	else {
		ip->name[0] = 0;
	}
	hostname = op->name;
	ip->hctl = hisctladdr;
	hisctladdr = op->hctl;
	ip->mctl = myctladdr;
	myctladdr = op->mctl;
	ip->in = cin;
	cin = op->in;
	ip->out = cout;
	cout = op->out;
	ip->tpe = type;
	type = op->tpe;
	ip->curtpe = curtype;
	curtype = op->curtpe;
	ip->cpnd = cpend;
	cpend = op->cpnd;
	ip->sunqe = sunique;
	sunique = op->sunqe;
	ip->runqe = runique;
	runique = op->runqe;
	ip->mcse = mcase;
	mcase = op->mcse;
	ip->ntflg = ntflag;
	ntflag = op->ntflg;
	(void) strncpy(ip->nti, ntin, sizeof(ip->nti) - 1);
	(ip->nti)[sizeof(ip->nti) - 1] = '\0';
	(void) strcpy(ntin, op->nti);
	(void) strncpy(ip->nto, ntout, sizeof(ip->nto) - 1);
	(ip->nto)[sizeof(ip->nto) - 1] = '\0';
	(void) strcpy(ntout, op->nto);
	ip->mapflg = mapflag;
	mapflag = op->mapflg;
	(void) strncpy(ip->mi, mapin, sizeof(ip->mi) - 1);
	(ip->mi)[sizeof(ip->mi) - 1] = '\0';
	(void) strcpy(mapin, op->mi);
	(void) strncpy(ip->mo, mapout, sizeof(ip->mo) - 1);
	(ip->mo)[sizeof(ip->mo) - 1] = '\0';
	(void) strcpy(mapout, op->mo);
	INTON;
}

static
void
abortpt(int ignore)
{
	(void)ignore;
	printf("\n");
	fflush(stdout);
	ptabflg++;
	mflag = 0;
}

static void
proxtrans(const char *cmd, char *local, char *remote)
{
	volatile int secndflag = 0, prox_type, nfnd;
	const char *volatile cmd2;
	fd_set mask;
	sigjmp_buf jmploc;
	sigjmp_buf *volatile oldtoplevel;

	if (strcmp(cmd, "RETR"))
		cmd2 = "RETR";
	else
		cmd2 = runique ? "STOU" : "STOR";
	if ((prox_type = type) == 0) {
		if (unix_server && unix_proxy)
			prox_type = TYPE_I;
		else
			prox_type = TYPE_A;
	}
	if (curtype != prox_type)
		changetype(prox_type, 1);
	if (command("PASV") != COMPLETE) {
		printf("proxy server does not support third party transfers.\n");
		return;
	}
	pswitch(0);
	if (!connected) {
		printf("No primary connection\n");
		pswitch(1);
		code = -1;
		return;
	}
	if (curtype != prox_type)
		changetype(prox_type, 1);
	if (command("PORT %s", pasv) != COMPLETE) {
		pswitch(1);
		return;
	}
	oldtoplevel = toplevel;
	if (sigsetjmp(jmploc, 1)) {
		abortpt(SIGINT);
		goto abort;
	}
	toplevel = &jmploc;
	if (command("%s %s", cmd, remote) != PRELIM) {
		toplevel = oldtoplevel;
		pswitch(1);
		return;
	}
	sleep(2);
	pswitch(1);
	secndflag++;
	if (command("%s %s", cmd2, local) != PRELIM)
		goto abort;
	ptflag++;
	(void) getreply(0);
	pswitch(0);
	(void) getreply(0);
	toplevel = oldtoplevel;
	pswitch(1);
	ptflag = 0;
	printf("local: %s remote: %s\n", local, remote);
	return;
abort:
	INTOFF;
	ptflag = 0;
	if (strcmp(cmd, "RETR") && !proxy)
		pswitch(1);
	else if (!strcmp(cmd, "RETR") && proxy)
		pswitch(0);
	if (!cpend && !secndflag) {  /* only here if cmd = "STOR" (proxy=1) */
		if (command("%s %s", cmd2, local) != PRELIM) {
			pswitch(0);
			if (cpend)
				abort_remote((FILE *) NULL);
		}
		pswitch(1);
		if (ptabflg)
			code = -1;
		toplevel = oldtoplevel;
		INTON;
		return;
	}
	if (cpend)
		abort_remote((FILE *) NULL);
	pswitch(!proxy);
	if (!cpend && !secndflag) {  /* only if cmd = "RETR" (proxy=1) */
		if (command("%s %s", cmd2, local) != PRELIM) {
			pswitch(0);
			if (cpend)
				abort_remote((FILE *) NULL);
			pswitch(1);
			if (ptabflg)
				code = -1;
			toplevel = oldtoplevel;
			INTON;
			return;
		}
	}
	if (cpend)
		abort_remote((FILE *) NULL);
	pswitch(!proxy);
	if (cpend) {
		FD_ZERO(&mask);
		FD_SET(fileno(cin), &mask);
		if ((nfnd = empty(&mask, fileno(cin), 10)) <= 0) {
			if (nfnd < 0) {
				perror("abort");
			}
			if (ptabflg)
				code = -1;
			lostpeer(0);
		}
		(void) getreply(0);
		(void) getreply(0);
	}
	if (proxy)
		pswitch(0);
	pswitch(1);
	if (ptabflg)
		code = -1;
	toplevel = oldtoplevel;
	INTON;
}

void
reset(void)
{
	fd_set mask;
	int nfnd = 1;

	FD_ZERO(&mask);
	while (nfnd > 0) {
		FD_SET(fileno(cin), &mask);
		if ((nfnd = empty(&mask, fileno(cin), 0)) < 0) {
			perror("reset");
			code = -1;
			INTOFF;
			lostpeer(0);
			INTON;
		}
		else if (nfnd) {
			(void) getreply(0);
		}
	}
}

static char *
gunique(char *local)
{
	static char new[MAXPATHLEN];
	char *cp = rindex(local, '/');
	int d, count=0;
	char ext = '1';

	if (cp)
		*cp = '\0';
	d = access(cp ? local : ".", W_OK);
	if (cp)
		*cp = '/';
	if (d < 0) {
		fprintf(stderr, "local: %s: %s\n", local, strerror(errno));
		return((char *) 0);
	}
	(void) strcpy(new, local);
	cp = new + strlen(new);
	*cp++ = '.';
	while (!d) {
		if (++count == 100) {
			printf("runique: can't find unique file name.\n");
			return((char *) 0);
		}
		*cp++ = ext;
		*cp = '\0';
		if (ext == '9')
			ext = '0';
		else
			ext++;
		if ((d = access(new, F_OK)) < 0)
			break;
		if (ext != '0')
			cp--;
		else if (*(cp - 2) == '.')
			*(cp - 1) = '1';
		else {
			*(cp - 2) = *(cp - 2) + 1;
			cp--;
		}
	}
	return(new);
}

static void
abort_remote(FILE *din)
{
	char buf[BUFSIZ];
	int nfnd, hifd = -1;
	fd_set mask;

	/*
	 * send IAC in urgent mode instead of DM because 4.3BSD places oob mark
	 * after urgent byte rather than before as is protocol now
	 */
	if (cout) {
		snprintf(buf, sizeof(buf), "%c%c%c", IAC, IP, IAC);
		if (send(fileno(cout), buf, 3, MSG_OOB) != 3)
			perror("abort");
		fprintf(cout,"%cABOR\r\n", DM);
		(void) fflush(cout);
	}
	FD_ZERO(&mask);
	if (cin) {
		FD_SET(fileno(cin), &mask);
		hifd = fileno(cin);
	}
	if (din) { 
		FD_SET(fileno(din), &mask);
		if (hifd < fileno(din)) hifd = fileno(din);
	}
	if (hifd >= 0 && (nfnd = empty(&mask, hifd, 10)) <= 0) {
		if (nfnd < 0) {
			perror("abort");
		}
		if (ptabflg)
			code = -1;
		lostpeer(0);
	}
	if (din && FD_ISSET(fileno(din), &mask)) {
		while (read(fileno(din), buf, BUFSIZ) > 0)
			/* LOOP */;
	}
	if (getreply(0) == ERROR && code == 552) {
		/* 552 needed for nic style abort */
		(void) getreply(0);
	}
	(void) getreply(0);
}

static void
printbytes(off_t bytes)
{
	printf("\rBytes transferred: %jd", (intmax_t) bytes);
	fflush(stdout);
}
