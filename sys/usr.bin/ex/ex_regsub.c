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

/* regsub.c */

/* This file contains the regsub() function, which performs substitutions
 * after a regexp match has been found.
 */

#include "config.h"
#include "ex_ctype.h"
#include "ex_common.h"
#include "ex_regexp.h"

/* perform substitutions after a regexp match */
void regsub(regexp *re, REG char *src, REG char *dst)
{
	REG char	*cpy;	/* pointer to start of text to copy */
	REG char	*end;	/* pointer to end of text to copy */
	REG char	c;
	char		*start;
#ifndef CRUNCH
	int		mod = 0;/* used to track \U, \L, \u, \l, and \E */
	int		len;	/* used to calculate length of subst string */
	static char	*prev;	/* a copy of the text from the previous subst */

	/* replace \~ (or maybe ~) by previous substitution text */

	/* step 1: calculate the length of the new substitution text */
	for (len = strlen(src), c = '\0', cpy = src; *cpy; cpy++)
	{
# ifdef NO_MAGIC
		if (c == '\\' && *cpy == '~')
# else
		if (c == (*o_magic ? '\0' : '\\') && *cpy == '~')
# endif
		{
			if (!prev)
			{
				regerror("No prev text to substitute for ~");
				return;
			}
			len += strlen(prev) - 1;
# ifndef NO_MAGIC
			if (!*o_magic)
# endif
				len -= 1; /* because we lose the \ too */
		}

		/* watch backslash quoting */
		if (c != '\\' && *cpy == '\\')
			c = '\\';
		else
			c = '\0';
	}

	/* allocate memory for the ~ed version of src */
	checkmem();
	start = cpy = (char *)malloc((unsigned)(len + 1));
	if (!cpy)
	{
		regerror("Not enough memory for ~ expansion");
		return;
	}

	/* copy src into start, replacing the ~s by the previous text */
	while (*src)
	{
# ifndef NO_MAGIC
		if (*o_magic && *src == '~')
		{
			strcpy(cpy, prev);
			cpy += strlen(prev);
			src++;
		}
		else if (!*o_magic && *src == '\\' && *(src + 1) == '~')
# else /* NO_MAGIC */
		if (*src == '\\' && *(src + 1) == '~')
# endif /* NO_MAGIC */
		{
			strcpy(cpy, prev);
			cpy += strlen(prev);
			src += 2;
		}
		else
		{
			if (*src == '\\')
			{
				*cpy++ = *src++;
			}
			*cpy++ = *src++;
		}
	}
	*cpy = '\0';
#ifdef DEBUG
	if ((int)(cpy - start) != len)
	{
		msg("Bug in regsub.c! Predicted length = %d, Actual length = %d", len, (int)(cpy - start));
	}
#endif
	checkmem();

	/* remember this as the "previous" for next time */
	if (prev)
		free(prev);
	prev = src = start;

#endif /* undef CRUNCH */

	start = src;
	while ((c = *src++) != '\0')
	{
#ifndef NO_MAGIC
		/* recognize any meta characters */
		if (c == '&' && *o_magic)
		{
			cpy = re->startp[0];
			end = re->endp[0];
		}
		else
#endif /* not NO_MAGIC */
		if (c == '\\')
		{
			c = *src++;
			switch (c)
			{
#ifndef NO_MAGIC
			  case '0':
			  case '1':
			  case '2':
			  case '3':
			  case '4':
			  case '5':
			  case '6':
			  case '7':
			  case '8':
			  case '9':
				/* \0 thru \9 mean "copy subexpression" */
				c -= '0';
				cpy = re->startp[c];
				end = re->endp[c];
				break;
# ifndef CRUNCH
			  case 'U':
			  case 'u':
			  case 'L':
			  case 'l':
				/* \U and \L mean "convert to upper/lowercase" */
				mod = c;
				continue;

			  case 'E':
			  case 'e':
				/* \E ends the \U or \L */
				mod = 0;
				continue;
# endif /* not CRUNCH */
			  case '&':
				/* "\&" means "original text" */
				if (*o_magic)
				{
					*dst++ = c;
					continue;
				}
				cpy = re->startp[0];
				end = re->endp[0];
				break;

#else /* NO_MAGIC */
			  case '&':
				/* "\&" means "original text" */
				cpy = re->startp[0];
				end = re->endp[0];
				break;
#endif /* NO_MAGIC */
			  default:
				/* ordinary char preceded by backslash */
				*dst++ = c;
				continue;
			}
		}
#ifndef CRUNCH
		else if (c == '\r')
		{
			/* transliterate ^M into newline */
			*dst++ = '\n';
			continue;
		}
#endif /* !CRUNCH */
		else
		{
			/* ordinary character, so just copy it */
			*dst++ = c;
			continue;
		}

		/* Note: to reach this point in the code, we must have evaded
		 * all "continue" statements.  To do that, we must have hit
		 * a metacharacter that involves copying.
		 */

		/* if there is nothing to copy, loop */
		if (!cpy)
			continue;

		/* copy over a portion of the original */
		while (cpy < end)
		{
#ifndef NO_MAGIC
# ifndef CRUNCH
			switch (mod)
			{
			  case 'U':
			  case 'u':
				/* convert to uppercase */
				*dst++ = toupper(*cpy++);
				break;

			  case 'L':
			  case 'l':
				/* convert to lowercase */
				*dst++ = tolower(*cpy++);
				break;

			  default:
				/* copy without any conversion */
				*dst++ = *cpy++;
			}

			/* \u and \l end automatically after the first char */
			if (mod && (mod == 'u' || mod == 'l'))
			{
				mod = 0;
			}
# else /* CRUNCH */
			*dst++ = *cpy++;
# endif /* CRUNCH */
#else /* NO_MAGIC */
			*dst++ = *cpy++;
#endif /* NO_MAGIC */
		}
	}
	*dst = '\0';
}
