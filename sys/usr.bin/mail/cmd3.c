/*	$NetBSD: cmd3.c,v 1.25 2003/11/10 21:40:22 ross Exp $	*/

/*
 * Copyright (c) 1980, 1993
 *	The Regents of the University of California.  All rights reserved.
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

#include <sys/cdefs.h>
#ifndef lint
#if 0
static char sccsid[] = "@(#)cmd3.c	8.2 (Berkeley) 4/20/95";
#else
//__RCSID("$NetBSD: cmd3.c,v 1.25 2003/11/10 21:40:22 ross Exp $");
#endif
#endif /* not lint */

#include "rcv.h"
#include "extern.h"

/*
 * Mail -- a mail program
 *
 * Still more user commands.
 */
static int diction(const void *, const void *);

/*
 * Process a shell escape by saving signals, ignoring signals,
 * and forking a sh -c
 */
int
shell(void *v)
{
	char *str = v;
	sig_t sigint = signal(SIGINT, SIG_IGN);
	char *shellcmd;
	char cmd[BUFSIZ];

	(void)strcpy(cmd, str);
	if (bangexp(cmd) < 0)
		return 1;
	if ((shellcmd = value("SHELL")) == NULL)
		shellcmd = _PATH_CSHELL;
	(void)run_command(shellcmd, 0, 0, 1, "-c", cmd, NULL);
	(void)signal(SIGINT, sigint);
	printf("!\n");
	return 0;
}

/*
 * Fork an interactive shell.
 */
/*ARGSUSED*/
int
dosh(void *v)
{
	sig_t sigint = signal(SIGINT, SIG_IGN);
	char *shellcmd;

	if ((shellcmd = value("SHELL")) == NULL)
		shellcmd = _PATH_CSHELL;
	(void)run_command(shellcmd, 0, 0, 1, NULL);
	(void)signal(SIGINT, sigint);
	putchar('\n');
	return 0;
}

/*
 * Expand the shell escape by expanding unescaped !'s into the
 * last issued command where possible.
 */

char	lastbang[128];

int
bangexp(char *str)
{
	char bangbuf[BUFSIZ];
	char *cp, *cp2;
	int n;
	int changed = 0;

	cp = str;
	cp2 = bangbuf;
	n = BUFSIZ;
	while (*cp) {
		if (*cp == '!') {
			if (n < strlen(lastbang)) {
overf:
				printf("Command buffer overflow\n");
				return(-1);
			}
			changed++;
			strcpy(cp2, lastbang);
			cp2 += strlen(lastbang);
			n -= strlen(lastbang);
			cp++;
			continue;
		}
		if (*cp == '\\' && cp[1] == '!') {
			if (--n <= 1)
				goto overf;
			*cp2++ = '!';
			cp += 2;
			changed++;
		}
		if (--n <= 1)
			goto overf;
		*cp2++ = *cp++;
	}
	*cp2 = 0;
	if (changed) {
		printf("!%s\n", bangbuf);
		fflush(stdout);
	}
	strcpy(str, bangbuf);
	strncpy(lastbang, bangbuf, 128);
	lastbang[127] = 0;
	return(0);
}

/*
 * Print out a nice help message from some file or another.
 */

int
help(void *v)
{
	int c;
	FILE *f;

	if ((f = Fopen(_PATH_HELP, "r")) == NULL) {
		warn(_PATH_HELP);
		return(1);
	}
	while ((c = getc(f)) != EOF)
		putchar(c);
	Fclose(f);
	return(0);
}

/*
 * Change user's working directory.
 */
int
schdir(void *v)
{
	char **arglist = v;
	char *cp;

	if (*arglist == NULL)
		cp = homedir;
	else
		if ((cp = expand(*arglist)) == NULL)
			return(1);
	if (chdir(cp) < 0) {
		warn("%s", cp);
		return(1);
	}
	return 0;
}

int
respond(void *v)
{
	int *msgvec = v;
	if (value("Replyall") == NULL)
		return (_respond(msgvec));
	else
		return (_Respond(msgvec));
}

/*
 * Reply to a list of messages.  Extract each name from the
 * message header and send them off to mail1()
 */
