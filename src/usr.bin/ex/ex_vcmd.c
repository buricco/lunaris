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

/* vcmd.c */

/* This file contains the functions that handle VI commands */

#include "config.h"
#include "ex_ctype.h"
#include "ex_common.h"

/* This function puts the editor in EX mode */
MARK v_quit(void)
{
	move(LINES - 1, 0);
	mode = MODE_EX;
	return cursor;
}

/* This function causes the screen to be redrawn */
MARK v_redraw(void)
{
	redraw(MARK_UNSET, FALSE);
	return cursor;
}

/* This function executes a string of EX commands, and waits for a user keystroke
 * before returning to the VI screen.  If that keystroke is another ':', then
 * another EX command is read and executed.
 */
/*ARGSUSED*/
MARK v_1ex(MARK m, char *text)
{
	/* run the command.  be careful about modes & output */
	exwrote = (mode == MODE_COLON);
	doexcmd(text, '\\');
	exrefresh();

	/* if mode is no longer MODE_VI, then we should quit right away! */
	if (mode != MODE_VI && mode != MODE_COLON)
		return cursor;

	/* The command did some output.  Wait for a keystoke. */
	if (exwrote)
	{
		mode = MODE_VI;	
		msg("[Hit <RETURN> to continue]");
		if (getkey(0) == ':')
		{	mode = MODE_COLON;
			addch('\n');
		}
		else
			redraw(MARK_UNSET, FALSE);
	}

	return cursor;
}

/* This function undoes the last change */
/*ARGSUSED*/
MARK v_undo(MARK m)
{
	if (undo())
	{
		redraw(MARK_UNSET, FALSE);
	}
	return cursor;
}

/* This function deletes the character(s) that the cursor is on */
MARK v_xchar(MARK m, long cnt, int cmd)
{
	DEFAULT(1);

	/* for 'X', adjust so chars are deleted *BEFORE* cursor */
	if (cmd == 'X')
	{
		if (markidx(m) < cnt)
			return MARK_UNSET;
		m -= cnt;
	}

	/* make sure we don't try to delete more thars than there are */
	pfetch(markline(m));
	if (markidx(m + cnt) > plen)
	{
		cnt = plen - markidx(m);
	}
	if (cnt == 0L)
	{
		return MARK_UNSET;
	}

	/* do it */
	ChangeText
	{
		cut(m, m + cnt);
		delete(m, m + cnt);
	}
	return m;
}

/* This function defines a mark */
/*ARGSUSED*/
MARK v_mark(MARK m, long count, int key)
{
	if (key < 'a' || key > 'z')
	{
		msg("Marks must be from a to z");
	}
	else
	{
		mark[key - 'a'] = m;
	}
	return m;
}

/* This function toggles upper & lower case letters */
MARK v_ulcase(MARK m, long cnt)
{
	REG char 	*pos;
	REG int		j;

	DEFAULT(1);

	/* fetch the current version of the line */
	pfetch(markline(m));

	/* for each position in the line */
	for (j = 0, pos = &ptext[markidx(m)]; j < cnt && *pos; j++, pos++)
	{
		if (isupper(*pos))
		{
			tmpblk.c[j] = tolower(*pos);
		}
		else
		{
			tmpblk.c[j] = toupper(*pos);
		}
	}

	/* if the new text is different from the old, then change it */
	if (strncmp(tmpblk.c, &ptext[markidx(m)], j))
	{
		ChangeText
		{
			tmpblk.c[j] = '\0';
			change(m, m + j, tmpblk.c);
		}
	}

	return m + j;
}

MARK v_replace(MARK m, long cnt, int key)
{
	REG char	*text;
	REG int		i;

	DEFAULT(1);

	/* map ^M to '\n' */
	if (key == '\r')
	{
		key = '\n';
	}

	/* make sure the resulting line isn't too long */
	if (cnt > BLKSIZE - 2 - markidx(m))
	{
		cnt = BLKSIZE - 2 - markidx(m);
	}

	/* build a string of the desired character with the desired length */
	for (text = tmpblk.c, i = (key == '\n' ? 1 : cnt); i > 0; i--)
	{
		*text++ = key;
	}
	*text = '\0';

	/* make sure cnt doesn't extend past EOL */
	pfetch(markline(m));
	key = markidx(m);
	if (key + cnt > plen)
	{
		cnt = plen - key;
	}

	/* do the replacement */
	ChangeText
	{
		change(m, m + cnt, tmpblk.c);
	}

	if (*tmpblk.c == '\n')
	{
		return (m & ~(BLKSIZE - 1)) + BLKSIZE;
	}
	else
	{
		return m + cnt - 1;
	}
}

