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

/* move5.c */

/* This file contains the word-oriented movement functions */

#include "config.h"
#include "ex_ctype.h"
#include "ex_common.h"

MARK	m_fword(MARK m, long cnt, int cmd, int prevkey)
{
	REG long	l;
	REG char	*text;
	REG int		i;

	DEFAULT(1);

	l = markline(m);
	pfetch(l);
	text = ptext + markidx(m);

#ifndef CRUNCH
	/* As a special case, "cw" or "cW" on whitespace without a count
	 * treats the single whitespace character under the cursor as a word.
	 */
	if (cnt == 1L && prevkey == 'c' && isspace(*text))
	{
		return m;
	}
#endif

	while (cnt-- > 0) /* yes, ASSIGNMENT! */
	{
		i = *text++;

		if (cmd == 'W')
		{
			/* include any non-whitespace */
			while (i && !isspace(i))
			{
				i = *text++;
			}
		}
		else if (isalnum(i) || i == '_')
		{
			/* include an alphanumeric word */
			while (i && isalnum(i))
			{
				i = *text++;
			}
		}
		else
		{
			/* include contiguous punctuation */
			while (i && !isalnum(i) && !isspace(i))
			{
				i = *text++;
			}
		}

		/* if not part of "cw" or "cW" command... */
		if (prevkey != 'c' || cnt > 0)
		{
			/* include trailing whitespace */
			while (!i || isspace(i))
			{
				/* did we hit the end of this line? */
				if (!i)
				{
					/* "dw" shouldn't delete newline after word */
					if (prevkey && cnt == 0)
					{
						break;
					}

					/* move to next line, if there is one */
					l++;
					if (l > nlines)
					{
						return MARK_EOF;
					}
					pfetch(l);
					text = ptext;
				}

				i = *text++;
			}
		}
		text--;
	}

	/* if argument to operator, then back off 1 char since "w" and "W"
	 * include the last char in the affected text.
	 */
	if (prevkey)
	{
		text--;
	}

	/* construct a MARK for this place */
	m = buildmark(text);
	return m;
}

MARK	m_bword(MARK m, long cnt, int cmd)
{
	REG long	l;
	REG char	*text;

	DEFAULT(1);

	l = markline(m);
	pfetch(l);
	text = ptext + markidx(m);
	while (cnt-- > 0) /* yes, ASSIGNMENT! */
	{
		text--;

		/* include preceding whitespace */
		while (text < ptext || isspace(*text))
		{
			/* did we hit the end of this line? */
			if (text < ptext)
			{
				/* move to preceding line, if there is one */
				l--;
				if (l <= 0)
				{
					return MARK_UNSET;
				}
				pfetch(l);
				text = ptext + plen - 1;
			}
			else
			{
				text--;
			}
		}

		if (cmd == 'B')
		{
			/* include any non-whitespace */
			while (text >= ptext && !isspace(*text))
			{
				text--;
			}
		}
		else if (isalnum(*text) || *text == '_')
		{
			/* include an alphanumeric word */
			while (text >= ptext && isalnum(*text))
			{
				text--;
			}
		}
		else
		{
			/* include contiguous punctuation */
			while (text >= ptext && !isalnum(*text) && !isspace(*text))
			{
				text--;
			}
		}
		text++;
	}

	/* construct a MARK for this place */
	m = buildmark(text);
	return m;
}

MARK	m_eword(MARK m, long cnt, int cmd)
{
	REG long	l;
	REG char	*text;
	REG int		i;

	DEFAULT(1);

	l = markline(m);
	pfetch(l);
	text = ptext + markidx(m);
	while (cnt-- > 0) /* yes, ASSIGNMENT! */
	{
		if (*text)
			text++;
		i = *text++;

		/* include preceding whitespace */
		while (!i || isspace(i))
		{
			/* did we hit the end of this line? */
			if (!i)
			{
				/* move to next line, if there is one */
				l++;
				if (l > nlines)
				{
					return MARK_UNSET;
				}
				pfetch(l);
				text = ptext;
			}

			i = *text++;
		}

		if (cmd == 'E')
		{
			/* include any non-whitespace */
			while (i && !isspace(i))
			{
				i = *text++;
			}
		}
		else if (isalnum(i) || i == '_')
		{
			/* include an alphanumeric word */
			while (i && isalnum(i))
			{
				i = *text++;
			}
		}
		else
		{
			/* include contiguous punctuation */
			while (i && !isalnum(i) && !isspace(i))
			{
				i = *text++;
			}
		}
		text -= 2;
	}

	/* construct a MARK for this place */
	m = buildmark(text);
	return m;
}