int
_respond(int *msgvec)
{
	struct message *mp;
	char *cp, *rcv, *replyto;
	char **ap;
	struct name *np;
	struct header head;

	if (msgvec[1] != 0) {
		printf("Sorry, can't reply to multiple messages at once\n");
		return(1);
	}
	mp = &message[msgvec[0] - 1];
	touch(mp);
	dot = mp;
	if ((rcv = skin(hfield("from", mp))) == NULL)
		rcv = skin(nameof(mp, 1));
	if ((replyto = skin(hfield("reply-to", mp))) != NULL)
		np = extract(replyto, GTO);
	else if ((cp = skin(hfield("to", mp))) != NULL)
		np = extract(cp, GTO);
	else
		np = NULL;
	np = elide(np);
	/*
	 * Delete my name from the reply list,
	 * and with it, all my alternate names.
	 */
	np = delname(np, myname);
	if (altnames)
		for (ap = altnames; *ap; ap++)
			np = delname(np, *ap);
	if (np != NULL && replyto == NULL)
		np = cat(np, extract(rcv, GTO));
	else if (np == NULL) {
		if (replyto != NULL)
			printf("Empty reply-to field -- replying to author\n");
		np = extract(rcv, GTO);
	}
	head.h_to = np;
	if ((head.h_subject = hfield("subject", mp)) == NULL)
		head.h_subject = hfield("subj", mp);
	head.h_subject = reedit(head.h_subject);
	if (replyto == NULL && (cp = skin(hfield("cc", mp))) != NULL) {
		np = elide(extract(cp, GCC));
		np = delname(np, myname);
		if (altnames != 0)
			for (ap = altnames; *ap; ap++)
				np = delname(np, *ap);
		head.h_cc = np;
	} else
		head.h_cc = NULL;
	head.h_bcc = NULL;
	head.h_smopts = NULL;
	mail1(&head, 1);
	return(0);
}

/*
 * Modify the subject we are replying to to begin with Re: if
 * it does not already.
 */
char *
reedit(char *subj)
{
	char *newsubj;

	if (subj == NULL)
		return NULL;
	if ((subj[0] == 'r' || subj[0] == 'R') &&
	    (subj[1] == 'e' || subj[1] == 'E') &&
	    subj[2] == ':')
		return subj;
	newsubj = salloc(strlen(subj) + 5);
	strcpy(newsubj, "Re: ");
	strcpy(newsubj + 4, subj);
	return newsubj;
}

/*
 * Preserve the named messages, so that they will be sent
 * back to the system mailbox.
 */
int
preserve(void *v)
{
	int *msgvec = v;
	struct message *mp;
	int *ip, mesg;

	if (edit) {
		printf("Cannot \"preserve\" in edit mode\n");
		return(1);
	}
	for (ip = msgvec; *ip != 0; ip++) {
		mesg = *ip;
		mp = &message[mesg-1];
		mp->m_flag |= MPRESERVE;
		mp->m_flag &= ~MBOX;
		dot = mp;
	}
	return(0);
}

/*
 * Mark all given messages as unread.
 */
int
unread(void *v)
{
	int *msgvec = v;
	int *ip;

	for (ip = msgvec; *ip != 0; ip++) {
		dot = &message[*ip-1];
		dot->m_flag &= ~(MREAD|MTOUCH);
		dot->m_flag |= MSTATUS;
	}
	return(0);
}

/*
 * Print the size of each message.
 */
int
messize(void *v)
{
	int *msgvec = v;
	struct message *mp;
	int *ip, mesg;

	for (ip = msgvec; *ip != 0; ip++) {
		mesg = *ip;
		mp = &message[mesg-1];
		printf("%d: %ld/%ld\n", mesg, mp->m_blines, mp->m_size);
	}
	return(0);
}

/*
 * Quit quickly.  If we are sourcing, just pop the input level
 * by returning an error.
 */
int
rexit(void *v)
{
	if (sourcing)
		return(1);
	exit(0);
	/*NOTREACHED*/
}

/*
 * Set or display a variable value.  Syntax is similar to that
 * of csh.
 */
