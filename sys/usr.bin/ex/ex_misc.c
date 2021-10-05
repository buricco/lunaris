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

/* misc.c */

/* This file contains functions which didn't seem happy anywhere else */

#include "config.h"
#include "ex_common.h"

/* find a particular line & return a pointer to a copy of its text */
char *fetchline(long line)
{
	int		i;
	REG char	*scan;	/* used to search for the line in a BLK */
	long		l;	/* line number counter */
	static BLK	buf;	/* holds ONLY the selected line (as string) */
	REG char	*cpy;	/* used while copying the line */
	static long	nextline;	/* }  These four variables are used */
	static long	chglevel;	/*  } to implement a shortcut when  */
	static char	*nextscan;	/*  } consecutive lines are fetched */
	static long	nextlnum;	/* }                                */

	/* can we do a shortcut? */
	if (changes == chglevel && line == nextline)
	{
		scan = nextscan;
	}
	else
	{
		/* scan lnum[] to determine which block its in */
		for (i = 1; line > lnum[i]; i++)
		{
		}
		nextlnum = lnum[i];

		/* fetch text of the block containing that line */
		scan = blkget(i)->c;

		/* find the line in the block */
		for (l = lnum[i - 1]; ++l < line; )
		{
			while (*scan++ != '\n')
			{
			}
		}
	}

	/* copy it into a block by itself, with no newline */
	for (cpy = buf.c; *scan != '\n'; )
	{
		*cpy++ = *scan++;
	}
	*cpy = '\0';

	/* maybe speed up the next call to fetchline() ? */
	if (line < nextlnum)
	{
		nextline = line + 1;
		chglevel = changes;
		nextscan = scan + 1;
	}
	else
	{
		nextline = 0;
	}

	/* Calls to fetchline() interfere with calls to pfetch().  Make sure
	 * that pfetch() resets itself on its next invocation.
	 */
	pchgs = 0L;

	/* Return a pointer to the line's text */
	return buf.c;
}

/* error message from the regexp code */
void regerror(char *txt)
{
	msg("RE error: %s", txt);
}

/* This function is equivelent to the pfetch() macro */
void	pfetch(long l)
{
	if(l != pline || changes != pchgs)
	{
		pline = (l);
		ptext = fetchline(pline);
		plen = strlen(ptext);
		pchgs = changes;
	}
}
