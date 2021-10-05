/*	$NetBSD: vars.c,v 1.12 2003/10/31 01:25:54 ross Exp $	*/

/*
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

#include <sys/cdefs.h>
#ifndef lint
#if 0
static char sccsid[] = "@(#)vars.c	8.1 (Berkeley) 6/6/93";
#else
//__RCSID("$NetBSD: vars.c,v 1.12 2003/10/31 01:25:54 ross Exp $");
#endif
#endif /* not lint */

#include "rcv.h"
#include "extern.h"

/*
 * Mail -- a mail program
 *
 * Variable handling stuff.
 */

/*
 * Assign a value to a variable.
 */
void
assign(char name[], char values[])
{
	struct var *vp;
	int h;

	h = hash(name);
	vp = lookup(name);
	if (vp == NULL) {
		vp = (struct var *) calloc(1, sizeof *vp);
		if (vp == NULL)
			errx(1, "Out of memory");
		vp->v_name = vcopy(name);
		vp->v_link = variables[h];
		variables[h] = vp;
	}
	else
                v_free(vp->v_value);
	vp->v_value = vcopy(values);
}

/*
 * Free up a variable string.  We do not bother to allocate
 * strings whose value is "" since they are expected to be frequent.
 * Thus, we cannot free same!
 */
void
v_free(char *cp)
{
	if (*cp)
		free(cp);
}

/*
 * Copy a variable value into permanent (ie, not collected after each
 * command) space.  Do not bother to alloc space for ""
 */

char *
vcopy(char str[])
{
	char *new;
	unsigned len;

	if (*str == '\0')
		return "";
	len = strlen(str) + 1;
	if ((new = malloc(len)) == NULL)
		errx(1, "Out of memory");
	memmove(new, str, (int) len);
	return new;
}

/*
 * Get the value of a variable and return it.
 * Look in the environment if its not available locally.
 */

char *
value(char name[])
{
	struct var *vp;

	if ((vp = lookup(name)) == NULL)
		return(getenv(name));
	return(vp->v_value);
}

/*
 * Locate a variable and return its variable
 * node.
 */

struct var *
lookup(char name[])
{
	struct var *vp;

	for (vp = variables[hash(name)]; vp != NULL; vp = vp->v_link)
		if (*vp->v_name == *name && equal(vp->v_name, name))
			return(vp);
	return(NULL);
}

/*
 * Locate a group name and return it.
 */

struct grouphead *
findgroup(char name[])
{
	struct grouphead *gh;

	for (gh = groups[hash(name)]; gh != NULL; gh = gh->g_link)
		if (*gh->g_name == *name && equal(gh->g_name, name))
			return(gh);
	return(NULL);
}

/*
 * Print a group out on stdout
 */
void
printgroup(char name[])
{
	struct grouphead *gh;
	struct group *gp;

	if ((gh = findgroup(name)) == NULL) {
		printf("\"%s\": not a group\n", name);
		return;
	}
	printf("%s\t", gh->g_name);
	for (gp = gh->g_list; gp != NULL; gp = gp->ge_link)
		printf(" %s", gp->ge_name);
	putchar('\n');
}

/*
 * Hash the passed string and return an index into
 * the variable or group hash table.
 */
int
hash(char *name)
{
	int h = 0;

	while (*name) {
		h <<= 2;
		h += *name++;
	}
	if (h < 0 && (h = -h) < 0)
		h = 0;
	return (h % HSHSIZE);
}