MARK v_overtype(MARK m)
{
	MARK		end;	/* end of a substitution */
	static long	width;	/* width of a single-line replace */

	/* the "doingdot" version of replace is really a substitution */
	if (doingdot)
	{
		/* was the last one really repeatable? */
		if (width < 0)
		{
			msg("Can't repeat a multi-line overtype command");
			return MARK_UNSET;
		}

		/* replacing nothing by nothing?  Don't bother */
		if (width == 0)
		{
			return m;
		}

		/* replace some chars by repeated text */
		return v_subst(m, width);
	}

	/* Normally, we input starting here, in replace mode */
	ChangeText
	{
		end = input(m, m, WHEN_VIREP, 0);
	}

	/* if we ended on the same line we started on, then this
	 * overtype is repeatable via the dot key.
	 */
	if (markline(end) == markline(m) && end >= m - 1L)
	{
		width = end - m + 1L;
	}
	else /* it isn't repeatable */
	{
		width = -1L;
	}

	return end;
}

/* This function selects which cut buffer to use */
/*ARGSUSED*/
MARK v_selcut(MARK m, long count, int key)
{
	cutname(key);
	return m;
}

/* This function pastes one or more copies of a cut buffer into the main
 * edit buffer.
 */
MARK v_paste(MARK m, long cnt, int cmd)
{
	MARK    dest;
	int     i;
  
	ChangeText
	{
		/* paste the text, and find out where it ends */
		dest = paste(m, cmd == 'p', TRUE);
		if (cnt > 1)
		{
			for (i = 1; i != cnt; i++)
			{
				dest = paste(dest, TRUE, TRUE);
			}
		}

		/* was that a line-mode paste? */
		if (dest && markline(dest) != markline(m))
		{
			/* line-mode pastes leave the cursor at the front
			 * of the first pasted line.
			 */
			dest = m;
			if (cmd == 'p')
			{
				dest += BLKSIZE;
			}
			force_flags |= FRNT;
		}
	}
	return dest;
}

/* This function yanks text into a cut buffer */
MARK v_yank(MARK m, MARK n)
{
	cut(m, n);
	return m;
}

/* This function deletes a range of text */
MARK v_delete(MARK m, MARK n)
{
	/* illegal to try and delete nothing */
	if (n <= m)
	{
		return MARK_UNSET;
	}

	/* Do it */
	ChangeText
	{
		cut(m, n);
		delete(m, n);
	}
	return m;
}

/* This starts input mode without deleting anything */
MARK v_insert(MARK m, long cnt, int key)
{
	int	wasdot;
	long	reps;
	int	delta = 0;/* 1 to take autoindent from line below, -1 for above */

	DEFAULT(1);

	ChangeText
	{
		/* tweak the insertion point, based on command key */
		switch (key)
		{
		  case 'i':
			break;

		  case 'a':
			pfetch(markline(m));
			if (plen > 0)
			{
				m++;
			}
			break;

		  case 'I':
			m = m_front(m, 1L);
			break;

		  case 'A':
			pfetch(markline(m));
			m = (m & ~(BLKSIZE - 1)) + plen;
			break;

		  case 'O':
			m &= ~(BLKSIZE - 1);
			add(m, "\n");
			delta = 1;
			break;

		  case 'o':
			m = (m & ~(BLKSIZE - 1)) + BLKSIZE;
			add(m, "\n");
			delta = -1;
			break;
		}

		/* insert the same text once or more */
		for (reps = cnt, wasdot = doingdot; reps > 0; reps--, doingdot = TRUE)
		{
			m = input(m, m, WHEN_VIINP, delta) + 1;
		}
		if (markidx(m) > 0)
		{
			m--;
		}

		doingdot = wasdot;
	}

#ifndef CRUNCH
# ifndef NO_EXTENSIONS
	if (key == 'i' && *o_inputmode && mode == MODE_VI)
	{
		msg("Now in command mode!  To return to input mode, hit <i>");
	}
# endif
#endif

	return m;
}

