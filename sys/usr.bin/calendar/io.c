/*	$OpenBSD: io.c,v 1.24 2003/06/03 02:56:06 millert Exp $	*/

/*
 * Copyright (c) 1989, 1993, 1994
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

#ifndef lint
static const char copyright[] =
"@(#) Copyright (c) 1989, 1993\n\
	The Regents of the University of California.  All rights reserved.\n";
#endif /* not lint */

#include <sys/param.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/wait.h>

#include <limits.h>
#include <ctype.h>
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <locale.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wchar.h>
#include <wctype.h>
#include <time.h>

#include "pathnames.h"
#include "calendar.h"


struct iovec header[] = {
	{"From: ", 6},
	{NULL, 0},
	{" (Reminder Service)\nTo: ", 24},
	{NULL, 0},
	{"\nSubject: ", 10},
	{NULL, 0},
	{"'s Calendar\nPrecedence: bulk\n\n",  30},
};


void
cal(void)
{
	int printing;
	wchar_t *p;
	FILE *fp;
	int l, i, bodun = 0, bodun_maybe = 0;
	int var;
	wchar_t buf[2048 + 1], *prefix = NULL;
	struct event *events, *cur_evt, *ev1, *tmp;
	struct match *m;
	size_t nlen;
	int r;

	events = NULL;
	cur_evt = NULL;
	if ((fp = opencal()) == NULL)
		return;
	for (printing = 0; myfgetws(buf, 2048 + 1, stdin) != NULL;) {
		for (l = wcslen(buf); l > 0 && iswspace(buf[l - 1]); l--)
			;
		buf[l] = L'\0';
		if (buf[0] == L'\0')
			continue;

		if (wcsncmp(buf, L"# ", 2) == 0)
			continue;

		if (wcsncmp(buf, L"LANG=", 5) == 0) {
			setnnames();
			if (!wcscmp(buf + 5, L"ru_RU.KOI8-R") ||
			    !wcscmp(buf + 5, L"uk_UA.KOI8-U") ||
			    !wcscmp(buf + 5, L"by_BY.KOI8-B")) {
				bodun_maybe++;
				bodun = 0;
				if (prefix)
					free(prefix);
				prefix = NULL;
			} else
				bodun_maybe = 0;
			continue;
		}
		if (bodun_maybe && wcsncmp(buf, L"BODUN=", 6) == 0) {
			bodun++;
			if (prefix)
				free(prefix);
			if ((prefix = wcsdup(buf + 6)) == NULL)
				err(1, NULL);
		}
		/* User defined names for special events */
		if ((p = wcschr(buf, L'='))) {
			for (i = 0; i < NUMEV; i++) {
			if (wcsncasecmp(buf, spev[i].name, spev[i].nlen) == 0 &&
			    (p - buf == spev[i].nlen) && buf[spev[i].nlen + 1]) {
				p++;
				if (spev[i].uname != NULL)
					free(spev[i].uname);
				if ((spev[i].uname = wcsdup(p)) == NULL)
					err(1, NULL);
				spev[i].ulen = wcslen(p);
				i = NUMEV + 1;
			}
			}
		if (i > NUMEV)
			continue;
		}
		if (buf[0] != L'\t') {
			printing = (m = isnow(buf, bodun)) ? 1 : 0;
			if ((p = wcschr(buf, L'\t')) == NULL) {
				printing = 0;
				continue;
			}
			/* Need the following to catch hardwired "variable"
			 * dates */
			if (p > buf && p[-1] == L'*')
				var = 1;
			else
				var = 0;
			if (printing) {
				struct match *foo;
				
				ev1 = NULL;
				while (m) {
				cur_evt = (struct event *) malloc(sizeof(struct event));
				if (cur_evt == NULL)
					err(1, NULL);

				cur_evt->when = m->when;
				cur_evt->var = (var + m->var) ? L'*' : L' ';
				if ((cur_evt->tm = malloc(sizeof(struct tm))) == NULL)
					err(1, NULL);
				memcpy(cur_evt->tm, m->tm, sizeof(struct tm));
				if (ev1) {
					cur_evt->desc = ev1->desc;
					cur_evt->ldesc = NULL;
				} else {
					if (m->bodun && prefix) {
						int l1 = wcslen(prefix);
						int l2 = wcslen(p);
						int len = l1 + l2 + 2;
						if ((cur_evt->ldesc =
						    malloc(len * sizeof (wchar_t))) == NULL)
							err(1, "malloc");
						swprintf(cur_evt->ldesc, len,
						    L"\t%ls %ls", prefix, p + 1);
					} else if ((cur_evt->ldesc =
					    wcsdup(p)) == NULL)
						err(1, NULL);
					cur_evt->desc = &(cur_evt->ldesc);
					ev1 = cur_evt;
				}
				insert(&events, cur_evt);
				foo = m;
				m = m->next;
				free(foo->tm);
				free(foo);
				}
			}
		}
		else if (printing) {
			wchar_t *ldesc;
			nlen = wcslen(ev1->ldesc) + wcslen(buf) + 2;
			if ((ldesc = malloc(nlen * sizeof (wchar_t))) == NULL)
				err(1, NULL);
			swprintf(ldesc, nlen, L"%ls\n%ls", ev1->ldesc, buf);
			if (ev1->ldesc)
				free(ev1->ldesc);
			ev1->ldesc = ldesc;
		}
	}
	/* reset the locale before we start output */
	(void) setlocale(LC_ALL, "");
	tmp = events;
	while (tmp) {
		char buf[1024 + 1];

		if (wcstombs(buf, *(tmp->desc), sizeof(buf) - 1) != -1) {
			wchar_t print_date[31];

			wcsftime(print_date, 31, L"%b %d", tmp->tm);
			fprintf(fp, "%ls%lc", print_date, tmp->var);

			if (memchr(buf, '\0', 1024) == NULL)
				buf[1024] = '\0';
			fprintf(fp, "%s\n", buf);
		}

		tmp = tmp->next;
	}
	tmp = events;
	while (tmp) {
		events = tmp;
		if (tmp->ldesc)
			free(tmp->ldesc);
		free(tmp->tm);
		tmp = tmp->next;
		free(events);
	}
	closecal(fp);
}

