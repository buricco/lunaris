/*	$OpenBSD: unexpand.c,v 1.13 2016/10/11 16:22:15 millert Exp $	*/
/*	$NetBSD: unexpand.c,v 1.5 1994/12/24 17:08:05 cgd Exp $	*/

/*-
 * Copyright (c) 1980, 1993
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
 * unexpand - put tabs into a file replacing blanks
 */
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

char	genbuf[BUFSIZ];
char	linebuf[BUFSIZ];

void tabify(bool);

/* uso */
size_t
strlcpy(char *d, const char *s, size_t n)
{
	size_t t;

	for (t=0; t<n; t++) {
		d[t]=s[t];
		if (!s[t])
			return t;
	}
	d[n-1]=0;
	return n;
}

int
main(int argc, char *argv[])
{
	bool all = false;
	char *cp;

	argc--, argv++;
	if (argc > 0 && argv[0][0] == '-') {
		if (strcmp(argv[0], "-a") != 0) {
			fprintf(stderr, "usage: unexpand [-a] [file ...]\n");
			exit(1);
		}
		all = true;
		argc--, argv++;
	}
	do {
		if (argc > 0) {
			if (freopen(argv[0], "r", stdin) == NULL) {
				perror(argv[0]);
				exit(1);
			}
			argc--, argv++;
		}
		while (fgets(genbuf, BUFSIZ, stdin) != NULL) {
			for (cp = linebuf; *cp; cp++)
				continue;
			if (cp > linebuf)
				cp[-1] = 0;
			tabify(all);
			printf("%s", linebuf);
		}
	} while (argc > 0);
	exit(0);
}

void
tabify(bool all)
{
	char *cp, *dp;
	int dcol;
	int ocol;
	size_t len;

	ocol = 0;
	dcol = 0;
	cp = genbuf;
	dp = linebuf;
	len = sizeof linebuf;

	for (;;) {
		switch (*cp) {

		case ' ':
			dcol++;
			break;

		case '\t':
			dcol += 8;
			dcol &= ~07;
			break;

		default:
			while (((ocol + 8) &~ 07) <= dcol) {
				if (ocol + 1 == dcol)
					break;
				if (len > 1) {
					*dp++ = '\t';
					len--;
				}
				ocol += 8;
				ocol &= ~07;
			}
			while (ocol < dcol) {
				if (len > 1) {
					*dp++ = ' ';
					len--;
				}
				ocol++;
			}
			if (*cp == '\0' || !all) {
				strlcpy(dp, cp, len);
				return;
			}
			*dp++ = *cp;
			len--;
			ocol++;
			dcol++;
		}
		cp++;
	}
}
