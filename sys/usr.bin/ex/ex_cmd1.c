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

/* cmd1.c */

/* This file contains some of the EX commands - mostly ones that deal with
 * files, options, etc. -- anything except text.
 */

#include "config.h"
#include "ex_ctype.h"
#include "ex_common.h"
#include "ex_regexp.h"

extern int rpipe (char *, int);
extern int rpclose (int);

#ifndef NO_TAGSTACK
/* These describe the current state of the tag related commands		  */
#define MAXTAGS	15

struct Tag_item {
    MARK		tag_mark;
    char		*tag_file;
};

static struct Tag_item	tag_stack[MAXTAGS];
static int		curr_tag = -1;
#endif /* !NO_TAGSTACK */

#ifdef DEBUG
/* print the selected lines with info on the blocks */
/*ARGSUSED*/
void cmd_debug(MARK frommark, MARK tomark, CMD cmd, int bang, char *extra)
{
	REG char	*scan;
	REG long	l;
	REG int		i;
	int		len;

	/* scan lnum[] to determine which block its in */
	l = markline(frommark);
	for (i = 1; l > lnum[i]; i++)
	{
	}

	do
	{
		/* fetch text of the block containing that line */
		scan = blkget(i)->c;

		/* calculate its length */
		if (scan[BLKSIZE - 1])
		{
			len = BLKSIZE;
		}
		else
		{
			len = strlen(scan);
		}

		/* print block stats */
		msg("##### hdr[%d]=%d, lnum[%d-1]=%ld, lnum[%d]=%ld (%ld lines)",
			i, hdr.n[i], i, lnum[i-1], i, lnum[i], lnum[i] - lnum[i - 1]);
		msg("##### len=%d, buf=0x%lx, %sdirty",
			len, scan, ((int *)scan)[MAXBLKS + 1] ? "" : "not ");
		if (bang)
		{
			while (--len >= 0)
			{
				addch(*scan);
				scan++;
			}
		}
		exrefresh();

		/* next block */
		i++;
	} while (i < MAXBLKS && lnum[i] && lnum[i - 1] < markline(tomark));
}

/* This function checks a lot of conditions to make sure they aren't screwy */
/*ARGSUSED*/
void cmd_validate(MARK frommark, MARK tomark, CMD cmd, int bang, char *extra)
{
	char	*scan;
	int	i;
	int	nlcnt;	/* used to count newlines */
	int	len;	/* counts non-NUL characters */

	/* check lnum[0] */
	if (lnum[0] != 0L)
	{
		msg("lnum[0] = %ld", lnum[0]);
	}

	/* check each block */
	for (i = 1; lnum[i] <= nlines; i++)
	{
		scan = blkget(i)->c;
		if (scan[BLKSIZE - 1])
		{
			msg("block %d has no NUL at the end", i);
		}
		else
		{
			for (nlcnt = len = 0; *scan; scan++, len++)
			{
				if (*scan == '\n')
				{
					nlcnt++;
				}
			}
			if (scan[-1] != '\n')
			{
				msg("block %d doesn't end with '\\n' (length %d)", i, len);
			}
			if (bang || nlcnt != lnum[i] - lnum[i - 1])
			{
				msg("block %d (line %ld?) has %d lines, but should have %ld",
					i, lnum[i - 1] + 1L, nlcnt, lnum[i] - lnum[i - 1]);
			}
		}
		exrefresh();
	}

	/* check lnum again */
	if (lnum[i] != INFINITY)
	{
		msg("hdr.n[%d] = %d, but lnum[%d] = %ld",
			i, hdr.n[i], i, lnum[i]);
	}

	msg("# = \"%s\", %% = \"%s\"", prevorig, origname);
	msg("V_from=%ld.%d, cursor=%ld.%d", markline(V_from), markidx(V_from), markline(cursor), markidx(cursor));
}
#endif /* DEBUG */

/*ARGSUSED*/
void cmd_mark(MARK frommark, MARK tomark, CMD cmd, int bang, char *extra)
{
	/* validate the name of the mark */
	if (*extra == '"')
	{
		extra++;
	}
	/* valid mark names are lowercase ascii characters */
	if (!isascii(*extra) || !islower(*extra) || extra[1])
	{
		msg("Invalid mark name");
		return;
	}

	mark[*extra - 'a'] = tomark;
}

/*ARGSUSED*/
void cmd_write(MARK frommark, MARK tomark, CMD cmd, int bang, char *extra)
{
	int		fd;
	int		append;	/* boolean: write in "append" mode? */
	REG long	l;
	REG char	*scan;
	REG int		i;

	/* if writing to a filter, then let filter() handle it */
	if (*extra == '!')
	{
		filter(frommark, tomark, extra + 1, FALSE);
		return;
	}

	/* if all lines are to be written, use tmpsave() since it is faster.
	 * Exception: if cmd_write was called by the filter() function
	 * (i.e., cmd == CMD_BANG) then do it the slow way anyway, so we
	 * can avoid clobbering prevorig.
	 */
	if (frommark == MARK_FIRST && tomark == MARK_LAST && cmd == CMD_WRITE)
	{
		tmpsave(extra, bang);
		return;
	}

	/* see if we're going to do this in append mode or not */
	append = FALSE;
	if (extra[0] == '>' && extra[1] == '>')
	{
		extra += 2;
		append = TRUE;
	}

	/* either the file must not exist, or we must have a ! or be appending */
	if (*extra && access(extra, 0) == 0 && !bang && !append)
	{
		msg("File already exists - Use :w! to overwrite");
		return;
	}

	/* else do it line-by-line, like cmd_print() */
	if (append)
	{
#ifdef O_APPEND
		fd = open(extra, O_WRONLY|O_APPEND);
#else
		fd = open(extra, O_WRONLY);
		if (fd >= 0)
		{
			lseek(fd, 0L, 2);
		}
#endif
	}
	else
	{
		fd = -1; /* so we know the file isn't open yet */
	}

	if (fd < 0)
	{
		fd = creat(extra, FILEPERMS);
		if (fd < 0)
		{
			msg("Can't write to \"%s\"", extra);
			return;
		}
	}
	for (l = markline(frommark); l <= markline(tomark); l++)
	{
		/* get the next line */
		scan = fetchline(l);
		i = strlen(scan);
		scan[i++] = '\n';

		/* print the line */
		if (twrite(fd, scan, i) < i)
		{
			msg("Write failed");
			break;
		}
	}
	rptlines = markline(tomark) - markline(frommark) + 1;
	rptlabel = "written";

#ifndef CRUNCH
	/* make # refer to the file we just wrote */
	if (strcmp(origname, extra) && cmd != CMD_BANG)
	{
		strcpy(prevorig, extra);
	}
#endif
	close(fd);
}	