int
getfield(wchar_t *p, wchar_t **endp, int *flags)
{
	int val, var, i;
	wchar_t *start, savech;

	for (; !iswdigit(*p) && !iswalpha(*p) && *p != L'*' && *p != L'\t'; ++p)
		;
	if (*p == L'*') {			/* `*' is every month */
		*flags |= F_ISMONTH;
		*endp = p+1;
		return (-1);	/* means 'every month' */
	}
	if (iswdigit(*p)) {
		val = wcstol(p, &p, 10);	/* if 0, it's failure */
		for (; !iswdigit(*p) && !iswalpha(*p) && *p != L'*'; ++p)
			;
		*endp = p;
		return (val);
	}
	for (start = p; iswalpha(*++p);)
		;

	/* Sunday-1 */
	if (*p == L'+' || *p == L'-')
	    for(; isdigit(*++p);)
		;

	savech = *p;
	*p = L'\0';

	/* Month */
	if ((val = getmonth(start)) != 0)
		*flags |= F_ISMONTH;

	/* Day */
	else if ((val = getday(start)) != 0) {
	    *flags |= F_ISDAY;

	    /* variable weekday */
	    if ((var = getdayvar(start)) != 0) {
		if (var <= 5 && var >= -4)
		    val += var * 10;
#ifdef DEBUG
		printf("var: %d\n", var);
#endif
	    }
	}

	/* Try specials (Easter, Paskha, ...) */
	else {
		for (i = 0; i < NUMEV; i++) {
			if (wcsncasecmp(start, spev[i].name, spev[i].nlen) == 0) {
				start += spev[i].nlen;
				val = i + 1;
				i = NUMEV + 1;
			} else if (spev[i].uname != NULL &&
			    wcsncasecmp(start, spev[i].uname, spev[i].ulen) == 0) {
				start += spev[i].ulen;
				val = i + 1;
				i = NUMEV + 1;
			}
		}
		if (i > NUMEV) {
			switch(*start) {
			case L'-':
			case L'+':
			   var = wcstol(start, (wchar_t **) NULL, 10);
			   if (var > 365 || var < -365)
				   return (0); /* Someone is just being silly */
			   val += (NUMEV + 1) * var;
			   /* We add one to the matching event and multiply by
			    * (NUMEV + 1) so as not to return 0 if there's a match.
			    * val will overflow if there is an obscenely large
			    * number of special events. */
			   break;
			}
		*flags |= F_SPECIAL;	
		}
		if (!(*flags & F_SPECIAL)) {
		/* undefined rest */
			*p = savech;
			return (0);
		}
	}
	for (*p = savech; !iswdigit(*p) && !iswalpha(*p) && *p != L'*' && *p != L'\t'; ++p)
		;
	*endp = p;
	return (val);
}

/*
 * Try a number of different paths for the calendar file, and return the file
 * descriptor of the first one that works. Also set calendarPath to the file
 * that is opened.
 */
