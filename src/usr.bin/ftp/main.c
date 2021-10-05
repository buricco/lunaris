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

char copyright[] =
  "@(#) Copyright (c) 1985, 1989 Regents of the University of California.\n"
  "All rights reserved.\n";

/*
 * from: @(#)main.c	5.18 (Berkeley) 3/1/91
 */
char main_rcsid[] = 
  "$Id: main.c,v 1.15 1999/10/02 13:25:23 netbug Exp $";


/*
 * FTP User Program -- Command Interface.
 */
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>

/* #include <arpa/ftp.h>	<--- unused? */

#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <netdb.h>
#include <pwd.h>
#include <obstack.h>
#ifdef	__USE_READLINE__
#include <readline/readline.h>
#include <readline/history.h>
#endif

#define Extern
#include "ftp_var.h"
#include "main.h"

#define obstack_chunk_alloc malloc
#define obstack_chunk_free free

int traceflag = 0;
const char *home = "/";

int suppressint;
volatile int intpending;
int intrnewline;
struct obstack mainobstack;
struct obstack lineobstack;

extern FILE *cin;
extern FILE *cout;
extern int data;
extern struct cmd cmdtab[];
extern int NCMDS;

void intr(int);
void lostpeer(int);
void help(int argc, char *argv[]);

static void inthandler(int);
static void cmdscanner(int top);
static char *slurpstring(void);
static void resetstack(struct obstack *);

static
void
usage(void)
{
	printf("\n\tUsage: { ftp | pftp } [-46pinegvtd] [hostname]\n");
	printf("\t   -4: use IPv4 addresses only\n");
	printf("\t   -6: use IPv6, nothing else\n");
	printf("\t   -p: enable passive mode (default for pftp)\n");
	printf("\t   -i: turn off prompting during mget\n");
	printf("\t   -n: inhibit auto-login\n");
	printf("\t   -e: disable readline support, if present\n");
	printf("\t   -g: disable filename globbing\n");
	printf("\t   -v: verbose mode\n");
	printf("\t   -t: enable packet tracing [nonfunctional]\n");
	printf("\t   -d: enable debugging\n");
	printf("\n");
}

int
main(volatile int argc, char **volatile argv)
{
	register char *cp;
	struct servent *sp;
	int top;
	struct passwd *pw = NULL;
	char homedir[MAXPATHLEN];
	sigjmp_buf jmploc;

	tick = 0;

	sp = getservbyname("ftp", "tcp");
	if (sp == 0) {
		fprintf(stderr, "ftp: ftp/tcp: unknown service\n");
		exit(1);
	}
	ftp_port = sp->s_port;
	doglob = 1;
	interactive = 1;
	autologin = 1;
	passivemode = 0;
	usefamily = AF_UNSPEC;	/* the default is to allow any family */

        cp = strrchr(argv[0], '/');
        cp = (cp == NULL) ? argv[0] : cp+1;
        if (strcmp(cp, "pftp") == 0)
            passivemode = 1;

	argc--, argv++;
	while (argc > 0 && **argv == '-') {
		for (cp = *argv + 1; *cp; cp++)
			switch (*cp) {

			case '4':
				usefamily = AF_INET;
				break;

			case '6':
				usefamily = AF_INET6;
				break;

			case 'd':
				options |= SO_DEBUG;
				debug++;
				break;
			
			case 'v':
				verbose++;
				break;

			case 't':
				traceflag++;
				break;

			case 'i':
				interactive = 0;
				break;

			case 'n':
				autologin = 0;
				break;

			case 'p':
				passivemode = 1;
				break;

			case 'g':
				doglob = 0;
				break;
				
			case 'e':
				rl_inhibit = 1;
				break;
				
			case 'h':
				usage();
				exit(0);

			default:
				fprintf(stdout,
				  "ftp: %c: unknown option\n", *cp);
				exit(1);
			}
		argc--, argv++;
	}
	fromatty = isatty(fileno(stdin));

#ifdef __USE_READLINE__
	/* 
	 * Set terminal type so libreadline can parse .inputrc correctly
	 */
	if (fromatty && !rl_inhibit) {
		rl_terminal_name = getenv("TERM");
	}
#endif

	if (fromatty)
		verbose++;
	cpend = 0;	/* no pending replies */
	proxy = 0;	/* proxy not active */
	crflag = 1;	/* strip c.r. on ascii gets */
	sendport = -1;	/* not using ports */
	qcflag = isatty(fileno(stdout));
	/*
	 * Set up the home directory in case we're globbing.
	 */
	cp = getlogin();
	if (cp != NULL) {
		pw = getpwnam(cp);
	}
	if (pw == NULL)
		pw = getpwuid(getuid());
	if (pw != NULL) {
		strncpy(homedir, pw->pw_dir, sizeof(homedir));
		homedir[sizeof(homedir)-1] = 0;
		home = homedir;
	}
	/*
	 * We need this since we want to return from unsafe library calls ASAP
	 * when a SIGINT happens.
	 */
	siginterrupt(SIGINT, 1);
	toplevel = &jmploc;
	obstack_init(&mainobstack);
	obstack_init(&lineobstack);
	if (argc > 0) {
		if (sigsetjmp(jmploc, 1))
			exit(0);
		(void) signal(SIGINT, inthandler);
		(void) signal(SIGPIPE, SIG_IGN);
		setpeer(argc + 1, argv - 1);
		resetstack(&mainobstack);
		resetstack(&lineobstack);
	}
	top = sigsetjmp(jmploc, 1) == 0;
	if (top) {
		(void) signal(SIGINT, inthandler);
		(void) signal(SIGPIPE, SIG_IGN);
	} else {
		INTOFF;
		resetstack(&mainobstack);
		resetstack(&lineobstack);
		INTON;
		pswitch(0);
	}
	for (;;) {
		cmdscanner(top);
		top = 1;
	}
}