/*ARGSUSED*/
void cmd_shell(MARK frommark, MARK tomark, CMD cmd, int bang, char *extra)
{
	static char	prevextra[132];

	/* special case: ":sh" means ":!sh" */
	if (cmd == CMD_SHELL)
	{
		extra = o_shell;
		frommark = tomark = 0L;
	}

	/* if already did one or more shell commands, need extra newline to
	 * avoid overwriting user's command string */
	if (mode == MODE_COLON)
	{
		addch('\n');
	}

	/* if extra is "!", substitute previous command */
	if (*extra == '!')
	{
		if (!*prevextra)
		{
			msg("No previous shell command to substitute for '!'");
			return;
		}
		strcat(prevextra, extra + 1);
		extra = prevextra;
	}
	else if (cmd == CMD_BANG && (unsigned)strlen(extra) < sizeof(prevextra) - 1)
	{
		strcpy(prevextra, extra);
	}

	/* warn the user if the file hasn't been saved yet */
	if (*o_warn && tstflag(file, MODIFIED))
	{
		if (mode == MODE_VI)
		{
			mode = MODE_COLON;
		}
		msg("Warning: \"%s\" has been modified but not yet saved", origname);
	}

	/* if no lines were specified, just run the command */
	suspend_curses();
	if (frommark == 0L)
	{
		system(extra);
	}
	else /* pipe lines from the file through the command */
	{
		filter(frommark, tomark, extra, TRUE);
	}

	/* resume curses quietly for MODE_EX, but noisily otherwise */
	resume_curses(mode == MODE_EX);
}

/*ARGSUSED*/
void cmd_global(MARK frommark, MARK tomark, CMD cmd, int bang, char *extra)
{
	char	*cmdptr;	/* the command from the command line */
	char	cmdln[100];	/* copy of the command from the command line */
	char	*line;		/* a line from the file */
	long	l;		/* used as a counter to move through lines */
	long	lqty;		/* quantity of lines to be scanned */
	long	nchanged;	/* number of lines changed */
	regexp	*re;		/* the compiled search expression */

	/* can't nest global commands */
	if (doingglobal)
	{
		msg("Can't nest global commands.");
		rptlines = -1L;
		return;
	}

	/* ":g! ..." is the same as ":v ..." */
	if (bang)
	{
		cmd = CMD_VGLOBAL;
	}

	/* make sure we got a search pattern */
	if (!*extra)
	{
		msg("Usage: %c /regular expression/ command", cmd == CMD_GLOBAL ? 'g' : 'v');
		return;
	}

	/* parse & compile the search pattern */
	cmdptr = parseptrn(extra);
#if 0
	if (!extra[1])
	{
		msg("Can't use empty regular expression with '%c' command", cmd == CMD_GLOBAL ? 'g' : 'v');
		return;
	}
#endif
	re = regcomp(extra + 1);
	if (!re)
	{
		/* regcomp found & described an error */
		return;
	}

	/* for each line in the range */
	doingglobal = TRUE;
	ChangeText
	{
		/* NOTE: we have to go through the lines in a forward order,
		 * otherwise "g/re/p" would look funny.  *BUT* for "g/re/d"
		 * to work, simply adding 1 to the line# on each loop won't
		 * work.  The solution: count lines relative to the end of
		 * the file.  Think about it.
		 */
		for (l = nlines - markline(frommark),
			lqty = markline(tomark) - markline(frommark) + 1L,
			nchanged = 0L;
		     lqty > 0 && nlines - l >= 0 && nchanged >= 0L;
		     l--, lqty--)
		{
			/* fetch the line */
			line = fetchline(nlines - l);

			/* if it contains the search pattern... */
			if ((!regexec(re, line, 1)) == (cmd != CMD_GLOBAL))
			{
				/* move the cursor to that line */
				cursor = MARK_AT_LINE(nlines - l);

				/* do the ex command (without mucking up
				 * the original copy of the command line)
				 */
				strcpy(cmdln, cmdptr);
				rptlines = 0L;
				doexcmd(cmdln, '\\');
				nchanged += rptlines;
			}
		}
	}
	doingglobal = FALSE;

	/* free the regexp */
	free(re);

	/* Reporting...*/
	rptlines = nchanged;
}