int
openfile()
{
	struct stat st;
	char *home;
	int fd;

	/* Is there a calendar file in the current directory? */
	if ((fd = open(calendarFile, O_RDONLY)) != -1 &&
	    fstat(fd, &st) != -1 && S_ISREG(st.st_mode)) {
	    	if ((calendarPath = strdup(calendarFile)) == NULL)
			err(1, NULL);

		return fd;
	}
	    	
	/* Try the ~/.calendar directory. */
	home = getenv("HOME");
	if (home == NULL || *home == '\0')
		errx(1, "cannot get home directory");

	if ((chdir(home) == 0 &&
	    chdir(calendarHome) == 0 &&
	    (fd = open(calendarFile, O_RDONLY)) != -1)) {
		if ((calendarPath = (char *) malloc(PATH_MAX)) == NULL)
			err(1, NULL);
		snprintf(calendarPath, PATH_MAX, "%s/%s/%s", home, calendarHome, calendarFile);

		return fd;
	}

	/* Try the system-wide calendar file. */
	if ((fd = open(_PATH_DEFAULT, O_RDONLY)) != -1) {
	    	if ((calendarPath = strdup(_PATH_DEFAULT)) == NULL)
			err(1, NULL);

		return fd;
	}

	errx(1, "no calendar file: ``%s'' or ``~/%s/%s''",
	    calendarFile, calendarHome, calendarFile);
}

FILE *
opencal(void)
{
	int pdes[2], fdin;

	/* open up calendar file as stdin */
	fdin = openfile();

	if (pipe(pdes) < 0)
		return (NULL);
	switch (vfork()) {
	case -1:			/* error */
		(void)close(pdes[0]);
		(void)close(pdes[1]);
		return (NULL);
	case 0:
		dup2(fdin, STDIN_FILENO);
		/* child -- set stdout to pipe input */
		if (pdes[1] != STDOUT_FILENO) {
			(void)dup2(pdes[1], STDOUT_FILENO);
			(void)close(pdes[1]);
		}
		(void)close(pdes[0]);
		/* 
		 * Set stderr to /dev/null.  Necessary so that cron does not
		 * wait for cpp to finish if it's running calendar -a.
		 */
		if (doall) {
			int fderr;
			fderr = open(_PATH_DEVNULL, O_WRONLY, 0);
			if (fderr == -1)
				_exit(0);
			(void)dup2(fderr, STDERR_FILENO);
			(void)close(fderr);
		}
		execl(_PATH_CPP, "cpp", "-traditional", "-I.", _PATH_EINCLUDE, _PATH_INCLUDE, (char *)NULL);
		warn(_PATH_CPP);
		_exit(1);
	}
	/* parent -- set stdin to pipe output */
	(void)dup2(pdes[0], STDIN_FILENO);
	(void)close(pdes[0]);
	(void)close(pdes[1]);

	/* not reading all calendar files, just set output to stdout */
	if (!doall)
		return (stdout);

	/* set output to a temporary file, so if no output don't send mail */
	return(tmpfile());
}

void
closecal(fp)
	FILE *fp;
{
	struct stat sbuf;
	int nread, pdes[2], status;
	char buf[1024];

	if (!doall)
		return;

	(void)rewind(fp);
	if (fstat(fileno(fp), &sbuf) || !sbuf.st_size)
		goto done;
	if (pipe(pdes) < 0)
		goto done;
	switch (vfork()) {
	case -1:			/* error */
		(void)close(pdes[0]);
		(void)close(pdes[1]);
		goto done;
	case 0:
		/* child -- set stdin to pipe output */
		if (pdes[0] != STDIN_FILENO) {
			(void)dup2(pdes[0], STDIN_FILENO);
			(void)close(pdes[0]);
		}
		(void)close(pdes[1]);
		execl(_PATH_SENDMAIL, "sendmail", "-i", "-t", "-F",
		    "\"Reminder Service\"", (char *)NULL);
		warn(_PATH_SENDMAIL);
		_exit(1);
	}
	/* parent -- write to pipe input */
	(void)close(pdes[0]);

	header[1].iov_base = header[3].iov_base = pw->pw_name;
	header[1].iov_len = header[3].iov_len = strlen(pw->pw_name);
	writev(pdes[1], header, 7);
	while ((nread = read(fileno(fp), buf, sizeof(buf))) > 0)
		(void)write(pdes[1], buf, nread);
	(void)close(pdes[1]);
done:	(void)fclose(fp);
	while (wait(&status) >= 0)
		;
}


void
insert(head, cur_evt)
	struct event **head;
	struct event *cur_evt;
{
	struct event *tmp, *tmp2;

	if (*head) {
		/* Insert this one in order */
		tmp = *head;
		tmp2 = NULL;
		while (tmp->next &&
		    tmp->when <= cur_evt->when) {
			tmp2 = tmp;
			tmp = tmp->next;
		}
		if (tmp->when > cur_evt->when) {
			cur_evt->next = tmp;
			if (tmp2)
				tmp2->next = cur_evt;
			else
				*head = cur_evt;
		} else {
			cur_evt->next = tmp->next;
			tmp->next = cur_evt;
		}
	} else {
		*head = cur_evt;
		cur_evt->next = NULL;
	}
}
