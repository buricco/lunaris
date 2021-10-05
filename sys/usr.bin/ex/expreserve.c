/*
 * Adapted from elvis 1.8p4, copyright 1990-1994 by Steve Kirkendall.
 * Copyright 2020 S. V. Nickolas.  All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal with the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 *   1. Redistributions of source code must retain the above copyright notice,
 *      this list of conditions and the following disclaimers.
 *   2. Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimers in the 
 *      documentation and/or other materials provided with the distribution.
 *   3. Neither the names of the authors nor the names of contributors may be
 *      used to endorse or promote products derived from this Software without
 *      specific prior written permission.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE CONTRIBUTORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS WITH THE SOFTWARE.
 */

/* elvprsv.c */

/* This file contains the portable sources for the "elvprsv" program.
 * "Elvprsv" is run by Elvis when Elvis is about to die.  It is also
 * run when the computer boots up.  It is not intended to be run directly
 * by the user, ever.
 *
 * Basically, this program does the following four things:
 *    - It extracts the text from the temporary file, and places the text in
 *	a file in the /usr/preserve directory.
 *    - It adds a line to the /usr/preserve/Index file, describing the file
 *	that it just preserved.
 *    - It removes the temporary file.
 *    -	It sends mail to the owner of the file, saying that the file was
 *	preserved, and how it can be recovered.
 *
 * The /usr/preserve/Index file is a log file that contains one line for each
 * file that has ever been preserved.  Each line of this file describes one
 * preserved file.  The first word on the line is the name of the file that
 * contains the preserved text.  The second word is the full pathname of the
 * file that was being edited; for anonymous buffers, this is the directory
 * name plus "/foo".
 *
 * If elvprsv's first argument (after the command name) starts with a hyphen,
 * then the characters after the hyphen are used as a description of when
 * the editor went away.  This is optional.
 *
 * The remaining arguments are all the names of temporary files that are
 * to be preserved.  For example, on a UNIX system, the /etc/rc file might
 * invoke it this way:
 *
 *	elvprsv "-the system went down" /tmp/elv_*.*
 *
 * This file contains only the portable parts of the preserve program.
 * It must #include a system-specific file.  The system-specific file is
 * expected to define the following functions:
 *
 *	char *ownername(char *filename)	- returns name of person who owns file
 *
 *	void mail(char *user, char *name, char *when, char *tmp)
 *					- tell user that file was preserved
 */

#include <sys/stat.h>
#include <pwd.h>
#include <stdio.h>
#include <time.h>
#include "config.h"
#include "ex_common.h"
#include "ex_ctype.h"

void preserve (char *, char *);


/* This variable is used to add extra error messages for mail sent to root */
char *ps;

/* This function returns the login name of the owner of a file */
char *ownername(char *filename)
{
	struct stat	st;
	struct passwd	*pw;

	/* stat the file, to get its uid */
	if (stat(filename, &st) < 0)
	{
		ps = "stat() failed";
		return "root";
	}

	/* get the /etc/passwd entry for that user */
	pw = getpwuid(st.st_uid);
	if (!pw)
	{
		ps = "uid not found in password file";
		return "root";
	}

	/* return the user's name */
	return pw->pw_name;
}

/* This function sends a mail message to a given user, saying that a file
 * has been preserved.
 */
void mail(char *user, char *file, char *when, char *tmp)
{
	char	cmd[80];/* buffer used for constructing a "mail" command */
	FILE	*m;	/* stream used for giving text to the "mail" program */
	char	*base;	/* basename of the file */
	
	time_t   t;

	/* separate the directory name from the basename. */
	for (base = file + strlen(file); --base > file && *base != SLASH; )
	{
	}
	if (*base == SLASH)
	{
		*base++ = '\0';
	}

	/* for anonymous buffers, pretend the name was "foo" */
	if (!strcmp(base, "*"))
	{
		base = "foo";
	}

	/* open a pipe to the "mail" program */
	sprintf(cmd, "%s -s %s", MAILER, user);
	switch (fork())
	{
	  case -1: /* error */
		return;

	  case 0: /* child */
		/* surrender any special privileges */
		setuid(getuid());
		break; /* continue with the rest of this function */

	  default: /* parent */
		wait(NULL);
		return;
	}
	m = popen(cmd, "w");
	if (!m)
	{
		perror(cmd);
		/* Can't send mail!  Hope the user figures it out. */
		return;
	}

	/* Tell the user that the file was preserved */
    t=time(0);
    fprintf(m, "On %s, user %s ", ctime(&t), user);
    fprintf(m, "was editing the file %s%c%s ", file, SLASH, base);
    fprintf(m, "when ex failed.\n");
    fprintf(m, "You may be able to recover most or all changes"
               "by entering the following:\n");
	fprintf(m, "\n");
	fprintf(m, "     cd %s\n", file);
	if (tmp)
	{
		fprintf(m, "     expreserve %s\n", tmp);
	}
	else
	{
		fprintf(m, "     exrecover %s\n", base);
	}
	if (ps)
	{
		fprintf(m, "\nP.S. %s\n", ps);
		ps = (char *)0;
	}

	/* close the stream */
	pclose(m);
	exit(0);
}

BLK	buf;
BLK	hdr;
BLK	name;
int	rewrite_now;	/* boolean: should we send text directly to orig file? */