void
intr(int ignore)
{
	(void)ignore;

	intpending = 0;
	siglongjmp(*toplevel, 1);
}

static void
inthandler(int sig)
{
	if (intrnewline) {
		putchar('\n');
		fflush(stdout);
	}
	if (suppressint) {
		intpending++;
		return;
	}
	intr(sig);
}

/*
 * Must be called with interrupts off.
 */
void
lostpeer(int ignore)
{
	(void)ignore;

	if (connected) {
		if (cout != NULL) {
			shutdown(fileno(cout), 1+1);
			fclose(cout);
			cout = NULL;
		}
		if (cin != NULL) {
			fclose(cin);
			cin = NULL;
		}
		if (data >= 0) {
			shutdown(data, 1+1);
			close(data);
			data = -1;
		}
		connected = 0;
	}
	pswitch(1);
	if (connected) {
		if (cout != NULL) {
			shutdown(fileno(cout), 1+1);
			fclose(cout);
			cout = NULL;
		}
		if (cin != NULL) {
			fclose(cin);
			cin = NULL;
		}
		connected = 0;
	}
	proxflag = 0;
	pswitch(0);
}

/*char *
tail(filename)
	char *filename;
{
	register char *s;
	
	while (*filename) {
		s = rindex(filename, '/');
		if (s == NULL)
			break;
		if (s[1])
			return (s + 1);
		*s = '\0';
	}
	return (filename);
}
*/

static char *get_input_line(void)
{
	char *lineread;
	size_t size;
	ssize_t len;

#ifdef __USE_READLINE__
	if (fromatty && !rl_inhibit) {
		lineread = readline("ftp> ");
		INTOFF;
		if (!lineread) goto err;
		if (lineread[0]) add_history(lineread);
		len = strlen(lineread);
		goto out;
	}
#endif
	if (fromatty) {
		printf("ftp> ");
		fflush(stdout);
	}
	size = 0;
	lineread = 0;
	INTOFF;
	len = getline(&lineread, &size, stdin);
	if (len == -1 || !lineread) {
err:
		INTON;
		return NULL;
	}
out:
	line = obstack_copy(&lineobstack, lineread, len + 1);
	free (lineread);
	INTON;
	return line;
}


/*
 * Command parser.
 */
static void
cmdscanner(int top)
{
	int margc;
	char *marg;
	char **margv;
	register struct cmd *c;
	register int l;
	int first = 1;

	if (!top)
		(void) putchar('\n');
	for (;;) {
		if (!first) {
			INTOFF;
			obstack_free(&lineobstack, line);
			resetstack(&mainobstack);
			INTON;
		}
		first = 0;
		if (!get_input_line()) {
			quit();
		}
		l = strlen(line);
		if (l == 0)
			break;
		if (line[--l] == '\n') {
			if (l == 0)
				break;
			line[l] = '\0';
		} 
		margv = makeargv(&margc, &marg);
		if (margc == 0) {
			continue;
		}
		c = getcmd(margv[0]);
		if (c == (struct cmd *)-1) {
			printf("?Ambiguous command\n");
			continue;
		}
		if (c == NULL) {
			printf("?Invalid command\n");
			continue;
		}
		if (c->c_conn && !connected) {
			printf("Not connected.\n");
			continue;
		}
		if (c->c_handler_v) c->c_handler_v(margc, margv);
		else if (c->c_handler_0) c->c_handler_0();
		else c->c_handler_1(marg);

		if (bell && c->c_bell) putchar('\007');
		if (c->c_handler_v != help)
			break;
	}
	INTOFF;
	obstack_free(&lineobstack, line);
	resetstack(&mainobstack);
	INTON;
}

struct cmd *
getcmd(const char *name)
{
	const char *p, *q;
	struct cmd *c, *found;
	int nmatches, longest;

	longest = 0;
	nmatches = 0;
	found = 0;
	for (c = cmdtab; (p = c->c_name) != NULL; c++) {
		for (q = name; *q == *p++; q++)
			if (*q == 0)		/* exact match? */
				return (c);
		if (!*q) {			/* the name was a prefix */
			if (q - name > longest) {
				longest = q - name;
				nmatches = 1;
				found = c;
			} else if (q - name == longest)
				nmatches++;
		}
	}
	if (nmatches > 1)
		return ((struct cmd *)-1);
	return (found);
}

/*
 * Slice a string up into argc/argv.
 */

int slrflag;