/*ARGSUSED*/
void cmd_file(MARK frommark, MARK tomark, CMD cmd, int bang, char *extra)
{
#ifndef CRUNCH
	/* if we're given a new filename, use it as this file's name */
	if (extra && *extra)
	{
		strcpy(origname, extra);
		storename(origname);
		setflag(file, NOTEDITED);
	}
#endif
	if (cmd == CMD_FILE)
	{
#ifndef CRUNCH
		msg("\"%s\" %s%s%s line %ld of %ld [%ld%%]",
#else
		msg("\"%s\" %s%s line %ld of %ld [%ld%%]",
#endif
			*origname ? origname : "[NO FILE]",
			tstflag(file, MODIFIED) ? "[MODIFIED]" : "",
#ifndef CRUNCH
			tstflag(file, NOTEDITED) ?"[NOT EDITED]":"",
#endif
			tstflag(file, READONLY) ? "[READONLY]" : "",
			markline(frommark),
			nlines,
			markline(frommark) * 100 / nlines);
	}
#ifndef CRUNCH
	else if (markline(frommark) != markline(tomark))
	{
		msg("range \"%ld,%ld\" contains %ld lines",
			markline(frommark),
			markline(tomark),
			markline(tomark) - markline(frommark) + 1L);
	}
#endif
	else
	{
		msg("%ld", markline(frommark));
	}
}

/*ARGSUSED*/
void cmd_edit(MARK frommark, MARK tomark, CMD cmd, int bang, char *extra)
{
	long	line = 1L;	/* might be set to prevline */
#ifndef CRUNCH
	char	init[100];
	int	i;
#endif

	/* if ":vi", then switch to visual mode, and if no file is named
	 * then don't switch files.
	 */
	if (cmd == CMD_VISUAL)
	{
		mode = MODE_VI;
		msg("");
		if (!*extra)
		{
			return;
		}
	}

	/* Editing previous file?  Then start at previous line */
	if (!strcmp(extra, prevorig))
	{
		line = prevline;
	}

#ifndef CRUNCH
	/* if we were given an explicit starting line, then start there */
	if (*extra == '+')
	{
		/* copy the command into init */
		for (i = 0, ++extra; i < sizeof(init) - 1 && !isspace(*extra); extra++)
		{
			init[i++] = *extra;
		}
		if (i == 0)
		{
			init[i++] = '$';
		}
		init[i] = '\0';

		/* leave "extra" pointing to filename */
		while (isspace(*extra))
		{
			extra++;
		}
		if (!*extra)
		{
			extra = origname;
		}
	}
	else
	{
		init[0] = '\0';
	}
#endif /* not CRUNCH */

	/* switch files */
	if (tmpabort(bang))
	{
		tmpstart(extra);
		if (line <= nlines && line >= 1L)
		{
			cursor = MARK_AT_LINE(line);
		}
#ifndef CRUNCH
		if (init[0])
		{
			doexcmd(init, '\\');
		}
#endif
	}
	else
	{
		msg("Use edit! to abort changes, or w to save changes");

		/* so we can say ":e!#" next time... */
		strcpy(prevorig, extra);
		prevline = 1L;
	}
}

/* This code is also used for rewind -- GB */

/*ARGSUSED*/
void cmd_next(MARK frommark, MARK tomark, CMD cmd, int bang, char *extra)
{
	int	i, j;
	char	*scan;

	/* if extra stuff given, use ":args" to define a new args list */
	if (cmd == CMD_NEXT && extra && *extra)
	{
		cmd_args(frommark, tomark, cmd, bang, extra);
	}

	/* move to the next arg */
	if (cmd == CMD_NEXT)
	{
		i = argno + 1;
	}
	else if (cmd == CMD_PREVIOUS)
	{
		i = argno - 1;
	}
	else /* cmd == CMD_REWIND */
	{
		i = 0;
	}	
	if (i < 0 || i >= nargs)
	{
		msg("No %sfiles to edit", cmd == CMD_REWIND ? "" : "more ");
		return;
	}

	/* find & isolate the name of the file to edit */
	for (j = i, scan = args; j > 0; j--)
	{
		while(*scan++)
		{
		}
	}

	/* switch to the next file */
	if (tmpabort(bang))
	{
		tmpstart(scan);
		argno = i;
	}
	else
	{
		msg("Use :%s! to abort changes, or w to save changes",
			cmd == CMD_NEXT ? "next" :
			cmd == CMD_PREVIOUS ? "previous" :
					"rewind");
	}
}

/* also called for :wq -- always writes back in this case */
/* also called for :q -- never writes back in that case */
/*ARGSUSED*/
void cmd_xit(MARK frommark, MARK tomark, CMD cmd, int bang, char *extra)
{
	static long	whenwarned;	/* when the user was last warned of extra files */
	int		oldflag;

	/* Handle :wq! command */
	if (cmd == CMD_WQUIT && bang && tstflag(file, MODIFIED)) {
		if (!tmpsave(extra, bang)) {
			msg("Could not save file");
			return;
		}
		mode = MODE_QUIT;
		return;
	}

	/* Unless the command is ":q", save the file if it has been modified */
	if (cmd != CMD_QUIT
	 && (cmd == CMD_WQUIT || tstflag(file, MODIFIED)) 
	 && !tmpsave((char *)0, FALSE) && !bang)
	{
		msg("Could not save file -- use quit! to abort changes, or w filename");
		return;
	}

	/* If there are more files to edit, then warn user */
	if (argno >= 0 && argno + 1 < nargs	/* more args */
	 && whenwarned != changes		/* user not already warned */
	 && (!bang || cmd != CMD_QUIT))		/* command not ":q!" */
	{
		msg("More files to edit -- Use \":n\" to go to next file");
		whenwarned = changes;
		return;
	}

	/* Discard the temp file.  Note that we should already have saved the
	 * the file, unless the command is ":q", so the only way that tmpabort
	 * could fail would be if you did a ":q" on a modified file.
	 */
	oldflag = *o_autowrite;
	*o_autowrite = FALSE;
	if (tmpabort(bang))
	{
		mode = MODE_QUIT;
	}
	else
	{
		msg("Use q! to abort changes, or wq to save changes");
	}
	*o_autowrite = oldflag;
}

/*ARGSUSED*/
void cmd_args(MARK frommark, MARK tomark, CMD cmd, int bang, char *extra)
{
	char	*scan;
	int	col;
	int	arg;
	int	scrolled = FALSE;
	int	width;

	/* if no extra names given, or just current name, then report the args
	 * we have now.
	 */
	if (!extra || !*extra)
	{
		/* empty args list? */
		if (nargs == 1 && !*args)
		{
			return;
		}

		/* list the arguments */
		for (scan = args, col = arg = 0;
		     arg < nargs;
		     scan += width + 1, col += width, arg++)
		{
			width = strlen(scan);
			if (col + width >= COLS - 4)
			{
				addch('\n');
				col = 0;
				scrolled = TRUE;
			}
			else if (col > 0)
			{
				addch(' ');
				col++;
			}
			if (arg == argno)
			{
				addch('[');
				addstr(scan);
				addch(']');
				col += 2;
			}
			else
			{
				addstr(scan);
			}
		}

		/* write a trailing newline */
		if ((mode == MODE_EX || mode == MODE_COLON || scrolled) && col)
		{
			addch('\n');
		}
		exrefresh();	
	}
	else /* new args list given */
	{
		for (scan = args, nargs = 1; *extra; )
		{
			if (isspace(*extra))
			{
				*scan++ = '\0';
				while (isspace(*extra))
				{
					extra++;
				}
				if (*extra)
				{
					nargs++;
				}
			}
			else if ((unsigned char)(*extra) == SPACEHOLDER)
			{
				*scan++ = ' ';
				extra++;
			}
			else
			{
				*scan++ = *extra++;
			}
		}
		*scan = '\0';

		/* reset argno to before the first, so :next will go to first */
		argno = -1;

		if (nargs != 1)
		{
                        msg("%d files to edit", nargs);
		}
	}
}

/*ARGSUSED*/
void cmd_cd(MARK frommark, MARK tomark, CMD cmd, int bang, char *extra)
{
#ifndef CRUNCH
	/* if current file is modified, and no '!' was given, then error */
	if (tstflag(file, MODIFIED) && !bang)
	{
		msg("File modified; use \"cd! %s\" to switch anyway", extra);
	}
#endif

	/* default directory name is $HOME */
	if (!*extra)
	{
		extra = gethome((char *)0);
		if (!extra)
		{
			msg("environment variable $HOME not set");
			return;
		}
	}

	/* go to the directory */
	if (chdir(extra) < 0)
	{
		perror(extra);
	}
}

/*ARGSUSED*/
void cmd_map(MARK frommark, MARK tomark, CMD cmd, int bang, char *extra)
{
	char	*mapto;
	char	*build, *scan;
#ifndef NO_FKEY
	static char *fnames[NFKEYS] =
	{
		"#10", "#1", "#2", "#3", "#4",
		"#5", "#6", "#7", "#8", "#9",
# ifndef NO_SHIFT_FKEY
		"#10s", "#1s", "#2s", "#3s", "#4s",
		"#5s", "#6s", "#7s", "#8s", "#9s",
#  ifndef NO_CTRL_FKEY
		"#10c", "#1c", "#2c", "#3c", "#4c",
		"#5c", "#6c", "#7c", "#8c", "#9c",
#   ifndef NO_ALT_FKEY
		"#10a", "#1a", "#2a", "#3a", "#4a",
		"#5a", "#6a", "#7a", "#8a", "#9a",
#   endif
#  endif
# endif
	};
	int	key;
#endif

	/* "map" with no extra will dump the map table contents */
	if (!*extra)
	{
#ifndef NO_ABBR
		if (cmd == CMD_ABBR)
		{
			dumpkey(bang ? WHEN_EX|WHEN_VIINP|WHEN_VIREP : WHEN_VIINP|WHEN_VIREP, TRUE);
		}
		else
#endif
		{
			dumpkey(bang ? WHEN_VIINP|WHEN_VIREP : WHEN_VICMD, FALSE);
		}
	}
	else
	{
		/* "extra" is key to map, followed by what it maps to */

		/* handle quoting inside the "raw" string */
		for (build = mapto = extra;
		     *mapto && (
#ifndef NO_ABBR
				cmd == CMD_UNABBR ||
#endif
				*mapto != ' ' && *mapto != '\t');
		     *build++ = *mapto++)
		{
			if (*mapto == ctrl('V') && mapto[1])
			{
				mapto++;
			}
		}

		/* skip whitespace, and mark the end of the "raw" string */
		while ((*mapto == ' ' || *mapto == '\t'))
		{
			*mapto++ = '\0';
		}
		*build = '\0';

		/* strip ^Vs from the "cooked" string */
		for (scan = build = mapto; *scan; *build++ = *scan++)
		{
			if (*scan == ctrl('V') && scan[1])
			{
				scan++;
			}
		}
		*build = '\0';

#ifndef NO_FKEY
		/* if the mapped string is '#' and a number, then assume
		 * the user wanted that function key
		 */
		if (extra[0] == '#' && isdigit(extra[1]))
		{
			key = atoi(extra + 1) % 10;
# ifndef NO_SHIFT_FKEY
			build = extra + strlen(extra) - 1;
			if (*build == 's')
				key += 10;
#  ifndef NO_CTRL_FKEY
			else if (*build == 'c')
				key += 20;
#   ifndef NO_ALT_FKEY
			else if (*build == 'a')
				key += 30;
#   endif
#  endif
# endif
			if (FKEY[key])
				mapkey(FKEY[key], mapto, bang ? WHEN_VIINP|WHEN_VIREP : WHEN_VICMD, fnames[key]);
			else
				msg("This terminal has no %s key", fnames[key]);
		}
		else
#endif
#ifndef NO_ABBR
		if (cmd == CMD_ABBR || cmd == CMD_UNABBR)
		{
			mapkey(extra, mapto, bang ? WHEN_EX|WHEN_VIINP|WHEN_VIREP : WHEN_VIINP|WHEN_VIREP, "abbr");
		}
		else
#endif
		{
			mapkey(extra, mapto, bang ? WHEN_VIINP|WHEN_VIREP : WHEN_VICMD, (char *)0);
		}
	}
}

/*ARGSUSED*/
void cmd_set(MARK frommark, MARK tomark, CMD cmd, int bang, char *extra)
{
	if (!*extra)
	{
		dumpopts(FALSE);/* "FALSE" means "don't dump all" - only set */
	}
	else if (!strcmp(extra, "all"))
	{
		dumpopts(TRUE);	/* "TRUE" means "dump all" - even unset vars */
	}
	else
	{
		setopts(extra);

		/* That option may have affected the appearence of text */
		changes++;
	}
}

/*ARGSUSED*/
void cmd_tag(MARK frommark, MARK tomark, CMD cmd, int bang, char *extra)
{
	int	fd;		/* file descriptor used to read the file */
	char	*scan;		/* used to scan through the tmpblk.c */
	char	search_string[256];	/* Search string used to locate tag */
	int	length;		/* Length of above string */
	char	*end;		/* marks the end of chars in tmpblk.c */
#ifndef NO_TAGSTACK
	char	*s;
#endif
	int	i;
#ifndef NO_MAGIC
	char	wasmagic; /* preserves the original state of o_magic */
#endif
	static char prevtag[128];

	/* if no tag is given, use the previous tag */
	if (!extra || !*extra)
	{
		if (!*prevtag)
		{
			msg("No previous tag");
			return;
		}
		extra = prevtag;
	}
	else
	{
		strncpy(prevtag, extra, sizeof prevtag);
		prevtag[sizeof prevtag - 1] = '\0';
	}

	/* use "ref" to look up the tag info for this tag */
	sprintf(tmpblk.c, "ref -t %s%s %s", (*origname ? "-f" : ""),origname, extra);
	fd = rpipe(tmpblk.c, 0);
	if (fd < 0)
	{
		msg("Can't run \"%s\"", tmpblk.c);
		return;
	}

	/* try to read the tag info */
	for (scan = tmpblk.c;
	     (i = tread(fd, scan, scan - tmpblk.c + BLKSIZE)) > 0;
	     scan += i)
	{
	}
	*scan = '\0';
	end = scan;

	/* close the pipe.  abort if error */
	if (rpclose(fd) != 0)
	{
		msg("Trouble running \"ref\" -- Can't do tag lookup");
		return;
	}
	else if (scan < tmpblk.c + 3)
	{
		msg("tag \"%s\" not found", extra);
		return;
	}

	/* extract the filename from the line, and edit the file */
	for (scan = tmpblk.c; *scan != '\t'; scan++)
	{
	}
	*scan++ = '\0';

	/* Save the search string, if possible.  This is weak code!  It is a
	temporary fix until the Elvis Gods can implement a proper fix.  This
	fix depends on the fact that there will be no subsequent use of the
	local variable "scan." */
	length = 0;
	while (scan < end && *scan != '\n' && length < sizeof(search_string))
		search_string[length++] = *scan++;
	if (scan >= end || length >= sizeof(search_string))
		length = 0;
	search_string[length] = '\0';

	if (strcmp(origname, tmpblk.c) != 0)
	{
		if (!tmpabort(bang))
		{
			msg("Use :tag! to abort changes, or :w to save changes");
			return;
		}
		/* This clobbers the search string, necessitating the above
		bug fix. */
		tmpstart(tmpblk.c);
#ifdef NO_TAGSTACK
	}
#else /* tagstack enabled */
		s = prevorig;
	}
	else
		s = origname;

	if (frommark != MARK_UNSET && *s && *o_tagstack)
	{
		curr_tag++;
		if (curr_tag >= MAXTAGS)
		{
			/* discard the oldest tag position */
			free(tag_stack[0].tag_file);
			for (curr_tag = 0; curr_tag < MAXTAGS - 1; curr_tag++)
			{
				tag_stack[curr_tag] = tag_stack[curr_tag + 1];
			}
			/* at this point, curr_tag = MAXTAGS-1 */
		}
		tag_stack[curr_tag].tag_file = (char *) malloc(strlen(s) + 1);
		strcpy(tag_stack[curr_tag].tag_file, s);
		tag_stack[curr_tag].tag_mark = frommark;
	}
#endif

	/* move to the desired line (or to line 1 if that fails) */
#ifndef NO_MAGIC
	wasmagic = *o_magic;
	*o_magic = FALSE;
#endif
	cursor = MARK_FIRST;
	linespec(search_string, &cursor);
	if (cursor == MARK_UNSET)
	{
		cursor = MARK_FIRST;
		msg("Tag's address is out of date");
	}
#ifndef NO_MAGIC
	*o_magic = wasmagic;
#endif
}