int
set(void *v)
{
	char **arglist = v;
	struct var *vp;
	char *cp;
	char varbuf[BUFSIZ], **ap, **p;
	int errs, h, s;
	size_t l;

	if (*arglist == NULL) {
		for (h = 0, s = 1; h < HSHSIZE; h++)
			for (vp = variables[h]; vp != NULL; vp = vp->v_link)
				s++;
		ap = (char **) salloc(s * sizeof *ap);
		for (h = 0, p = ap; h < HSHSIZE; h++)
			for (vp = variables[h]; vp != NULL; vp = vp->v_link)
				*p++ = vp->v_name;
		*p = NULL;
		sort(ap);
		for (p = ap; *p != NULL; p++)
			printf("%s\t%s\n", *p, value(*p));
		return(0);
	}
	errs = 0;
	for (ap = arglist; *ap != NULL; ap++) {
		cp = *ap;
		while (*cp != '=' && *cp != '\0')
			++cp;
		l = cp - *ap;
		if (l >= sizeof varbuf)
			l = sizeof varbuf - 1;
		strncpy(varbuf, *ap, l);
		varbuf[l] = '\0';
		if (*cp == '\0')
			cp = "";
		else
			cp++;
		if (equal(varbuf, "")) {
			printf("Non-null variable name required\n");
			errs++;
			continue;
		}
		assign(varbuf, cp);
	}
	return(errs);
}

/*
 * Unset a bunch of variable values.
 */
int
unset(void *v)
{
	char **arglist = v;
	struct var *vp, *vp2;
	int errs, h;
	char **ap;

	errs = 0;
	for (ap = arglist; *ap != NULL; ap++) {
		if ((vp2 = lookup(*ap)) == NULL) {
			if (getenv(*ap)) {
				unsetenv(*ap);
			} else if (!sourcing) {
				printf("\"%s\": undefined variable\n", *ap);
				errs++;
			}
			continue;
		}
		h = hash(*ap);
		if (vp2 == variables[h]) {
			variables[h] = variables[h]->v_link;
			v_free(vp2->v_name);
                        v_free(vp2->v_value);
			free((char *)vp2);
			continue;
		}
		for (vp = variables[h]; vp->v_link != vp2; vp = vp->v_link)
			;
		vp->v_link = vp2->v_link;
                v_free(vp2->v_name);
                v_free(vp2->v_value);
		free((char *) vp2);
	}
	return(errs);
}

/*
 * Put add users to a group.
 */
int
group(void *v)
{
	char **argv = v;
	struct grouphead *gh;
	struct group *gp;
	int h;
	int s;
	char **ap, *gname, **p;

	if (*argv == NULL) {
		for (h = 0, s = 1; h < HSHSIZE; h++)
			for (gh = groups[h]; gh != NULL; gh = gh->g_link)
				s++;
		ap = (char **) salloc(s * sizeof *ap);
		for (h = 0, p = ap; h < HSHSIZE; h++)
			for (gh = groups[h]; gh != NULL; gh = gh->g_link)
				*p++ = gh->g_name;
		*p = NULL;
		sort(ap);
		for (p = ap; *p != NULL; p++)
			printgroup(*p);
		return(0);
	}
	if (argv[1] == NULL) {
		printgroup(*argv);
		return(0);
	}
	gname = *argv;
	h = hash(gname);
	if ((gh = findgroup(gname)) == NULL) {
		gh = (struct grouphead *) calloc(1, sizeof *gh);
		gh->g_name = vcopy(gname);
		gh->g_list = NULL;
		gh->g_link = groups[h];
		groups[h] = gh;
	}

	/*
	 * Insert names from the command list into the group.
	 * Who cares if there are duplicates?  They get tossed
	 * later anyway.
	 */

	for (ap = argv+1; *ap != NULL; ap++) {
		gp = (struct group *) calloc(1, sizeof *gp);
		gp->ge_name = vcopy(*ap);
		gp->ge_link = gh->g_list;
		gh->g_list = gp;
	}
	return(0);
}

/*
 * Sort the passed string vecotor into ascending dictionary
 * order.
 */
void
sort(char **list)
{
	char **ap;

	for (ap = list; *ap != NULL; ap++)
		;
	if (ap-list < 2)
		return;
	qsort(list, ap-list, sizeof(*list), diction);
}

/*
 * Do a dictionary order comparison of the arguments from
 * qsort.
 */
static int
diction(const void *a, const void *b)
{
	return(strcmp(*(char **)a, *(char **)b));
}