/* This starts input mode with some text deleted */
MARK v_change(MARK m, MARK n)
{
	int	lnmode;	/* is this a line-mode change? */

	/* swap them if they're in reverse order */
	if (m > n)
	{
		MARK	tmp;
		tmp = m;
		m = n;
		n = tmp;
	}

	/* for line mode, retain the last newline char */
	lnmode = (markidx(m) == 0 && markidx(n) == 0 && m != n);
	if (lnmode)
	{
		n -= BLKSIZE;
		pfetch(markline(n));
		n = (n & ~(BLKSIZE - 1)) + plen;
	}

	ChangeText
	{
		cut(m, n);
		m = input(m, n, WHEN_VIINP, 0);
	}

	return m;
}

/* This function replaces a given number of characters with input */
MARK v_subst(MARK m, long cnt)
{
	DEFAULT(1);

	/* make sure we don't try replacing past EOL */
	pfetch(markline(m));
	if (markidx(m) + cnt > plen)
	{
		cnt = plen - markidx(m);
	}

	/* Go for it! */
	ChangeText
	{
		cut(m, m + cnt);
		m = input(m, m + cnt, WHEN_VIINP, 0);
	}
	return m;
}

/* This calls the ex "join" command to join some lines together */
MARK v_join(MARK m, long cnt)
{
	MARK	joint;	/* where the lines were joined */

	DEFAULT(1);
	if (cnt > 1)
	{
		cnt--;
	}

	/* figure out where the joint will be */
	pfetch(markline(m));
	joint = (m & ~(BLKSIZE - 1)) + plen;

	/* join the lines */
	cmd_join(m, m + MARK_AT_LINE(cnt), CMD_JOIN, 0, "");

	/* the cursor should be left at the joint */
	return joint;
}

/* This calls the ex "<" command to shift some lines left */
MARK v_lshift(MARK m, MARK n)
{
	/* adjust for inclusive endmarks in ex */
	n -= BLKSIZE;

	cmd_shift(m, n, CMD_SHIFTL, FALSE, "");

	return m;
}

/* This calls the ex ">" command to shift some lines right */
MARK v_rshift(MARK m, MARK n)
{
	/* adjust for inclusive endmarks in ex */
	n -= BLKSIZE;

	cmd_shift(m, n, CMD_SHIFTR, FALSE, "");

	return m;
}

/* This filters some lines through a preset program, to reformat them */
MARK v_reformat(MARK m, MARK n)
{
	/* adjust for inclusive endmarks in ex */
	n -= BLKSIZE;

	/* run the filter command */
	filter(m, n, o_equalprg, TRUE);

	redraw(MARK_UNSET, FALSE);
	return m;
}

/* This runs some lines through a filter program */
MARK v_filter(MARK m, MARK n)
{
	static char	cmdln[150];	/* a shell command line */

#ifndef NO_SAFER
	if (*o_safer)
	{
		msg("visual ! command is unsafe");
		return MARK_UNSET;
	}
#endif

	/* adjust for inclusive endmarks in ex */
	n -= BLKSIZE;

	if (doingdot || vgets('!', cmdln, sizeof(cmdln)) > 0)
	{
		filter(m, n, cmdln, TRUE);
	}

	redraw(MARK_UNSET, FALSE);
	return m;
}

/* This function runs the ex "file" command to show the file's status */
MARK v_status(void)
{
	cmd_file(cursor, cursor, CMD_FILE, 0, "");
	return cursor;
}

/* This function runs the ":&" command to repeat the previous :s// */
MARK v_again(MARK m, MARK n)
{
	cmd_substitute(m, n - BLKSIZE, CMD_SUBAGAIN, TRUE, "");
	return cursor;
}

/* This function switches to the previous file, if possible */
MARK v_switch(void)
{
	if (!*prevorig)
		msg("No previous file");
	else
	{	strcpy(tmpblk.c, prevorig);
		cmd_edit(cursor, cursor, CMD_EDIT, 0, tmpblk.c);
	}
	return cursor;
}