#ifndef NO_TAGSTACK
/*ARGSUSED*/
void cmd_pop(MARK frommark, MARK tomark, CMD cmd, int bang, char *extra)
{
	char	buf[8];

	if (!*o_tagstack)
	{
		msg("Tagstack not enabled");
		return;
	}

	if (curr_tag < 0)
		msg("Tagstack empty");
	else
	{
		if (strcmp(origname, tag_stack[curr_tag].tag_file) != 0)
		{
			if (!tmpabort(bang))
			{
				msg("Use :pop! to abort changes, or :w to save changes");
				return;
			}
			tmpstart(tag_stack[curr_tag].tag_file);
		}
		cursor = tag_stack[curr_tag].tag_mark;
		if (cursor < MARK_FIRST || cursor > MARK_LAST + BLKSIZE)
		{
			cursor = MARK_FIRST;
		}
		free(tag_stack[curr_tag--].tag_file);
	}
}
#endif

/* describe this version of the program */
/*ARGSUSED*/
void cmd_version(MARK frommark, MARK tomark, CMD cmd, int bang, char *extra)
{
	msg("%s", VERSION);
#ifdef CREDIT
	msg("%s", CREDIT);
#endif
#ifdef CREDIT2
	msg("%s", CREDIT2);
#endif
#ifdef COMPILED_BY
	msg("Compiled by %s", COMPILED_BY);
#endif
#ifdef COPYING
	msg("%s", COPYING);
#endif
}

