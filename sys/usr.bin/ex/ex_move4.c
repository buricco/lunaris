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

/* move4.c */

/* This file contains movement functions which are screen-relative */

#include "config.h"
#include "ex_common.h"

/* This moves the cursor to a particular row on the screen */
/*ARGSUSED*/
MARK m_row(MARK m, long cnt, int key)
{
	DEFAULT(1);

	/* calculate destination line based on key */
	cnt--;
	switch (key)
	{
	  case 'H':
	  case ctrl('_'):
		cnt = topline + cnt;
		break;

	  case 'M':
		cnt = topline + (LINES - 1) / 2;
		break;

	  case 'L':
		cnt = botline - cnt;
		break;
	}

	/* return the mark of the destination line */
	return MARK_AT_LINE(cnt);
}

/* This function repositions the current line to show on a given row */
MARK m_z(MARK m, long cnt, int key)
{
	long	newtop;
	int	i;

	/* Which line are we talking about? */
	if (cnt < 0 || cnt > nlines)
	{
		return MARK_UNSET;
	}
	if (cnt)
	{
		m = MARK_AT_LINE(cnt);
		newtop = cnt;
	}
	else
	{
		newtop = markline(m);
	}

	/* allow a "window size" number to be entered */
	for (i = 0; key >= '0' && key <= '9'; key = getkey(0))
	{
		i = i * 10 + key - '0';
	}
#ifndef CRUNCH
	if (i > 0 && i <= LINES - 1)
	{
		*o_window = i;
	}
#else
	/* the number is ignored if -DCRUNCH */
#endif

	/* figure out which line will have to be at the top of the screen */
	switch (key)
	{
	  case '\n':
	  case '\r':
	  case '+':
		break;

	  case '.':
	  case 'z':
		newtop -= LINES / 2;
		break;

	  case '-':
		newtop -= LINES - 1;
		break;

	  default:
		return MARK_UNSET;
	}

	/* make the new topline take effect */
	redraw(MARK_UNSET, FALSE);
	if (newtop >= 1)
	{
		topline = newtop;
	}
	else
	{
		topline = 1L;
	}
	redrawrange(0L, INFINITY, INFINITY);

	/* The cursor doesn't move */
	return m;
}

/* This function scrolls the screen.  It does this by calling redraw() with
 * an off-screen line as the argument.  It will move the cursor if necessary
 * so that the cursor is on the new screen.
 */
/*ARGSUSED*/
MARK m_scroll(MARK m, long cnt, int key)
{
	MARK	tmp;	/* a temporary mark, used as arg to redraw() */
#ifndef CRUNCH
	int	savenearscroll;

	savenearscroll = *o_nearscroll;
	*o_nearscroll = LINES;
#endif

	/* adjust cnt, and maybe *o_scroll, depending on key */
	switch (key)
	{
	  case ctrl('F'):
	  case ctrl('B'):
		DEFAULT(1);
		redrawrange(0L, INFINITY, INFINITY); /* force complete redraw */
		cnt = cnt * (LINES - 1) - 2; /* keeps two old lines on screen */
		break;

	  case ctrl('E'):
	  case ctrl('Y'):
		DEFAULT(1);
		break;

	  case ctrl('U'):
	  case ctrl('D'):
		if (cnt == 0) /* default */
		{
			cnt = *o_scroll;
		}
		else
		{
			if (cnt > LINES - 1)
			{
				cnt = LINES - 1;
			}
			*o_scroll = cnt;
		}
		break;
	}

	/* scroll up or down, depending on key */
	switch (key)
	{
	  case ctrl('B'):
	  case ctrl('Y'):
	  case ctrl('U'):
		cnt = topline - cnt;
		if (cnt < 1L)
		{
			cnt = 1L;
			m = MARK_FIRST;
		}
#ifndef CRUNCH
		if (key == ctrl('Y') && !has_AL && !has_SR)
		{
			topline = cnt;
			redrawrange(0L, INFINITY, INFINITY);
		}
#endif
		tmp = MARK_AT_LINE(cnt) + markidx(m);
		redraw(tmp, FALSE);
		if (markline(m) > botline)
		{
			m = MARK_AT_LINE(botline);
		}
		break;

	  case ctrl('F'):
	  case ctrl('E'):
	  case ctrl('D'):
		cnt = botline + cnt;
		if (cnt > nlines)
		{
			cnt = nlines;
			m = MARK_LAST;
		}
		tmp = MARK_AT_LINE(cnt) + markidx(m);
		redraw(tmp, FALSE);
		if (markline(m) < topline)
		{
			m = MARK_AT_LINE(topline);
		}
		break;
	}

#ifndef CRUNCH
	*o_nearscroll = savenearscroll;
#endif
	return m;
}
