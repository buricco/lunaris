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

/* elvrec.c */

/* This file contains the file recovery program */

#include <stdio.h>
#include "config.h"
#include "ex_common.h"

void recover(char *basename, char *outname)
{
	char	pathname[500];	/* full pathname of the file to recover */
	char	line[600];	/* a line from the /usr/preserve/Index file */
	int	ch;		/* a character from the text being recovered */
	FILE	*from;		/* the /usr/preserve file, or /usr/preserve/Index */
	FILE	*to;		/* the user's text file */
	char	*ptr;

	/* convert basename to a full pathname */
	if (basename)
	{
#ifndef CRUNCH
		if (basename[0] != SLASH)
		{
			ptr = getcwd(pathname, sizeof pathname);
			if (ptr != pathname)
			{
				strcpy(pathname, ptr);
			}
			ptr = pathname + strlen(pathname);
			*ptr++ = SLASH;
			strcpy(ptr, basename);
		}
		else
#endif
		{
			strcpy(pathname, basename);
		}
	}

	/* scan the /usr/preserve/Index file, for the *oldest* unrecovered
	 * version of this file.
	 */
	from = fopen(PRSVINDEX, "r");
	while (from && fgets(line, sizeof line, from))
	{
		/* strip off the newline from the end of the string */
		line[strlen(line) - 1] = '\0';

		/* parse the line into a "preserve" name and a "text" name */
		for (ptr = line; *ptr != ' '; ptr++)
		{
		}
		*ptr++ = '\0';

		/* If the "preserve" file is missing, then ignore this line
		 * because it describes a file that has already been recovered.
		 */
		if (access(line, 0) < 0)
		{
			continue;
		}

		/* are we looking for a specific file? */
		if (basename)
		{
			/* quit if we found it */
			if (!strcmp(ptr, pathname))
			{
				break;
			}
		}
		else
		{
			/* list this file as "available for recovery" */
			puts(ptr);
		}
	}

	/* file not found? */
	if (!basename || !from || feof(from))
	{
		if (from != NULL) fclose(from);
		if (basename)
		{
			fprintf(stderr, "%s: no recovered file has that exact name\n", pathname);
		}
		return;
	}
	if (from != NULL) fclose(from);

	/* copy the recovered text back into the user's file... */

	/* open the /usr/preserve file for reading */
	from = fopen(line, "r");
	if (!from)
	{
		perror(line);
		exit(2);
	}

	/* Be careful about user-id.  We want to be running under the user's
	 * real id when we open/create the user's text file... but we want
	 * to be superuser when we delete the /usr/preserve file.  For UNIX,
	 * we accomplish this by deleting the /usr/preserve file *now*,
	 * when it is open but before we've read it.  Then we revert to the
	 * user's real id.  (For safety's sake, we first check for write
	 * permission on the user's file before deleting the preserved file.)
	 */
	if (access(outname, 2) < 0)
	{
		perror(outname);
		return;
	}
	unlink(line);
	setuid(getuid());

	if (outname == NULL) return;

	/* open the user's file for writing */
	to = fopen(outname, "w");
	if (!to)
	{
		perror(outname);
		exit(2);
	}

	/* copy the text */
	while ((ch = getc(from)) != EOF)
	{
		putc(ch, to);
	}

}

int 
main(int argc, char **argv)
{
	/* check arguments */
	if (argc > 3)
	{
		fprintf(stderr, "usage: %s [preserved_file [recovery_file]]\n", argv[0]);
		exit(2);
	}

	/* recover the requested file, or list recoverable files */
	if (argc == 3)
	{
		/* recover the file, but write it to a different filename */
		recover (argv[1], argv[2]);
	}
	else if (argc == 2)
	{
		/* recover the file */
		recover(argv[1], argv[1]);
	}
	else
	{
		/* list the recoverable files */
		recover((char *)0, (char *)0);
	}

	/* success! */
	return 0;
}
