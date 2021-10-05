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

/* vars.c */

/* This file contains variables which weren't happy anyplace else */

#include "config.h"
#include "ex_common.h"

/*------------------------------------------------------------------------*/

/* used to remember whether the file has been modified */
struct _viflags	viflags;

/* used to access the tmp file */
long		lnum[MAXBLKS];
long		nlines;
int		tmpfd = -1;
int		tmpnum;

/* used to keep track of the current file & alternate file */
long		origtime;
char		origname[256];
char		prevorig[256];
long		prevline = 1;

/* used to track various places in the text */
MARK		mark[NMARKS];	/* marks 'a through 'z, plus mark '' */
MARK		cursor;		/* the cursor position within the file */

/* which mode of the editor we're in */
int		mode;		/* vi mode? ex mode? quitting? */

/* used to manage the args list */
char		args[BLKSIZE];	/* list of filenames to edit */
int		argno;		/* index of current file in args list */
int		nargs;		/* number of filenames in args[] */

/* dummy var, never explicitly referenced */
int		bavar;		/* used only in BeforeAfter macros */

/* used to detect changes that invalidate cached text/blocks */
long		changes;	/* incremented when file is changed */
int		significant;	/* boolean: was a *REAL* change made? */
int		exitcode = 1;	/* 0=overwritten, 1=not updated, else error */

/* used to support the pfetch() macro */
int		plen;		/* length of the line */
long		pline;		/* line number that len refers to */
long		pchgs;		/* "changes" level that len refers to */
char		*ptext;		/* text of previous line, if valid */

/* misc temporary storage - mostly for strings */
BLK		tmpblk;		/* a block used to accumulate changes */

/* screen oriented stuff */
long		topline;	/* file line number of top line */
int		leftcol;	/* column number of left col */
int		physcol;	/* physical column number that cursor is on */
int		physrow;	/* physical row number that cursor is on */

/* used to help minimize that "[Hit a key to continue]" message */
int		exwrote;	/* Boolean: was the last ex command wordy? */

/* This variable affects the behaviour of certain functions -- most importantly
 * the input function.
 */
int		doingdot;	/* boolean: are we doing the "." command? */

/* This variable affects the behaviour of the ":s" command, and it is also
 * used to detect & prohibit nesting of ":g" commands
 */
int		doingglobal;	/* boolean: are doing a ":g" command? */

/* This variable is zeroed before a command executes, and later ORed with the
 * command's flags after the command has been executed.  It is used to force
 * certain flags to be TRUE for *some* invocations of a particular command.
 * For example, "/regexp/+offset" forces the LNMD flag, and sometimes a "p"
 * or "P" command will force FRNT.
 */
int		force_flags;

/* These are used for reporting multi-line changes to the user */
long		rptlines;	/* number of lines affected by a command */
char		*rptlabel;	/* description of how lines were affected */

/* These store info that pertains to the shift-U command */
long	U_line;			/* line# of the undoable line, or 0l for none */
char	U_text[BLKSIZE];	/* contents of the undoable line */

#ifndef NO_VISIBLE
/* These are used to implement the 'v' and 'V' commands */
MARK	V_from;			/* starting point for v or V */
int	V_linemd;		/* boolean: doing line-mode version? (V, not v) */
#endif

#ifndef NO_LEARN
/* This is used by the "learn" mode */
char	learn;			/* name of buffer being learned */
#endif