/*
 * The do nothing command for comments.
 */

/*ARGSUSED*/
int
null(void *v)
{
	return 0;
}

/*
 * Change to another file.  With no argument, print information about
 * the current file.
 */
int
file(void *v)
{
	char **argv = v;

	if (argv[0] == NULL) {
		newfileinfo(0);
		return 0;
	}
	if (setfile(*argv) < 0)
		return 1;
	announce();
	return 0;
}

/*
 * Expand file names like echo
 */
int
echo(void *v)
{
	char **argv = v;
	char **ap;
	char *cp;

	for (ap = argv; *ap != NULL; ap++) {
		cp = *ap;
		if ((cp = expand(cp)) != NULL) {
			if (ap != argv)
				putchar(' ');
			printf("%s", cp);
		}
	}
	putchar('\n');
	return 0;
}

int
Respond(void *v)
{
	int *msgvec = v;
	if (value("Replyall") == NULL)
		return (_Respond(msgvec));
	else
		return (_respond(msgvec));
}

/*
 * Reply to a series of messages by simply mailing to the senders
 * and not messing around with the To: and Cc: lists as in normal
 * reply.
 */
int
_Respond(int msgvec[])
{
	struct header head;
	struct message *mp;
	int *ap;
	char *cp;

	head.h_to = NULL;
	for (ap = msgvec; *ap != 0; ap++) {
		mp = &message[*ap - 1];
		touch(mp);
		dot = mp;
		if ((cp = skin(hfield("from", mp))) == NULL)
			cp = skin(nameof(mp, 2));
		head.h_to = cat(head.h_to, extract(cp, GTO));
	}
	if (head.h_to == NULL)
		return 0;
	mp = &message[msgvec[0] - 1];
	if ((head.h_subject = hfield("subject", mp)) == NULL)
		head.h_subject = hfield("subj", mp);
	head.h_subject = reedit(head.h_subject);
	head.h_cc = NULL;
	head.h_bcc = NULL;
	head.h_smopts = NULL;
	mail1(&head, 1);
	return 0;
}

/*
 * Conditional commands.  These allow one to parameterize one's
 * .mailrc and do some things if sending, others if receiving.
 */
int
ifcmd(void *v)
{
	char **argv = v;
	char *cp;

	if (cond != CANY) {
		printf("Illegal nested \"if\"\n");
		return(1);
	}
	cond = CANY;
	cp = argv[0];
	switch (*cp) {
	case 'r': case 'R':
		cond = CRCV;
		break;

	case 's': case 'S':
		cond = CSEND;
		break;

	default:
		printf("Unrecognized if-keyword: \"%s\"\n", cp);
		return(1);
	}
	return(0);
}

/*
 * Implement 'else'.  This is pretty simple -- we just
 * flip over the conditional flag.
 */
int
elsecmd(void *v)
{

	switch (cond) {
	case CANY:
		printf("\"Else\" without matching \"if\"\n");
		return(1);

	case CSEND:
		cond = CRCV;
		break;

	case CRCV:
		cond = CSEND;
		break;

	default:
		printf("Mail's idea of conditions is screwed up\n");
		cond = CANY;
		break;
	}
	return(0);
}

/*
 * End of if statement.  Just set cond back to anything.
 */
int
endifcmd(void *v)
{

	if (cond == CANY) {
		printf("\"Endif\" without matching \"if\"\n");
		return(1);
	}
	cond = CANY;
	return(0);
}

/*
 * Set the list of alternate names.
 */
int
alternates(void *v)
{
	char **namelist = v;
	int c;
	char **ap, **ap2, *cp;

	c = argcount(namelist) + 1;
	if (c == 1) {
		if (altnames == 0)
			return(0);
		for (ap = altnames; *ap; ap++)
			printf("%s ", *ap);
		printf("\n");
		return(0);
	}
	if (altnames != 0)
		free((char *) altnames);
	altnames = (char **) calloc((unsigned) c, sizeof (char *));
	for (ap = namelist, ap2 = altnames; *ap; ap++, ap2++) {
		cp = (char *) calloc((unsigned) strlen(*ap) + 1, sizeof (char));
		strcpy(cp, *ap);
		*ap2 = cp;
	}
	*ap2 = 0;
	return(0);
}