/*ARGSUSED*/
void cmd_comment(MARK frommark, MARK tomark, CMD cmd, int bang, char *extra)
{
}

#ifndef NO_MKEXRC
/* make a .exrc file which describes the current configuration */
/*ARGSUSED*/
void cmd_mkexrc(MARK frommark, MARK tomark, CMD cmd, int bang, char *extra)
{
	int	fd;

	/* the default name for the .exrc file EXRC */
	if (!*extra)
	{
		extra = EXRC;
	}

	/* create the .exrc file */
	fd = creat(extra, FILEPERMS);
	if (fd < 0)
	{
		msg("Couldn't create a new \"%s\" file", extra);
		return;
	}

	/* save stuff */
	saveopts(fd);
	savemaps(fd, FALSE);
#ifndef NO_ABBR
	savemaps(fd, TRUE);
#endif
#ifndef NO_DIGRAPH
	savedigs(fd);
#endif
#ifndef NO_COLOR
	savecolor(fd);
#endif

	/* close the file */
	close(fd);
	msg("Configuration saved");
}
#endif

#ifndef NO_DIGRAPH
/*ARGSUSED*/
void cmd_digraph(MARK frommark, MARK tomark, CMD cmd, int bang, char *extra)
{
	do_digraph(bang, extra);
}
#endif

