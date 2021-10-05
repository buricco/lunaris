/* $Id: wcslib.c 125 2004-11-02 01:31:58Z bob $ */

/*
 * Copyright (c) 1987, 1993
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

/*
 * This file contains some wcs* functions that are based on their
 * corresponding str* functions from NetBSD's libc. There are also some
 * other wide-char functions that are of my own creation.
 */

#include <stdio.h>
#include <wchar.h>
#include <wctype.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>
#include <time.h>
#include <iconv.h>
#include <errno.h>
#include <locale.h>
#include "calendar.h"

int
wcscasecmp(s1, s2)
	const wchar_t *s1, *s2;
{
	const wchar_t *us1 = (const wchar_t *)s1,
			*us2 = (const wchar_t *)s2;

	while (towlower(*us1) == towlower(*us2++))
		if (*us1++ == '\0')
			return (0);
	return (towlower(*us1) - towlower(*--us2));
}

int
wcsncasecmp(s1, s2, n)
	const wchar_t *s1, *s2;
	size_t n;
{
	if (n != 0) {
		const wchar_t *us1 = (const wchar_t *)s1,
				*us2 = (const wchar_t *)s2;

		do {
			if (towlower(*us1) != towlower(*us2++))
				return (towlower(*us1) - towlower(*--us2));
			if (*us1++ == '\0')
				break;
		} while (--n != 0);
	}
	return (0);
}

wchar_t *
wcsdup(str)
	const wchar_t *str;
{
	size_t len;
	wchar_t *copy;

	if (str == NULL)
		return NULL;

	len = wcslen(str) + 1;
	if (!(copy = malloc(len * sizeof (wchar_t))))
		return (NULL);
	wmemcpy(copy, str, len);
	return (copy);
}

static size_t
utf8towcs (wchar_t *out, char *in, size_t n)
{
	char *ip = in, *op = (char *) out;
	size_t ibl = strlen(in), obl = n * sizeof(wchar_t);
	static iconv_t cd = (iconv_t) -1;
	int r;

	if (cd == (iconv_t) -1)
		if ((cd = iconv_open("WCHAR_T", "UTF-8")) == (iconv_t) -1) 
			err(1, NULL);

	r = iconv(cd, &ip, &ibl, &op, &obl);
	if ((r == -1) && (errno != EINVAL) && (errno != E2BIG))
		return -1;

	if (obl >= sizeof(wchar_t))
		*((wchar_t *) op) = L'\0';
	else
		*(((wchar_t *) op) - 1) = L'\0';
	return r;
}

wchar_t *
myfgetws(wchar_t *s, int size, FILE *stream)
{
	static int utf8mode = 0;
	static char *filename = NULL;
	static int line = 0;

	char buf[2048 + 1], *p;
	int ch, r;

	if (fgets(buf, sizeof(buf), stream) == NULL)
		return NULL;

	/* Keep a count of line numbers for printing errors. */
	line++;

	if ((p = strchr(buf, '\n')) != NULL)
		*p = '\0';
	else
		while ((ch = getchar()) != '\n' && ch != EOF);

	/* handle special directives */
	if (strncmp(buf, "# ", 2) == 0) {
		char *newfile, *end;
		int n;

		line = strtol(buf + 2, &end, 10) - 1;

		/* Skip whitespace and the opening double quote. */
		while (isspace(*end))
			end++;
		end++;

		/* Copy the filename to a buffer for printing errors. */
		for (n = 0, p = end; *p && *p != '"'; p++)
			n++;
		if ((newfile = (char *) malloc(n + 1)) == NULL)
			err(1, NULL);
		strncpy(newfile, end, n);
		newfile[n] = 0;

		/* If this is a new file, clear file-specific state. */
		if (!filename || strcmp(newfile, filename) != 0) {
			int i;

			utf8mode = 0;
			for (i = 0; i < NUMEV; i++)
				if (spev[i].uname) {
					free(spev[i].uname);
					spev[i].uname = NULL;
				}
			(void) setlocale(LC_ALL, "");
		}
		if (filename)
			free(filename);
		filename = newfile;
	} else if (strncmp(buf, "LANG=utf-8", 10) == 0) {
		utf8mode = 1;
		(void) setlocale(LC_ALL, "C");
	} else if (strncmp(buf, "LANG=", 5) == 0) {
		utf8mode = 0;
		if (setlocale(LC_ALL, buf + 5) == NULL)
			errx(1, "switch to locale ``%ls'' cannot be honored", buf + 5);
	}

	/* convert the line */
	if (utf8mode)
		r = utf8towcs(s, buf, size);
	else
		r = mbstowcs(s, buf, size);

	if (r == -1) {
		if (!filename || strcmp(filename, "<stdin>") == 0)
			err(1, "%s:%d", calendarPath, line);
		else
			err(1, "%s:%d", filename, line);
	}

	return s;
}