/* This function does a tag search on a keyword */
/*ARGSUSED*/
MARK v_tag(char *keyword, MARK m, long cnt)
{
	/* move the cursor to the start of the tag name, where m is */
	cursor = m;

	/* perform the tag search */
	cmd_tag(cursor, cursor, CMD_TAG, 0, keyword);

	return cursor;
}

#ifndef NO_EXTENSIONS
/* This function looks up a keyword by calling the helpprog program */
/*ARGSUSED*/
MARK v_keyword(char *keyword, MARK m, long cnt)
{
	int	waswarn;
	char	cmdline[130];

	move(LINES - 1, 0);
	addstr("---------------------------------------------------------\n");
	clrtoeol();
	refresh();
	sprintf(cmdline, "%s %s", o_keywordprg, keyword);
	waswarn = *o_warn;
	*o_warn = FALSE;
	suspend_curses();
	if (system(cmdline))
	{
		addstr("<<< failed >>>\n");
	}
	resume_curses(FALSE);
	mode = MODE_VI;
	redraw(MARK_UNSET, FALSE);
	*o_warn = waswarn;

	return m;
}

MARK v_increment(char *keyword, MARK m, long cnt)
{
	static	int sign;
	char	newval[12];

	DEFAULT(1);

	/* get one more keystroke, unless doingdot */
	if (!doingdot)
	{
		sign = getkey(0);
	}

	/* adjust the number, based on that second keystroke */
	switch (sign)
	{
	  case '+':
	  case '#':
		cnt = atol(keyword) + cnt;
		break;

	  case '-':
		cnt = atol(keyword) - cnt;
		break;

	  case '=':
		break;

	  default:
		return MARK_UNSET;
	}
	sprintf(newval, "%ld", cnt);

	ChangeText
	{
		change(m, m + strlen(keyword), newval);
	}

	return m;
}
#endif

/* This function acts like the EX command "xit" */
/*ARGSUSED*/
MARK v_xit(MARK m, long count, int key)
{
	/* if second char wasn't 'Z', fail */
	if (key != 'Z')
	{
		return MARK_UNSET;
	}

	/* move the cursor to the bottom of the screen */
	move(LINES - 1, 0);
	clrtoeol();

	/* do the xit command */
	cmd_xit(m, m, CMD_XIT, FALSE, "");

	/* return the cursor */
	return m;
}

/* This function undoes changes to a single line, if possible */
MARK v_undoline(MARK m)
{
	/* make sure we have the right line in the buffer */
	if (markline(m) != U_line)
	{
		return MARK_UNSET;
	}

	/* fix it */
	ChangeText
	{
		strcat(U_text, "\n");
		change(MARK_AT_LINE(U_line), MARK_AT_LINE(U_line + 1), U_text);
	}

	/* nothing in the buffer anymore */
	U_line = -1L;

	/* return, with the cursor at the front of the line */
	return m & ~(BLKSIZE - 1);
}

#ifndef NO_ERRLIST
MARK v_errlist(MARK m)
{
	cmd_errlist(m, m, CMD_ERRLIST, FALSE, "");
	return cursor;
}
#endif

#ifndef NO_AT
/*ARGSUSED*/
MARK v_at(MARK m, long count, int key)
{
	int	size;

	size = cb2str(key, tmpblk.c, BLKSIZE);
	if (size <= 0 || size == BLKSIZE)
	{
		return MARK_UNSET;
	}

	execmap(0, tmpblk.c, FALSE);
	return cursor;
}
#endif

#ifdef SIGTSTP
MARK v_suspend(void)
{
	cmd_suspend(MARK_UNSET, MARK_UNSET, CMD_SUSPEND, FALSE, "");
	return MARK_UNSET;
}
#endif

#ifndef NO_VISIBLE
/*ARGSUSED*/
MARK v_start(MARK m, long cnt, int cmd)
{
	if (V_from)
	{
		V_from = MARK_UNSET;
	}
	else
	{
		V_from = m;
		V_linemd = isupper(cmd);
	}
	return m;
}
#endif