#ifndef NO_ERRLIST 
static char	errfile[256];	/* the name of a file containing an error */
static long	errline;	/* the line number for an error */
static int	errfd = -2;	/* fd of the errlist file */

/* This static function tries to parse an error message.
 *
 * For most compilers, the first word is taken to be the name of the erroneous
 * file, and the first number after that is taken to be the line number where
 * the error was detected.  The description of the error follows, possibly
 * preceded by an "error ... :" or "warning ... :" label which is skipped.
 *
 * [sdw]: (for HPUX) Use first word after first quotation mark as filename;
 *        then first number after that as line number.
 *
 * For Coherent, error messages look like "line#: filename: message".
 *
 * For non-error lines, or unparsable error lines, this function returns NULL.
 * Normally, though, it alters errfile and errline, and returns a pointer to
 * the description.
 */
static char *parse_errmsg(REG char *text)
{
	REG char	*cpy;
	char		*errmsg;

	/* the error message is the whole line, by default */
	errmsg = text;

	/* ignore the word "Error" or "Warning" at the start of the line. */
	if (text[0])
	{
		if (!strncmp(text+1, "rror", 4))
			text += 5;
		else if (!strncmp(text+1, "arning", 6))
			text += 7;
	}

#ifdef hpux
	/* skip to first quotation mark [sdw] */
	while (*text && *text != '"')
	{
		text++;
	}
#endif

	/* skip leading garbage */
	while (*text && !isalnum(*text))
	{
		text++;
	}

	/* copy over the filename */
	cpy = errfile;
	while(isalnum(*text) || *text == '.' || *text == '_')
	{
		*cpy++ = *text++;
	}
	*cpy = '\0';

	/* ignore the name "Error" and filenames that contain a '/' */
	if (*text == '/' || !*errfile || !strcmp(errfile + 1, "rror") || access(errfile, 0) < 0)
	{
		return (char *)0;
	}

	/* skip garbage between filename and line number */
	while (*text && !isdigit(*text))
	{
		text++;
	}

	/* if the number is part of a larger word, then ignore this line */
	if (*text && (isalpha(text[-1]) || text[-1] == '_'))
	{
		return (char *)0;
	}

	/* get the error line */
	errline = 0L;
	while (isdigit(*text))
	{
		errline *= 10;
		errline += (*text - '0');
		text++;
	}

	/* any line which lacks a filename or line number should be ignored */
	if (!errfile[0] || !errline)
	{
		return (char *)0;
	}

	/* locate the beginning of the error description */
	while (*text && !isspace(*text))
	{
		text++;
	}
	while (*text)
	{
#  ifndef CRUNCH
		/* skip "error #:" and "warning #:" clauses */
		if (!strncmp(text + 1, "rror ", 5)
		 || !strncmp(text + 1, "arning ", 7)
		 || !strncmp(text + 1, "atal error", 10))
		{
			do
			{
				text++;
			} while (*text && *text != ':');
			continue;
		}
#  endif

		/* anything other than whitespace or a colon is important */
		if (!isspace(*text) && *text != ':')
		{
			errmsg = text;
			break;
		}

		/* else keep looking... */
		text++;
	}

	return errmsg;
}

/*ARGSUSED*/
void cmd_errlist(MARK frommark, MARK tomark, CMD cmd, int bang, char *extra)
{
	static long	endline;/* original number of lines in this file */
	static long	offset;	/* offset of the next line in the errlist file */
	int		i;
	char		*errmsg;
	char		buf[300];

	/* if a new errlist file is named, open it */
	if (extra && extra[0])
	{
		/* close the old one */
		if (errfd >= 0)
		{
			close(errfd);
		}

		/* open the new one */
		errfd = open(extra, O_RDONLY);
		offset = 0L;
		endline = nlines;
	}
	else if (errfd < 0)
	{
		/* open the default file */
		errfd = open(ERRLIST, O_RDONLY);
		offset = 0L;
		endline = nlines;
	}

	/* do we have an errlist file now? */
	if (errfd < 0)
	{
		msg("There is no errlist file");
		beep();
		return;
	}

	/* find the next error message in the file */
	do
	{
		/* read the next line from the errlist */
		lseek(errfd, offset, 0);
		if (tread(errfd, buf, sizeof buf) <= 0)
		{
			msg("No more errors");
			beep();
			close(errfd);
			errfd = -2;
			return;
		}
		for (i = 0; buf[i] != '\n'; i++)
		{
		}
		buf[i++] = 0;

		/* look for an error message in the line */
		errmsg = parse_errmsg(buf);
		if (!errmsg)
		{
			offset += i;
		}

	} while (!errmsg);

	/* switch to the file containing the error, if this isn't it */
	if (strcmp(origname, errfile))
	{
		if (!tmpabort(bang))
		{
			msg("Use :er! to abort changes, or :w to save changes");
			beep();
			return;
		}
		tmpstart(errfile);
		endline = nlines;
	}
	else if (endline == 0L)
	{
		endline = nlines;
	}

	/* go to the line where the error was detected */
	cursor = MARK_AT_LINE(errline + (nlines - endline));
	if (cursor > MARK_LAST)
	{
		cursor = MARK_LAST;
	}
	if (mode == MODE_VI)
	{
		redraw(cursor, FALSE);
	}

	/* display the error message */
#ifdef CRUNCH
	msg("%.70s", errmsg);
#else
	if (nlines > endline)
	{
		msg("line %ld(+%ld): %.60s", errline, nlines - endline, errmsg);
	}
	else if (nlines < endline)
	{
		msg("line %ld(-%ld): %.60s", errline, endline - nlines, errmsg);
	}
	else
	{
		msg("line %ld: %.65s", errline, errmsg);
	}
#endif

	/* remember where the NEXT error line will start */
	offset += i;
}