/* This function preserves a single file, and announces its success/failure
 * via an e-mail message.
 */
void preserve(char *tname, char *when)
{
	int	infd;		/* fd used for reading from the temp file */
	FILE	*outfp;		/* fp used for writing to the recovery file */
	FILE	*index;		/* fp used for appending to index file */
	char	outname[100];	/* the name of the recovery file */
	char	*user;		/* name of the owner of the temp file */
	int	i;

	/* open the temp file */
	infd = open(tname, O_RDONLY|O_BINARY);
	if (infd < 0)
	{
		/* if we can't open the file, then we should assume that
		 * the filename contains wildcard characters that weren't
		 * expanded... and also assume that they weren't expanded
		 * because there are no files that need to be preserved.
		 * THEREFORE... we should silently ignore it.
		 * (Or loudly ignore it if the user was using -R)
		 */
		if (rewrite_now)
		{
			perror(tname);
		}
		return;
	}

	/* read the header and name from the file */
	if (read(infd, hdr.c, BLKSIZE) != BLKSIZE
	 || read(infd, name.c, BLKSIZE) != BLKSIZE)
	{
		/* something wrong with the file - sorry */
		fprintf(stderr, "%s: truncated header blocks\n", tname);
		close(infd);
		return;
	}

	/* If the filename block contains an empty string, then Elvis was
	 * only keeping the temp file around because it contained some text
	 * that was needed for a named cut buffer.  The user doesn't care
	 * about that kind of temp file, so we should silently delete it.
	 */
	if (name.c[0] == '\0' && name.c[1] == '\177')
	{
		close(infd);
		unlink(tname);
		return;
	}

	/* If there are no text blocks in the file, then we must've never
	 * really started editing.  Discard the file.
	 */
	if (hdr.n[1] == 0)
	{
		close(infd);
		unlink(tname);
		return;
	}

	if (rewrite_now)
	{
		/* we don't need to open the index file */
		index = (FILE *)0;

		/* make sure we can read every block! */
		for (i = 1; i < MAXBLKS && hdr.n[i]; i++)
		{
			lseek(infd, (long)hdr.n[i] * (long)BLKSIZE, 0);
			if (read(infd, buf.c, BLKSIZE) != BLKSIZE
			 || buf.c[0] == '\0')
			{
				/* messed up header */
				fprintf(stderr, "%s: unrecoverable -- header trashed\n", name.c);
				close(infd);
				return;
			}
		}

		/* open the user's file for writing */
		outfp = fopen(name.c, "w");
		if (!outfp)
		{
			perror(name.c);
			close(infd);
			return;
		}
	}
	else
	{
		/* open/create the index file */
		index = fopen(PRSVINDEX, "a");
		if (!index)
		{
			perror(PRSVINDEX);
			mail(ownername(tname), name.c, when, tname);
			exit(2);
		}

		/* should be at the end of the file already, but MAKE SURE */
		fseek(index, 0L, 2);

		/* create the recovery file in the PRESVDIR directory */
		sprintf(outname, "%s%cp%ld", PRSVDIR, SLASH, ftell(index));
		outfp = fopen(outname, "w");
		if (!outfp)
		{
			perror(outname);
			close(infd);
			fclose(index);
			mail(ownername(tname), name.c, when, tname);
			return;
		}
	}

	/* write the text of the file out to the recovery file */
	for (i = 1; i < MAXBLKS && hdr.n[i]; i++)
	{
		lseek(infd, (long)hdr.n[i] * (long)BLKSIZE, 0);
		if (read(infd, buf.c, BLKSIZE) != BLKSIZE
		 || buf.c[0] == '\0')
		{
			/* messed up header */
			fprintf(stderr, "%s: unrecoverable -- header trashed\n", name.c);
			fclose(outfp);
			close(infd);
			if (index)
			{
				fclose(index);
			}
			unlink(outname);
			return;
		}
		fputs(buf.c, outfp);
	}

	/* add a line to the index file */
	if (index)
	{
		fprintf(index, "%s %s\n", outname, name.c);
	}

	/* close everything */
	close(infd);
	fclose(outfp);
	if (index)
	{
		fclose(index);
	}

	/* Are we doing this due to something more frightening than just
	 * a ":preserve" command?
	 */
	if (*when)
	{
		/* send a mail message */
		mail(ownername(tname), name.c, when, (char *)0);

		/* remove the temp file -- the editor has died already */
		unlink(tname);
	}
}

int main(int argc, char **argv)
{
	int	i;
	char	*when = "the editor went away";

	/* do we have a "-c", "-R", or "-when elvis died" argument? */
	i = 1;
	if (argc >= i + 1 && !strcmp(argv[i], "-R"))
	{
		rewrite_now = 1;
		when = "";
		i++;
		setuid(geteuid());
	}
	if (argc >= i + 1 && argv[i][0] == '-')
	{
		when = argv[i] + 1;
		i++;
	}

	/* preserve everything we're supposed to */
	while (i < argc)
	{
		char *p = argv[i] - 1;

		/* reconvert any converted spaces */
		while (*++p)
		{
			if ((unsigned char)(*p) == SPACEHOLDER)
			{
				*p = ' ';
			}
		}
		preserve(argv[i], when);
		i++;
	}
	
	return 0;
}
