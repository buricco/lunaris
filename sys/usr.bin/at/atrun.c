/*
 * Copyright (c) 1977-1995 by Robert Swartz.
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions are met:
 * 
 *   1. Redistributions of source code must retain the above copyright notice, 
 *      this list of conditions and the following disclaimer.
 *   2. Redistributions in binary form must reproduce the above copyright 
 *      notice, this list of conditions and the following disclaimer in the 
 *      documentation and/or other materials provided with the distribution.
 *   3. Neither the name of the copyright holder nor the names of its 
 *      contributors may be used to endorse or promote products derived from 
 *      this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE 
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
 * POSSIBILITY OF SUCH DAMAGE.
 */

/* $Header: /newbits/usr/lib/RCS/atrun.c,v 1.1 91/04/19 12:26:17 bin Exp $ */

/*
 * Atrun is the daemon which is regularly invoked by cron inorder
 * to run scripts saved by at.  It runs the files by changeing its
 * uid and gid to the owner and then running a shell on them.
 * In order to have the ability to set its uid and group id, it
 * must be set uid to root.
 * Since anyone can write into the at spooling directory, for a
 * script to be run, it must have the set uid, set gid and owner
 * execute bits set.
 *
 * $Log:	/newbits/usr/lib/RCS/atrun.c,v $
 * Revision 1.1	91/04/19  12:26:17 	bin
 * Initial revision
 * 
 * Revision 1.1	89/04/03  13:02:41 	src
 * Initial revision
 * 
 *	85/02/11	Allan Cornish	eliminated stdio functions.
 */

#include <stdio.h>
#include <sys/types.h>
#include <ctype.h>
#include <sys/dir.h>
#include <time.h>
#include <setjmp.h>
#include <sys/stat.h>


#define	MODE	06100			/* set uid, set gid, owner exec */
#define	TRUE	(0 == 0)
#define FALSE	(0 != 0)
#define	STDIN	0			/* standard input file descriptor */
#define	STDOUT	1			/* standard output file descriptor */
#define	STDERR	2			/* standard error file descriptor */
#define	RW	2			/* open code for read and write */


struct tm	*now;			/* current time */
char		atdir[]	= "/usr/spool/at";
jmp_buf		cerror;			/* where to go if file is strange */
int		atfd;			/* stream for file atdir */


main()
{
	register struct direct *dp;
	register int n;
	time_t		ct;
	time_t		time();
	static char dbuf[BUFSIZ];

	if (chdir(atdir) < 0)
		exit(1);

	time(&ct);
	now = localtime(&ct);
	if ((atfd = open(".", 0)) < 0)
		exit(1);

	while ((n = read( atfd, dbuf, sizeof dbuf)) > 0) {

		dp = (struct direct *) &dbuf[n];

		while (--dp >= dbuf)
			if (dp->d_ino)
				check(dp);
	}
	return (0);
}


/*
 * Check checks to see if the file with directory entry `dent'
 * should be executed.
 */
check(dent)
register struct direct	*dent;
{
	struct tm	dot;

	if (dent->d_ino == 0 || setjmp(cerror))
		return;
	gettime(&dot, dent->d_name);
	if (cmptime(&dot, now) <= 0)
		docmd(dent->d_name);
}


/*
 * Gettime fills in certain fields in the struct tm `time' according to
 * the file name `name'.  The file name is assumed to be of the form
 * yymmddhhmm.xx where
 *	yy is the last two digits of the year
 *	mm is the month number (1 - 12)
 *	dd is the day of the month (1 - 31)
 *	hh is the hour
 *	mm is the minute
 * Only the corresponding fields of `time' are filled in.
 */
gettime(time, name)
register struct tm	*time;
register char		*name;
{
	time->tm_year = cnum2(name);
	name += 2;

	/*
	 * This kludge will fail in 2070.
	 */
	if (time->tm_year < 70)
		time->tm_year += 100;

	time->tm_mon = cnum2(name) - 1;
	name += 2;
	time->tm_mday = cnum2(name);
	name += 2;
	time->tm_hour = cnum2(name);
	name += 2;
	time->tm_min = cnum2(name);
	if (name[2] != '.')
		longjmp(cerror, TRUE);
}


/*
 * Cnum2 converts the first two characters of the string `str'
 * to a number.
 */
cnum2(str)
register char	*str;
{
	register int	res,
			i;

	for (res=0, i=2; --i >= 0; ++str) {
		if (!isascii(*str) || !isdigit(*str))
			longjmp(cerror, TRUE);
		res *= 10;
		res += *str - '0';
	}
	return (res);
}


/*
 * Cmptime compares the two struct tm's `a' and `b'.  It returns
 * a number which compares to zero in the same way that `a' compares
 * to `b'.
 */
cmptime(a, b)
register struct tm	*a,
			*b;
{
	register int	res;

	if ((res = a->tm_year - b->tm_year) != 0)
		return (res);
	if ((res = a->tm_mon - b->tm_mon) != 0)
		return (res);
	if ((res = a->tm_mday - b->tm_mday) != 0)
		return (res);
	if ((res = a->tm_hour - b->tm_hour) != 0)
		return (res);
	return (a->tm_min - b->tm_min);
}


/*
 * Docmd executes the shell command file `file'.  This entails
 * forking of a child which changes its user and group id and
 * then calls the shell.
 */
docmd(char* file)
{
	register int	fd;
	char		cbuf[PATH_MAX + 2];
	struct stat	sbuf;

	if (fork() != 0)
		return;
	close(atfd);
	strncpy(cbuf, file, PATH_MAX+1);
	if (stat(cbuf, &sbuf) != 0)
		exit(1);
	if ((MODE & ~sbuf.st_mode) != 0)
		exit(1);
	if (setgid(sbuf.st_gid) != 0)
		exit(1);
	if (setuid(sbuf.st_uid) != 0)
		exit(1);
	if ((fd = open("/dev/null", RW)) == EOF)
		exit(1);
	dup2(fd, STDIN);
	dup2(fd, STDOUT);
	dup2(fd, STDERR);
	close(fd);
	strcat(cbuf, "&");
	execle("/bin/sh", "sh", "-c", cbuf, NULL, NULL);
	exit(1);
}