/*ARGSUSED*/
void cmd_make(MARK frommark, MARK tomark, CMD cmd, int bang, char *extra)
{
	BLK	buf;

	/* if the file hasn't been saved, then complain unless ! */
	if (tstflag(file, MODIFIED) && !bang)
	{
		msg("\"%s\" not saved yet", origname);
		return;
	}

	/* build the command */
	sprintf(buf.c, "%s %s %s%s", (cmd == CMD_CC ? o_cc : o_make), extra, REDIRECT, ERRLIST);
	qaddstr(buf.c);
	addch('\n');

	/* close the old errlist file, if any */
	if (errfd >= 0)
	{
		close(errfd);
		errfd = -3;
	}

#if MINT
	/* I guess MiNT can't depend on the shell for redirection? */
	close(creat(ERRLIST, 0666));
	if ((fd = open(ERRLIST, O_RDWR)) == -1)
	{
		unlink(ERRLIST);
		return;
	}
	suspend_curses();
	old2 = dup(2);
	dup2(fd, 2);
	system(buf.c);
	dup2(old2, 2);
	close(old2);
	close(fd);
#else
	/* run the command, with curses temporarily disabled */
	suspend_curses();
	system(buf.c);
#endif
	resume_curses(mode == MODE_EX);
	if (mode == MODE_COLON)
		/* ':' hit instead of CR, so let him escape... -nox */
		return;

	/* run the "errlist" command */
	cmd_errlist(MARK_UNSET, MARK_UNSET, cmd, bang, ERRLIST);

	/* avoid spurious `Hit <RETURN>' after 1st error message  -nox */
	/* (which happened when cmd_errlist didn't have to change files...)  */
	if (mode == MODE_VI)
		refresh();
}
#endif

#ifndef NO_COLOR

/* figure out the number of text colors we use with this configuration */
# ifndef NO_POPUP
#  ifndef NO_VISIBLE
#   define NCOLORS 8
#  else
#   define NCOLORS 7
#  endif
# else
#  ifndef NO_VISIBLE
#   define NCOLORS 7
#  else
#   define NCOLORS 6
#  endif
# endif

/* the attribute bytes used in each of "when"s */
static char bytes[NCOLORS];

static struct
{
	char	*word;	/* a legal word */
	int	type;	/* what type of word this is */
	int	val;	/* some other value */
}
	words[] =
{
	{"normal",	1,	A_NORMAL},	/* all "when" names must come */
	{"standout",	1,	A_STANDOUT},	/* at the top of the list.    */
	{"bold",	1,	A_BOLD},	/* The first 4 must be normal,*/	
	{"quit",	1,	A_QUIT},	/* standout, bold, and quit;  */
	{"underlined",	1,	A_UNDERLINE},	/* the remaining names follow.*/
	{"italics",	1,	A_ALTCHARSET},
#ifndef NO_POPUP
	{"popup",	1,	A_POPUP},
#endif
#ifndef NO_VISIBLE
	{"visible",	1,	A_VISIBLE},
#endif

	{"black",	3,	0x00},		/* The color names start right*/
	{"blue",	3,	0x01},		/* after the "when" names.    */
	{"green",	3,	0x02},
	{"cyan",	3,	0x03},
	{"red",		3,	0x04},
	{"magenta",	3,	0x05},
	{"brown",	3,	0x06},
	{"white",	3,	0x07},
	{"yellow",	3,	0x0E}, /* bright brown */
	{"gray",	3,	0x08}, /* bright black?  of course! */
	{"grey",	3,	0x08},

	{"bright",	2,	0x08},
	{"light",	2,	0x08},
	{"blinking",	2,	0x80},
	{"on",		0,	0},
	{"n",		1,	A_NORMAL},
	{"s",		1,	A_STANDOUT},
	{"b",		1,	A_BOLD},
	{"u",		1,	A_UNDERLINE},
	{"i",		1,	A_ALTCHARSET},
	{"q",		1,	A_QUIT},
#ifndef NO_POPUP
	{"p",		1,	A_POPUP},
	{"menu",	1,	A_POPUP},
#endif
#ifndef NO_VISIBLE
	{"v",		1,	A_VISIBLE},
#endif
	{(char *)0,	0,	0}
};

