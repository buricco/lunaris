/*	$NetBSD: win.c,v 1.12 2009/05/25 00:39:45 dholland Exp $	*/

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
static char sccsid[] = "@(#)win.c	8.1 (Berkeley) 5/31/93";
#else
static char *rcsid="$NetBSD: win.c,v 1.12 2009/05/25 00:39:45 dholland Exp $";
#endif
#endif /* not lint */

#include <stdio.h>
#include <unistd.h>
#include <setjmp.h>
#include "trek.h"
#include "getpar.h"

/*
**  Signal game won
**
**	This routine prints out the win message, arranges to print out
**	your score, tells you if you have a promotion coming to you,
**	cleans up the current input line, and arranges to have you
**	asked whether or not you want another game (via the longjmp()
**	call).
**
**	Pretty straightforward, although the promotion algorithm is
**	pretty off the wall.
*/

extern jmp_buf env;

void
win(void)
{
	long		s;
	const struct cvntab *p;

	sleep(1);
	printf("\nCongratulations, you have saved the Federation\n");
	Move.endgame = 1;

	/* print and return the score */
	s = score();

	/* decide if she gets a promotion */
	if (Game.helps == 0 && Game.killb == 0 && Game.killinhab == 0 &&
	    5 * Game.kills + Game.deaths < 100 &&
	    s >= 1000 && Ship.ship == ENTERPRISE) {
		printf("In fact, you are promoted one step in rank,\n");
		if (Game.skill >= 6) {
			printf("to the exalted rank of Commodore Emeritus\n");
		} else {
			p = &Skitab[Game.skill - 1];
			printf("from %s%s ", p->abbrev, p->full);
			p++;
			printf("to %s%s\n", p->abbrev, p->full);
		}
	}

	/* clean out input, and request new game */
	skiptonl(0);
	longjmp(env, 1);
}