char **
makeargv(int *pargc, char **parg)
{
        static char **rargv = NULL;
       static int arglimit = 0;
	int rargc = 0;
	char **argp;

       if (arglimit == 0) {
               arglimit = 10;
               rargv = malloc(arglimit * sizeof(char*));
               if (rargv == NULL) fatal ("Out of memory");
       }
	INTOFF;
	argbuf = obstack_alloc(&mainobstack, strlen(line) + 1);
	INTON;
	argp = rargv;
	stringbase = line;		/* scan from first of buffer */
	argbase = argbuf;		/* store from first of buffer */
	slrflag = 0;
       while ((*argp++ = slurpstring())!=NULL) {
		rargc++;
               if (rargc == arglimit) {
                       rargv = realloc(rargv, (arglimit+10) * sizeof(char*));
                       if (rargv == NULL) fatal ("Out of memory");
                       argp = rargv + arglimit;
                       arglimit += 10;
               }
       }
	*pargc = rargc;
	if (parg) *parg = altarg;
	return rargv;
}

/*
 * Parse string into argbuf;
 * implemented with FSM to
 * handle quoting and strings
 */
static
char *
slurpstring(void)
{
	static char excl[] = "!", dols[] = "$";

	int got_one = 0;
	register char *sb = stringbase;
	register char *ap = argbase;
	char *tmp = argbase;		/* will return this if token found */

	if (*sb == '!' || *sb == '$') {	/* recognize ! as a token for shell */
		switch (slrflag) {	/* and $ as token for macro invoke */
			case 0:
				slrflag++;
				stringbase++;
				return ((*sb == '!') ? excl : dols);
				/* NOTREACHED */
			case 1:
				slrflag++;
				altarg = stringbase;
				break;
			default:
				break;
		}
	}

S0:
	switch (*sb) {

	case '\0':
		goto OUT;

	case ' ':
	case '\t':
		sb++; goto S0;

	default:
		switch (slrflag) {
			case 0:
				slrflag++;
				break;
			case 1:
				slrflag++;
				altarg = sb;
				break;
			default:
				break;
		}
		goto S1;
	}

S1:
	switch (*sb) {

	case ' ':
	case '\t':
	case '\0':
		goto OUT;	/* end of token */

	case '\\':
		sb++; goto S2;	/* slurp next character */

	case '"':
		sb++; goto S3;	/* slurp quoted string */

	default:
		*ap++ = *sb++;	/* add character to token */
		got_one = 1;
		goto S1;
	}

S2:
	switch (*sb) {

	case '\0':
		goto OUT;

	default:
		*ap++ = *sb++;
		got_one = 1;
		goto S1;
	}

S3:
	switch (*sb) {

	case '\0':
		goto OUT;

	case '"':
		sb++; goto S1;

	default:
		*ap++ = *sb++;
		got_one = 1;
		goto S3;
	}

OUT:
	if (got_one)
		*ap++ = '\0';
	argbase = ap;			/* update storage pointer */
	stringbase = sb;		/* update scan pointer */
	if (got_one) {
		return(tmp);
	}
	switch (slrflag) {
		case 0:
			slrflag++;
			break;
		case 1:
			slrflag++;
			altarg = NULL;
			break;
		default:
			break;
	}
	return NULL;
}

#define HELPINDENT ((int) sizeof ("directory"))

/*
 * Help command.
 * Call each command handler with argc == 0 and argv[0] == name.
 */
void
help(int argc, char *argv[])
{
	struct cmd *c;

	if (argc == 1) {
		int i, j, w;
		unsigned k;
		int columns, width = 0, lines;

		printf("Commands may be abbreviated.  Commands are:\n\n");
		for (c = cmdtab; c < &cmdtab[NCMDS]; c++) {
			int len = strlen(c->c_name);

			if (len > width)
				width = len;
		}
		width = (width + 8) &~ 7;
		columns = 80 / width;
		if (columns == 0)
			columns = 1;
		lines = (NCMDS + columns - 1) / columns;
		for (i = 0; i < lines; i++) {
			for (j = 0; j < columns; j++) {
				c = cmdtab + j * lines + i;
				if (c->c_name && (!proxy || c->c_proxy)) {
					printf("%s", c->c_name);
				}
				else if (c->c_name) {
					for (k=0; k < strlen(c->c_name); k++) {
						(void) putchar(' ');
					}
				}
				if (c + lines >= &cmdtab[NCMDS]) {
					printf("\n");
					break;
				}
				w = strlen(c->c_name);
				while (w < width) {
					w = (w + 8) &~ 7;
					(void) putchar('\t');
				}
			}
		}
		return;
	}
	while (--argc > 0) {
		register char *arg;
		arg = *++argv;
		c = getcmd(arg);
		if (c == (struct cmd *)-1)
			printf("?Ambiguous help command %s\n", arg);
		else if (c == NULL)
			printf("?Invalid help command %s\n", arg);
		else
			printf("%-*s\t%s\n", HELPINDENT,
				c->c_name, c->c_help);
	}
}

static void
resetstack(struct obstack *stack)
{
	obstack_free(stack, 0);
	obstack_init(stack);
}