#ifndef NO_POPUP
# define MENU_HEIGHT 11
# define MENU_WIDTH  23
MARK v_popup(MARK m, MARK n)
{
	int		i;
	int		y, x;	/* position where the window will pop up at */
	int		key;	/* keystroke from the user */
	int		sel;	/* index of the selected operation */
	static int	dfltsel;/* default value of sel */
	static char	*labels[11] =
	{
		"ESC cancel!         ",
		" d  delete (cut)    ",
		" y  yank (copy)     ",
		" p  paste after     ",
		" P  paste before    ",
		" >  more indented   ",
		" <  less indented   ",
		" =  reformat        ",
		" !  external filter ",
		" ZZ save & exit     ",
		" u  undo previous   "
	};

	/* try to position the menu near the cursor */
	x = physcol - (MENU_WIDTH / 2);
	if (x < 0)
		x = 0;
	else if (x + MENU_WIDTH + 2 > COLS)
		x = COLS - MENU_WIDTH - 2;
	if (markline(cursor) < topline || markline(cursor) > botline)
		y = 0;
	else if (markline(cursor) + 1L + MENU_HEIGHT > botline)
		y = (int)(markline(cursor) - topline) - MENU_HEIGHT;
	else
		y = (int)(markline(cursor) - topline) + 1L;

	/* draw the menu */
	for (sel = 0; sel < MENU_HEIGHT; sel++)
	{
		move(y + sel, x);
		do_POPUP();
		if (sel == dfltsel)
			qaddstr("-->");
		else
			qaddstr("   ");
		qaddstr(labels[sel]);
		do_SE();
	}

	/* get a selection */
	move(y + dfltsel, x + 4);
	for (sel = dfltsel; (key = getkey(WHEN_POPUP)) != '\\' && key != '\r'; )
	{
		/* erase the selection arrow */
		move(y + sel, x);
		do_POPUP();
		qaddstr("   ");
		qaddstr(labels[sel]);
		do_SE();

		/* process the user's keystroke */
		if (key == 'j' && sel < MENU_HEIGHT - 1)
		{
			sel++;
		}
		else if (key == 'k' && sel > 0)
		{
			sel--;
		}
		else if (key == '\033')
		{
			sel = 0;
			break;
		}
		else
		{
			for (i = 1; i < MENU_HEIGHT && labels[i][1] != key; i++)
			{
			}
			if (i < MENU_HEIGHT)
			{
				sel = i;
				break;
			}
		}

		/* redraw the arrow, possibly in a new place */
		move(y + sel, x);
		do_POPUP();
		qaddstr("-->");
		qaddstr(labels[sel]);
		do_SE();
		move(y + sel, x + 4);
	}

	/* in most cases, the default selection is "paste after" */
	dfltsel = 3;

	/* perform the appropriate action */
	switch (sel)
	{
	  case 0:
		m = cursor;
		dfltsel = 0;
		break;

	  case 1: /* delete (cut) */
		m = v_delete(m, n);
		break;

	  case 2: /* yank (copy) */
		m = v_yank(m, n);
		break;

	  case 3: /* paste after */
		m = v_paste(n, 1L, 'P');
		break;

	  case 4: /* paste before */
		m = v_paste(m, 1L, 'P');
		dfltsel = 4;
		break;

	  case 5: /* more indented */
		m = v_rshift(m, n);
		dfltsel = 5;
		break;

	  case 6: /* less indented */
		m = v_lshift(m, n);
		dfltsel = 6;
		break;

	  case 7: /* reformat */
		m = v_reformat(m, n);
		dfltsel = 7;
		break;

	  case 8: /* external filter */
		m = v_filter(m, n);
		dfltsel = 0;
		break;

	  case 9: /* save & exit */
		/* get confirmation first */
		do
		{
			key = getkey(0);
		} while (key != '\\' && key != 'Z' && key != '\r'    /* good */
		      && key != '\033');			     /* bad  */
		if (key != '\033')
		{
			m = v_xit(m, 0L, 'Z');
		}
		break;

	  case 10: /* undo previous */
		m = v_undo(m);
		dfltsel = 9;
		break;
	}

	/* arrange for the menu to be erased (except "save & exit" doesn't care)
	 */
	if (mode == MODE_VI)
		redraw(MARK_UNSET, FALSE);

	return m;
}
#endif /* undef NO_POPUP */

#ifndef NO_TAGSTACK
MARK v_pop(MARK m, long cnt, int cmd)
{
	DEFAULT(1L);
	sprintf(tmpblk.c, "%ld", cnt);
	cmd_pop(m, m, CMD_POP, FALSE, tmpblk.c);
	return cursor;
}
#endif
