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

/* move2.c */

/* This function contains the movement functions that perform RE searching */

#include "config.h"
#include "ex_common.h"
#include "ex_regexp.h"
#include <string.h>

static regexp	*re;	/* compiled version of the pattern to search for */
static	int	prevsf;	/* boolean: previous search direction was forward? */

#ifndef NO_EXTENSIONS
/*ARGSUSED*/
MARK m_wsrch(word, m, cnt)
	char	*word;	/* the word to search for */
	MARK	m;	/* the starting point */
	int	cnt;	/* ignored */
{
	char	buffer[WSRCH_MAX + 6];

	/* wrap \< and \> around the word */
	strncpy(buffer, "/\\<", WSRCH_MAX + 6);
	strncat(buffer, word, WSRCH_MAX);
	strcat(buffer, "\\>");

	/* show the searched-for word on the bottom line */
	move(LINES - 1, 0);
	qaddstr(buffer);
	clrtoeol();
	refresh();

	/* search for the word */
	return m_fsrch(m, buffer);
}
#endif

MARK	m_nsrch(MARK m, long cnt, int cmd)
{
	int	oldprevsf; /* original value of prevsf, so we can fix any changes */

	DEFAULT(1L);

	/* clear the bottom line.  In particular, we want to loose any
	 * "(wrapped)" notice.
	 */
	move(LINES - 1, 0);
	clrtoeol();

	/* if 'N' command, then invert the "prevsf" variable */
	oldprevsf = prevsf;
	if (cmd == 'N')
	{
		prevsf = !prevsf;
	}

	/* search forward if prevsf -- i.e., if previous search was forward */
	while (--cnt >= 0L && m != MARK_UNSET)
	{
		if (prevsf)
		{
			m = m_fsrch(m, (char *)0);
		}
		else
		{
			m = m_bsrch(m, (char *)0);
		}
	}

	/* restore the old value of prevsf -- if cmd=='N' then it was inverted,
	 * and the m_fsrch() and m_bsrch() functions force it to a (possibly
	 * incorrect) value.  The value of prevsf isn't supposed to be changed
	 * at all here!
	 */
	prevsf = oldprevsf;
	return m;
}

MARK	m_fsrch(MARK m, char *ptrn)
{
	long	l;	/* line# of line to be searched */
	char	*line;	/* text of line to be searched */
	int	wrapped;/* boolean: has our search wrapped yet? */
	int	pos;	/* where we are in the line */
#ifndef CRUNCH
	long	delta = INFINITY;/* line offset, for things like "/foo/+1" */
#endif

	/* remember: "previous search was forward" */
	prevsf = TRUE;

	if (ptrn && *ptrn)
	{
		/* locate the closing '/', if any */
		line = parseptrn(ptrn);
#ifndef CRUNCH
		if (*line)
		{
			delta = atol(line);
		}
#endif
		ptrn++;

		/* free the previous pattern */
		if (re) free(re);

		/* compile the pattern */
		re = regcomp(ptrn);
		if (!re)
		{
			return MARK_UNSET;
		}
	}
	else if (!re)
	{
		msg("No previous expression");
		return MARK_UNSET;
	}

	/* search forward for the pattern */
	pos = markidx(m) + 1;
	pfetch(markline(m));
	if (pos >= plen)
	{
		pos = 0;
		m = (m | (BLKSIZE - 1)) + 1;
	}
	wrapped = FALSE;
	for (l = markline(m); l != markline(m) + 1 || !wrapped; l++)
	{
		/* wrap search */
		if (l > nlines)
		{
			/* if we wrapped once already, then the search failed */
			if (wrapped)
			{
				break;
			}

			/* else maybe we should wrap now? */
			if (*o_wrapscan)
			{
				l = 0;
				wrapped = TRUE;
				continue;
			}
			else
			{
				break;
			}
		}

		/* get this line */
		line = fetchline(l);

		/* check this line */
		if (regexec(re, &line[pos], (pos == 0)))
		{
			/* match! */
			if (wrapped && *o_warn)
				msg("(wrapped)");
#ifndef CRUNCH
			if (delta != INFINITY)
			{
				l += delta;
				if (l < 1 || l > nlines)
				{
					msg("search offset too big");
					return MARK_UNSET;
				}
				force_flags = LNMD|INCL;
				return MARK_AT_LINE(l);
			}
#endif
			if (re->leavep)
				return MARK_AT_LINE(l) + (int)(re->leavep - line);
			else
				return MARK_AT_LINE(l) + (int)(re->startp[0] - line);
		}
		pos = 0;
	}

	/* not found */
#ifdef DEBUG
	msg("/%s/ not found", ptrn);
#else
	msg(*o_wrapscan ? "Not found" : "Hit bottom without finding RE");
#endif
	return MARK_UNSET;
}

MARK	m_bsrch(MARK m, char *ptrn)
{
	long	l;	/* line# of line to be searched */
	char	*line;	/* text of line to be searched */
	int	wrapped;/* boolean: has our search wrapped yet? */
	int	pos;	/* last acceptable idx for a match on this line */
	int	last;	/* remembered idx of the last acceptable match on this line */
	int	try;	/* an idx at which we strat searching for another match */
#ifndef CRUNCH
	long	delta = INFINITY;/* line offset, for things like "/foo/+1" */
#endif

	/* remember: "previous search was not forward" */
	prevsf = FALSE;

	if (ptrn && *ptrn)
	{
		/* locate the closing '?', if any */
		line = parseptrn(ptrn);
#ifndef CRUNCH
		if (*line)
		{
			delta = atol(line);
		}
#endif
		ptrn++;

		/* free the previous pattern, if any */
		if (re) free(re);

		/* compile the pattern */
		re = regcomp(ptrn);
		if (!re)
		{
			return MARK_UNSET;
		}
	}
	else if (!re)
	{
		msg("No previous expression");
		return MARK_UNSET;
	}

	/* search backward for the pattern */
	pos = markidx(m);
	wrapped = FALSE;
	for (l = markline(m); l != markline(m) - 1 || !wrapped; l--)
	{
		/* wrap search */
		if (l < 1)
		{
			if (*o_wrapscan)
			{
				l = nlines + 1;
				wrapped = TRUE;
				continue;
			}
			else
			{
				break;
			}
		}

		/* get this line */
		line = fetchline(l);

		/* check this line */
		if (regexec(re, line, 1) && (int)(re->startp[0] - line) < pos)
		{
			/* match!  now find the last acceptable one in this line */
			do
			{
				if (re->leavep)
					last = (int)(re->leavep - line);
				else
					last = (int)(re->startp[0] - line);
				try = (int)(re->endp[0] - line);
			} while (try > 0
				 && regexec(re, &line[try], FALSE)
				 && (int)(re->startp[0] - line) < pos);

			if (wrapped && *o_warn)
				msg("(wrapped)");
#ifndef CRUNCH
			if (delta != INFINITY)
			{
				l += delta;
				if (l < 1 || l > nlines)
				{
					msg("search offset too big");
					return MARK_UNSET;
				}
				force_flags = LNMD|INCL;
				return MARK_AT_LINE(l);
			}
#endif
			return MARK_AT_LINE(l) + last;
		}
		pos = BLKSIZE;
	}

	/* not found */
#ifdef DEBUG
	msg("?%s? not found", ptrn);
#else
	msg(*o_wrapscan ? "Not found" : "Hit bottom without finding RE");
#endif
	return MARK_UNSET;
}