/*ARGSUSED*/
void cmd_color(MARK frommark, MARK tomark, CMD cmd, int bang, char *extra)
{
	int	attrbyte;
	int	cmode;
	int	nowbg;	/* BOOLEAN: is the next color background? */

	REG char *scan;
	REG	int i;

#ifndef CRUNCH
	/* if no args are given, then report the current colors */
	if (!*extra)
	{
		/* if no colors are set, then say so */
		if (!bytes[0])
		{
			msg("no colors have been set");
			return;
		}

		/* report all color combinations */
		for (i = 0; i < NCOLORS; i++)
		{
			qaddstr("color ");
			qaddstr(words[i].word);
			qaddch(' ');
			if (bytes[i] & 0x80)
				qaddstr("blinking ");
			switch (bytes[i] & 0xf)
			{
			  case 0x08:	qaddstr("gray");	break;
			  case 0x0e:	qaddstr("yellow");	break;
			  case 0x0f:	qaddstr("bright white");break;
			  default:
				if (bytes[i] & 0x08)
					qaddstr("light ");
				qaddstr(words[(bytes[i] & 0x07) + NCOLORS].word);
			}
			qaddstr(" on ");
			qaddstr(words[((bytes[i] >> 4) & 0x07) + NCOLORS].word);
			addch('\n');
			exrefresh();
		}
		return;
	}
#endif

	/* The default background color is the same as "normal" chars.
	 * There is no default foreground color.
	 */
	cmode = A_NORMAL;
	attrbyte = bytes[0] & 0x70;
	nowbg = FALSE;

	/* parse each word in the "extra" text */
	for (scan = extra; *extra; extra = scan)
	{
		/* locate the end of the word */
		while (*scan && *scan != ' ')
		{
			scan++;
		}

		/* skip whitespace at the end of the word */
		while(*scan == ' ')
		{
			*scan++ = '\0';
		}

		/* lookup the word */
		for (i = 0; words[i].word && strcmp(words[i].word, extra); i++)
		{
		}

		/* if not a word, then complain */
		if (!words[i].word)
		{
			msg("Invalid color name: %s", extra);
			return;
		}

		/* process the word */
		switch (words[i].type)
		{
		  case 1:
			cmode = words[i].val;
			break;

		  case 2:
			attrbyte |= words[i].val;
			break;

		  case 3:
			if (nowbg)
				attrbyte = ((attrbyte & ~0x70) | ((words[i].val & 0x07) << 4));
			else
				attrbyte |= words[i].val;
			nowbg = TRUE;
			break;
		}
	}

	/* if nowbg isn't set now, then we were never given a foreground color */
	if (!nowbg)
	{
		msg("usage: color [when] [\"bright\"] [\"blinking\"] foreground [background]");
		return;
	}

	/* the first ":color" command MUST define the "normal" colors */
	if (!bytes[0])
		cmode = A_NORMAL;

	/* we should now have a cmode and an attribute byte... */

	/* set the color */
	setcolor(cmode, attrbyte);

	/* remember what we just did */
	bytes[cmode] = attrbyte;

	/* if the other colors haven't been set yet, then set them to defaults */
	if (!bytes[1])
	{
		/* standout is the opposite of normal */
		bytes[1] = ((attrbyte << 4) & 0x70 | (attrbyte >> 4) & 0x07);
		setcolor(A_STANDOUT, bytes[1]);

		/* if "normal" isn't bright, then bold defaults to normal+bright
		 * else bold defaults to bright white.
		 */
		bytes[2] = attrbyte | ((attrbyte & 0x08) ? 0x0f : 0x08);
		setcolor(A_BOLD, bytes[2]);

		/* quit colors default to white on black */
		bytes[3] = 0x07;
		setcolor(A_QUIT, bytes[3]);

		/* all others default to the "standout" colors, without blinking */
		for (i = 4; i < NCOLORS; i++)
		{
			bytes[i] = (bytes[1] & 0x7f);
			setcolor(words[i].val, bytes[i]);
		}
	}

	/* force a redraw, so we see the new colors */
	redraw(MARK_UNSET, FALSE);
}

void savecolor(int fd)
{
	int	i;
	char	buf[80];

	/* if no colors are set, then return */
	if (!bytes[0])
	{
		return;
	}

	/* save all five color combinations */
	for (i = 0; i < NCOLORS; i++)
	{
		strcpy(buf, "color ");
		strcat(buf, words[i].word);
		strcat(buf, " ");
		if (bytes[i] & 0x80)
			strcat(buf, "blinking ");
		switch (bytes[i] & 0xf)
		{
		  case 0x08:	strcat(buf, "gray");	break;
		  case 0x0e:	strcat(buf, "yellow");	break;
		  case 0x0f:	strcat(buf, "bright white");break;
		  default:
			if (bytes[i] & 0x08)
				strcat(buf, "light ");
			strcat(buf, words[(bytes[i] & 0x07) + NCOLORS].word);
		}
		strcat(buf, " on ");
		strcat(buf, words[((bytes[i] >> 4) & 0x07) + NCOLORS].word);
		strcat(buf, "\n");
		twrite(fd, buf, (unsigned)strlen(buf));
	}
}
#endif

#ifdef SIGTSTP
/* temporarily suspend elvis */
/*ARGSUSED*/
void cmd_suspend(MARK frommark, MARK tomark, CMD cmd, int bang, char *extra)
{
	SIGTYPE	(*func)();	/* stores the previous setting of SIGTSTP */

	/* the Bourne shell can't handle ^Z, but BASH can. */
	if (!strcmp(o_shell, "/bin/sh") && !getenv("BASH_VERSION") && !getenv("BASH"))
	{
		msg("The /bin/sh shell doesn't support ^Z");
		return;
	}

	move(LINES - 1, 0);
	if (tstflag(file, MODIFIED))
	{
		if (!*o_autowrite || !tmpsave(origname, *o_writeany))
		{
			addstr("Warning: \"");
			addstr(origname);
			addstr("\" modified but not yet saved");
			clrtoeol();
		}
	}
	refresh();
	suspend_curses();
	func = signal(SIGTSTP, SIG_DFL);
	kill (0, SIGTSTP);

	/* the process stops and resumes here */

	signal(SIGTSTP, func);
	resume_curses(TRUE);
	if (mode == MODE_VI || mode == MODE_COLON)
		redraw(MARK_UNSET, FALSE);
	else
		refresh ();
}
#endif
