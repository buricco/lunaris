/*	$OpenBSD: calendar.c,v 1.21 2003/06/10 22:20:45 deraadt Exp $	*/

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

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <err.h>
#include <errno.h>
#include <locale.h>
#include <pwd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "pathnames.h"
#include "calendar.h"
#include "freebsd.h"

void atodays(char, char *, unsigned short *);
unsigned short lookahead = 1, weekend = 2;

char *calendarFile = "calendar";  /* default calendar file */
char *calendarHome = ".calendar"; /* HOME */
char *calendarPath;               /* calendar file we end up using */
char *calendarNoMail = "nomail";  /* don't sent mail if this file exists */

struct passwd *pw;
int doall = 0;
time_t f_time = 0;
int bodun_always = 0;

struct specialev spev[NUMEV];

void childsig(int);

int
main(int argc, char *argv[])
{
	int ch;
	char *caldir;
	unsigned short before = 0;

	(void)setlocale(LC_ALL, "");

	while ((ch = getopt(argc, argv, "abf:l:t:w:A:B:-")) != -1)
		switch (ch) {
		case '-':		/* backward contemptible */
		case 'a':
			if (getuid())
				errx(1, "%s", strerror(EPERM));
			doall = 1;
			break;

		case 'b':
			bodun_always++;
			break;

		case 'f': /* other calendar file */
		        calendarFile = optarg;
			break;

		case 't': /* other date, undocumented, for tests */
			if ((f_time = Mktime(optarg)) <= 0)
				errx(1, "specified date is outside allowed range");
			break;

		case 'w':
			atodays(ch, optarg, &weekend);
			break;

		case 'A': /* days after current date */
		case 'l':
			atodays(ch, optarg, &lookahead);
			break;

		case 'B': /* days before current date */
			atodays(ch, optarg, &before);
			break;

		default:
			usage();
		}
	argc -= optind;
	argv += optind;

	if (argc)
		usage();

	spev_init();

	/* use current time */
	if (f_time <= 0)
	    (void)time(&f_time);

	if (before) {
		/* Move back in time and only look forwards */
		lookahead += before;
		f_time -= SECSPERDAY * before;
	}
	settime(&f_time);

	if (doall) {
		pid_t kid, deadkid;
		int kidstat, kidreaped, runningkids;
		int acstat;
		struct stat sbuf;
		time_t t;
		unsigned int sleeptime;

		signal(SIGCHLD, childsig);
		runningkids = 0;
		t = time(NULL);
		while ((pw = getpwent()) != NULL) {
			acstat = 0;
			/* Avoid unnecessary forks.  The calendar file is only
			 * opened as the user later; if it can't be opened,
			 * it's no big deal.  Also, get to correct directory.
			 * Note that in an NFS environment root may get EACCES
			 * on a chdir(), in which case we have to fork.  As long as
			 * we can chdir() we can stat(), unless the user is
			 * modifying permissions while this is running.
			 */
			if (chdir(pw->pw_dir)) {
				if (errno == EACCES)
					acstat = 1;
				else
					continue;
			}
			if (stat(calendarFile, &sbuf) != 0) {
				if (chdir(calendarHome)) {
					if (errno == EACCES)
						acstat = 1;
					else
						continue;
				}
				if (stat(calendarNoMail, &sbuf) == 0 ||
				    stat(calendarFile, &sbuf) != 0)
					continue;
			}
			sleeptime = USERTIMEOUT;
			switch ((kid = fork())) {
			case -1:	/* error */
				warn("fork");
				continue;
			case 0:	/* child */
				(void)setlocale(LC_ALL, "");
				(void)setegid(pw->pw_gid);
				(void)initgroups(pw->pw_name, pw->pw_gid);
				(void)seteuid(pw->pw_uid);
				if (acstat) {
					if (chdir(pw->pw_dir) ||
					    stat(calendarFile, &sbuf) != 0 ||
					    chdir(calendarHome) || 
					    stat(calendarNoMail, &sbuf) == 0 ||
					    stat(calendarFile, &sbuf) != 0)
						exit(0);
				}
				cal();
				exit(0);
			}
			/* parent: wait a reasonable time, then kill child if
			 * necessary.
			 */
			runningkids++;
			kidreaped = 0;
			do {
				sleeptime = sleep(sleeptime);
				/* Note that there is the possibility, if the sleep
				 * stops early due to some other signal, of the child
				 * terminating and not getting detected during the next
				 * sleep.  In that unlikely worst case, we just sleep
				 * too long for that user.
				 */
				for (;;) {
					deadkid = waitpid(-1, &kidstat, WNOHANG);
					if (deadkid <= 0)
						break;
					runningkids--;
					if (deadkid == kid) {
						kidreaped = 1;
						sleeptime = 0;
					}
				}
			} while (sleeptime);

			if (!kidreaped) {
				/* It doesn't _really_ matter if the kill fails, e.g.
				 * if there's only a zombie now.
				 */
				(void)kill(kid, SIGTERM);
				warnx("uid %u did not finish in time", pw->pw_uid);
			}
			if (time(NULL) - t >= SECSPERDAY)
				errx(2, "'calendar -a' took more than a day; stopped at uid %u",
				    pw->pw_uid);
		}
		for (;;) {
			deadkid = waitpid(-1, &kidstat, WNOHANG);
			if (deadkid <= 0)
				break;
			runningkids--;
		}
		if (runningkids)
			warnx(
"%d child processes still running when 'calendar -a' finished", runningkids);
	}
	else if ((caldir = getenv("CALENDAR_DIR")) != NULL) {
		if(!chdir(caldir))
			cal();
	} else
		cal();

	exit(0);
}

void
atodays(char ch, char *optarg, unsigned short *days)
{
	int u;

	u = atoi(optarg);
	if ((u < 0) || (u > 366))
		warnx("warning: -%c %d out of range 0-366, ignored.\n", ch, u);
	else
		*days = u;
}

void
usage(void)
{
	(void)fprintf(stderr,
	    "usage: calendar [-a] [-A num] [-b] [-B num] [-l num] [-w num] "
	    "[-t dd[.mm[.year]]] [-f calendarfile]\n");
	exit(1);
}


void
childsig(int signo)
{
}
