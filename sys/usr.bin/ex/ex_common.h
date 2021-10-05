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

/* vi.h */

#define VERSION "sivle 1.9.2, based on elvis 1.8pl4 (" __DATE__ ")"
#define COPYING	"See the file COPYING for distribution terms."

#include <errno.h>
#include <sys/types.h>
#include <fcntl.h>

#ifndef O_BINARY
# define O_BINARY	0
#endif

#include "ex_curses.h"

#include <signal.h>
#include <stdio.h>	/* for [v]sprintf prototype		*/
#include <string.h>	/* for str* prototypes			*/
#include <stdlib.h>	/* for atoi, system, malloc, free	*/
#include <stdarg.h>	/* for vararg definitions		*/
#include <unistd.h>	/* for read, write, ... prototypes      */
#include <sys/wait.h>/* for wait prototype                   */

/*------------------------------------------------------------------------*/
/* Miscellaneous constants.						  */

#define INFINITY	2000000001L	/* a very large integer */
#define LONGKEY		10		/* longest possible raw :map key */
#ifndef MAXRCLEN
# define MAXRCLEN	1000		/* longest possible :@ command */
#endif

/*------------------------------------------------------------------------*/
/* These describe how temporary files are divided into blocks             */

#define MAXBLKS	(BLKSIZE / sizeof(unsigned short))
typedef union
{
	char		c[BLKSIZE];	/* for text blocks */
	unsigned short	n[MAXBLKS];	/* for the header block */
}
	BLK;

/*------------------------------------------------------------------------*/
/* These are used manipulate BLK buffers.                                 */

extern BLK	hdr;		/* buffer for the header block */
extern BLK *blkget (int);	/* given index into hdr.c[], reads block */
extern BLK *blkadd (int);	/* inserts a new block into hdr.c[] */

/*------------------------------------------------------------------------*/
/* These are used to keep track of various flags                          */
extern struct _viflags
{
	short	file;		/* file flags */
}
	viflags;

/* file flags */
#define NEWFILE		0x0001	/* the file was just created */
#define READONLY	0x0002	/* the file is read-only */
#define HADNUL		0x0004	/* the file contained NUL characters */
#define MODIFIED	0x0008	/* the file has been modified, but not saved */
#define ADDEDNL		0x0010	/* newlines were added to the file */
#define HADBS		0x0020	/* backspace chars were lost from the file */
#define UNDOABLE	0x0040	/* file has been modified */
#define NOTEDITED	0x0080	/* the :file command has been used */

/* macros used to set/clear/test flags */
#define setflag(x,y)	viflags.x |= y
#define clrflag(x,y)	viflags.x &= ~y
#define tstflag(x,y)	(viflags.x & y)
#define initflags()	viflags.file = 0;

/* The options */
extern char	o_autoindent[1];
extern char	o_autoprint[1];
extern char	o_autotab[1];
extern char	o_autowrite[1];
extern char	o_columns[3];
extern char	o_directory[30];
extern char	o_edcompatible[1];
extern char	o_equalprg[80];
extern char	o_errorbells[1];
extern char	o_exrefresh[1];
extern char	o_ignorecase[1];
extern char	o_keytime[3];
extern char	o_keywordprg[80];
extern char	o_lines[3];
extern char	o_list[1];
extern char	o_number[1];
extern char	o_readonly[1];
extern char	o_remap[1];
extern char	o_report[3];
extern char	o_scroll[3];
extern char	o_shell[60];
extern char	o_shiftwidth[3];
extern char	o_sidescroll[3];
extern char	o_sync[1];
extern char	o_tabstop[3];
extern char	o_term[30];
extern char	o_flash[1];
extern char	o_warn[1];
extern char	o_wrapscan[1];

#ifndef CRUNCH
extern char	o_beautify[1];
extern char	o_exrc[1];
extern char	o_mesg[1];
extern char	o_more[1];
extern char	o_nearscroll[3];
extern char	o_newfile[1];
extern char	o_novice[1];
extern char	o_optimize[1];
extern char	o_prompt[1];
extern char	o_taglength[3];
extern char	o_tags[256];
extern char	o_terse[1];
extern char	o_window[3];
extern char	o_wrapmargin[3];
extern char	o_writeany[1];
#endif

#ifndef NO_ERRLIST
extern char	o_cc[30];
extern char	o_make[30];
#endif

#ifndef NO_CHARATTR
extern char	o_charattr[1];
#endif

#ifndef NO_DIGRAPH
extern char	o_digraph[1];
extern char	o_flipcase[80];
#endif

#ifndef NO_SENTENCE
extern char	o_hideformat[1];
#endif

#ifndef NO_EXTENSIONS
extern char	o_inputmode[1];
extern char	o_keepanon[1];
extern char	o_ruler[1];
#endif

#ifndef NO_MAGIC
extern char	o_magic[1];
#endif

#ifndef NO_MODELINES
extern char	o_modelines[1];
#endif

#ifndef NO_SENTENCE
extern char	o_paragraphs[30];
extern char	o_sections[30];
#endif

#ifndef NO_SHOWMATCH
extern char	o_showmatch[1];
#endif

#ifndef	NO_SHOWMODE
extern char	o_smd[1];
#endif

#ifndef NO_TAGSTACK
extern char	o_tagstack[1];
#endif

#ifndef NO_SAFER
extern char	o_safer[1];
#endif

#ifdef DEBUG
extern char	o_slowmacro[1];
#endif

/*------------------------------------------------------------------------*/
/* These help support the single-line multi-change "undo" -- shift-U      */

extern char	U_text[BLKSIZE];
extern long	U_line;

/*------------------------------------------------------------------------*/
/* These are used to refer to places in the text 			  */

typedef long	MARK;
#define markline(x)	(long)((x) / BLKSIZE)
#define markidx(x)	(int)((x) & (BLKSIZE - 1))
#define MARK_UNSET	((MARK)0)
#define MARK_FIRST	((MARK)BLKSIZE)
#define MARK_LAST	((MARK)(nlines * BLKSIZE))
#define MARK_EOF	((MARK)((nlines + 1) * BLKSIZE - 1L))	/* -g.t. */
#define MARK_AT_LINE(x)	((MARK)(x) * BLKSIZE)

#define NMARKS	29
extern MARK	mark[NMARKS];	/* marks a-z, plus mark ' and two temps */
extern MARK	cursor;		/* mark where line is */

/*------------------------------------------------------------------------*/
/* These are used to keep track of the current & previous files.	  */

extern long	origtime;	/* modification date&time of the current file */
extern char	origname[256];	/* name of the current file */
extern char	prevorig[256];	/* name of the preceding file */
extern long	prevline;	/* line number from preceding file */

/*------------------------------------------------------------------------*/
/* misc housekeeping variables & functions				  */

extern int	tmpfd;				/* fd used to access the tmp file */
extern int	tmpnum;				/* counter used to generate unique filenames */
extern long	lnum[MAXBLKS];			/* last line# of each block */
extern long	nlines;				/* number of lines in the file */
extern char	args[BLKSIZE];			/* file names given on the command line */
extern int	argno;				/* the current element of args[] */
extern int	nargs;				/* number of filenames in args */
extern long	changes;			/* counts changes, to prohibit short-cuts */
extern int	significant;			/* boolean: was a *REAL* change made? */
extern int	exitcode;			/* 0=not updated, 1=overwritten, else error */
extern BLK	tmpblk;				/* a block used to accumulate changes */
extern long	topline;			/* file line number of top line */
extern int	leftcol;			/* column number of left col */
#define		botline	 (topline + LINES - 2)
#define		rightcol (leftcol + COLS - (*o_number ? 9 : 1))
extern int	physcol;			/* physical column number that cursor is on */
extern int	physrow;			/* physical row number that cursor is on */
extern int	exwrote;			/* used to detect verbose ex commands */
extern int	doingdot;			/* boolean: are we doing the "." command? */
extern int	doingglobal;			/* boolean: are doing a ":g" command? */
extern long	rptlines;			/* number of lines affected by a command */
extern char	*rptlabel;			/* description of how lines were affected */
extern char	*fetchline (long);		/* read a given line from tmp file */
extern char	*parseptrn (REG char *);	/* isolate a regexp in a line */
extern MARK	paste (MARK, int, int);	/* paste from cut buffer to a given point */
extern char	*wildcard (char *);		/* expand wildcards in filenames */
extern MARK	input (MARK, MARK, int, int);	/* inserts characters from keyboard */
extern char	*linespec (REG char *, MARK *);	/* finds the end of a /regexp/ string */
#define		ctrl(ch) ((ch)&037)
#ifndef NO_RECYCLE
extern long	allocate (void);		/* allocate a free block of the tmp file */
#endif
extern SIGTYPE	trapint (int);		/* trap handler for SIGINT */
extern SIGTYPE	deathtrap (int);		/* trap handler for deadly signals */
extern void	blkdirty (BLK *);		/* marks a block as being "dirty" */
extern void	blksync (void);		/* forces all "dirty" blocks to disk */
extern void	blkinit (void);		/* resets the block cache to "empty" state */
extern void	beep (void);		/* rings the terminal's bell */
extern void	exrefresh (void);		/* writes text to the screen */
extern void	msg (char *, ...);		/* writes a printf-style message to the screen */
extern void	endmsgs (void);		/* if "manymsgs" is set, then scroll up 1 line */
extern void	garbage (void);		/* reclaims any garbage blocks */
extern void	redraw (MARK, int);		/* updates the screen after a change */
extern void	resume_curses (int);	/* puts the terminal in "cbreak" mode */
extern void	beforedo (int);		/* saves current revision before a new change */
extern void	afterdo (void);		/* marks end of a beforedo() change */
extern void	abortdo (void);		/* like "afterdo()" followed by "undo()" */
extern int	undo (void);		/* restores file to previous undo() */
extern void	dumpkey (int, int);		/* lists key mappings to the screen */
extern void	mapkey (char *, char *, int, char *);	/* defines a new key mapping */
extern void	redrawrange (long, long, long);	/* records clues from modify.c */
#ifndef NO_LEARN
extern void	learnkey (char);		/* adds keystroke to learn buffer */
extern char	learn;				/* name of buffer being learned */
#endif
extern void	cut (MARK, MARK);		/* saves text in a cut buffer */
extern void	delete (MARK, MARK);	/* deletes text */
extern void	add (MARK, char *);		/* adds text */
extern void	change (MARK, MARK, char *);/* deletes text, and then adds other text */
extern void	cutswitch (void);		/* updates cut buffers when we switch files */
extern void	do_digraph (int, char []);	/* defines or lists digraphs */
extern void	exstring (char *, int, int);/* execute a string as EX commands */
extern void	dumpopts (int);		/* display current option settings on the screen */
extern void	setopts (char *);		/* assign new values to options */
extern void	saveopts (int);		/* save current option values to a given fd */
extern void	savedigs (int);		/* save current non-standard digraphs to fd */
extern void	savecolor (int);		/* save current color settings (if any) to fd */
extern void	cutname (int);		/* select cut buffer for next cut/paste */
extern void	initopts (void);		/* initialize options */
extern void	cutend (void);		/* free all cut buffers & delete temp files */
extern int	storename (char *);		/* stamp temp file with pathname of text file */
extern int	tmpstart (char *);		/* load a text file into edit buffer */
extern int	tmpsave (char *, int);	/* write edit buffer out to text file */
extern int	tmpend (int);		/* call tmpsave(), and then tmpabort */
extern int	tmpabort (int);		/* abandon the current edit buffer */
extern void	savemaps (int, int);	/* write current :map or :ab commands to fd */
extern int	ansicolor (int, int);	/* emit ANSI color command to terminal */
extern int	filter (MARK, MARK, char *, int); /* I/O though another program */
extern int	getkey (int);		/* return a keystroke, interpretting maps */
extern int	vgets (int, char *, int);	/* read a single line from keyboard */
extern int	doexrc (char *);		/* execute a string as a sequence of EX commands */
extern int	cb2str (int, char *, unsigned);/* return a string containing cut buffer's contents */
extern int	ansiquit (void);		/* neutralize previous ansicolor() call */
extern int	ttyread (char *, int, int);	/* read from keyboard with optional timeout */
extern int	tgetent (char *, char *);	/* start termcap */
extern int	tgetnum (char *);		/* get a termcap number */
extern int	tgetflag (char *);		/* get a termcap boolean */
extern int	endcolor (void);		/* used during color output */
extern int	getabkey (int, char *, int);/* like getkey(), but also does abbreviations */
extern int	idx2col (MARK, REG char *, int); /* returns column# of a given MARK */
extern int	cutneeds (BLK *);		/* returns bitmap of blocks needed to hold cutbuffer text */
extern void	execmap (int, char *, int);	/* replaces "raw" keys with "mapped" keys */

/*------------------------------------------------------------------------*/
/* macros that are used as control structures                             */

#define BeforeAfter(before, after) for((before),bavar=1;bavar;(after),bavar=0)
#define ChangeText	BeforeAfter(beforedo(FALSE),afterdo())

extern int	bavar;		/* used only in BeforeAfter macros */

/*------------------------------------------------------------------------*/
/* These are the movement commands.  Each accepts a mark for the starting */
/* location & number and returns a mark for the destination.		  */

extern MARK	m_updnto (MARK, long, int);		/* k j G */
extern MARK	m_right (MARK, long, int, int);	/* h */
extern MARK	m_left (MARK, long);		/* l */
extern MARK	m_tocol (MARK, long, int);		/* | */
extern MARK	m_front (MARK, long);		/* ^ */
extern MARK	m_rear (MARK, long);		/* $ */
extern MARK	m_fword (MARK, long, int, int);	/* w */
extern MARK	m_bword (MARK, long, int);		/* b */
extern MARK	m_eword (MARK, long, int);		/* e */
extern MARK	m_paragraph (MARK, long, int);	/* { } [[ ]] */
extern MARK	m_match (MARK, long);		/* % */
#ifndef NO_SENTENCE
extern MARK	m_sentence (MARK, long, int);	/* ( ) */
#endif
extern MARK	m_tomark (MARK, long, int);		/* 'm */
#ifndef NO_EXTENSIONS
extern MARK	m_wsrch (char *, MARK, int);	/* ^A */
#endif
extern MARK	m_nsrch (MARK, long, int);		/* n */
extern MARK	m_fsrch (MARK, char *);		/* /regexp */
extern MARK	m_bsrch (MARK, char *);		/* ?regexp */
#ifndef NO_CHARSEARCH
extern MARK	m__ch (MARK, long, int);		/* ; , */
extern MARK	m_fch (MARK, long, int);		/* f */
extern MARK	m_tch (MARK, long, int);		/* t */
extern MARK	m_Fch (MARK, long, int);		/* F */
extern MARK	m_Tch (MARK, long, int);		/* T */
#endif
extern MARK	m_row (MARK, long, int);		/* H L M */
extern MARK	m_z (MARK, long, int);		/* z */
extern MARK	m_scroll (MARK, long, int);		/* ^B ^F ^E ^Y ^U ^D */

/* Some stuff that is used by movement functions... */

extern MARK	adjmove (MARK, REG MARK, int);	/* a helper fn, used by move fns */
extern char	*get_cursor_word (MARK);		/* returns word at cursor */

/* This macro is used to set the default value of cnt */
#define DEFAULT(val)	if (cnt < 1) cnt = (val)

/* These are used to minimize calls to fetchline() */
extern int	plen;	/* length of the line */
extern long	pline;	/* line number that len refers to */
extern long	pchgs;	/* "changes" level that len refers to */
extern char	*ptext;	/* text of previous line, if valid */
extern void	pfetch (long);
extern char	digraph (int, int);

/* This is used to build a MARK that corresponds to a specific point in the
 * line that was most recently pfetch'ed.
 */
#define buildmark(text)	(MARK)(BLKSIZE * pline + (int)((text) - ptext))

/*------------------------------------------------------------------------*/
/* These are used to handle EX commands.				  */

#define CMD_NULL	0	/* NOT A VALID COMMAND */
#define CMD_ABBR	1	/* "define an abbreviation" */
#define CMD_AND		2	/* "logical-AND the test condition with this one" */
#define CMD_APPEND	3	/* "insert lines after this line" */
#define CMD_ARGS	4	/* "show me the args" */
#define CMD_AT		5	/* "execute a cut buffer's contents via EX" */
#define CMD_BANG	6	/* "run a single shell command" */
#define CMD_CC		7	/* "run `cc` and then do CMD_ERRLIST" */
#define CMD_CD		8	/* "change directories" */
#define CMD_CHANGE	9	/* "change some lines" */
#define CMD_COLOR	10	/* "change the default colors" */
#define CMD_COMMENT	11	/* "ignore the rest of this command line" */
#define CMD_COPY	12	/* "copy the selected text to a given place" */
#define CMD_DEBUG	13	/* access to internal data structures */
#define CMD_DELETE	14	/* "delete the selected text" */
#define CMD_DIGRAPH	15	/* "add a digraph, or display them all" */
#define CMD_EDIT	16	/* "switch to a different file" */
#define CMD_ELSE	17	/* "execute these commands if test condition is false */
#define CMD_EQUAL	18	/* "display a line number" */
#define CMD_ERRLIST	19	/* "locate the next error in a list" */
#define CMD_FILE	20	/* "show the file's status" */
#define CMD_GLOBAL	21	/* "globally search & do a command" */
#define CMD_IF		22	/* "set the test condition */
#define CMD_INSERT	23	/* "insert lines before the current line" */
#define CMD_JOIN	24	/* "join the selected line & the one after" */
#define CMD_LIST	25	/* "print lines, making control chars visible" */
#define CMD_MAKE	26	/* "run `make` and then do CMD_ERRLIST" */
#define CMD_MAP		27	/* "adjust the keyboard map" */
#define CMD_MARK	28	/* "mark this line" */
#define CMD_MKEXRC	29	/* "make a .exrc file" */
#define CMD_MOVE	30	/* "move the selected text to a given place" */
#define CMD_NEXT	31	/* "switch to next file in args" */
#define CMD_NUMBER	32	/* "print lines from the file w/ line numbers" */
#define CMD_OR		33	/* "logical-OR the test condition with this one" */
#define CMD_POP		34	/* "pop a position off the tagstack" */
#define CMD_PRESERVE	35	/* "act as though vi crashed" */
#define CMD_PREVIOUS	36	/* "switch to the previous file in args" */
#define CMD_PRINT	37	/* "print the selected text" */
#define CMD_PUT		38	/* "insert any cut lines before this line" */
#define CMD_QUIT	39	/* "quit without writing the file" */
#define CMD_READ	40	/* "append the given file after this line */
#define CMD_RECOVER	41	/* "recover file after vi crashes" - USE -r FLAG */
#define CMD_REWIND	42	/* "rewind to first file" */
#define CMD_SET		43	/* "set a variable's value" */
#define CMD_SHELL	44	/* "run some lines through a command" */
#define CMD_SHIFTL	45	/* "shift lines left" */
#define CMD_SHIFTR	46	/* "shift lines right" */
#define CMD_SOURCE	47	/* "interpret a file's contents as ex commands" */
#define CMD_STOP	48	/* same as CMD_SUSPEND */
#define CMD_SUBAGAIN	49	/* "repeat the previous substitution" */
#define CMD_SUBSTITUTE	50	/* "substitute text in this line" */
#define CMD_SUSPEND	51	/* "suspend the vi session" */
#define CMD_TAG		52	/* "go to a particular tag" */
#define CMD_THEN	53	/* "execute commands if previous test was true" */
#define CMD_TR		54	/* "transliterate chars in the selected lines" */
#define CMD_UNABBR	55	/* "remove an abbreviation definition" */
#define CMD_UNDO	56	/* "undo the previous command" */
#define CMD_UNMAP	57	/* "remove a key sequence map */
#define CMD_VALIDATE	58	/* check for internal consistency */
#define CMD_VERSION	59	/* "describe which version this is" */
#define CMD_VGLOBAL	60	/* "apply a cmd to lines NOT containing an RE" */
#define CMD_VISUAL	61	/* "go into visual mode" */
#define CMD_WQUIT	62	/* "write this file out (any case) & quit" */
#define CMD_WRITE	63	/* "write the selected(?) text to a given file" */
#define CMD_XIT		64	/* "write this file out (if modified) & quit" */
#define CMD_YANK	65	/* "copy the selected text into the cut buffer" */
typedef int CMD;

extern void	ex (void);
extern void	vi (void);
extern int	doexcmd (char *, int);

extern void	cmd_append (MARK, MARK, CMD, int, char *);
extern void	cmd_args (MARK, MARK, CMD, int, char *);
#ifndef NO_AT
 extern void	cmd_at (MARK, MARK, CMD, int, char *);
#endif
extern void	cmd_cd (MARK, MARK, CMD, int, char *);
#ifndef NO_COLOR
 extern void	cmd_color (MARK, MARK, CMD, int, char *);
#endif
extern void	cmd_comment (MARK, MARK, CMD, int, char *);
extern void	cmd_delete (MARK, MARK, CMD, int, char *);
#ifndef NO_DIGRAPH
 extern void	cmd_digraph (MARK, MARK, CMD, int, char *);
#endif
extern void	cmd_edit (MARK, MARK, CMD, int, char *);
#ifndef NO_ERRLIST
 extern void	cmd_errlist (MARK, MARK, CMD, int, char *);
#endif
extern void	cmd_file (MARK, MARK, CMD, int, char *);
extern void	cmd_global (MARK, MARK, CMD, int, char *);
#ifndef NO_IF
extern void	cmd_if (MARK, MARK, CMD, int, char *);
#endif
extern void	cmd_join (MARK, MARK, CMD, int, char *);
extern void	cmd_mark (MARK, MARK, CMD, int, char *);
#ifndef NO_ERRLIST
 extern void	cmd_make (MARK, MARK, CMD, int, char *);
#endif
extern void	cmd_map (MARK, MARK, CMD, int, char *);
#ifndef NO_MKEXRC
 extern void	cmd_mkexrc (MARK, MARK, CMD, int, char *);
#endif
extern void	cmd_next (MARK, MARK, CMD, int, char *);
#ifndef NO_TAGSTACK
extern void	cmd_pop (MARK, MARK, CMD, int, char *);
#endif
extern void	cmd_print (MARK, MARK, CMD, int, char *);
extern void	cmd_put (MARK, MARK, CMD, int, char *);
extern void	cmd_read (MARK, MARK, CMD, int, char *);
extern void	cmd_set (MARK, MARK, CMD, int, char *);
extern void	cmd_shell (MARK, MARK, CMD, int, char *);
extern void	cmd_shift (MARK, MARK, CMD, int, char *);
extern void	cmd_source (MARK, MARK, CMD, int, char *);
extern void	cmd_substitute (MARK, MARK, CMD, int, char *);
extern void	cmd_tag (MARK, MARK, CMD, int, char *);
#ifndef NO_IF
extern void	cmd_then (MARK, MARK, CMD, int, char *);
#endif
extern void	cmd_undo (MARK, MARK, CMD, int, char *);
extern void	cmd_version (MARK, MARK, CMD, int, char *);
extern void	cmd_write (MARK, MARK, CMD, int, char *);
extern void	cmd_xit (MARK, MARK, CMD, int, char *);
extern void	cmd_move (MARK, MARK, CMD, int, char *);
#ifdef DEBUG
 extern void	cmd_debug (MARK, MARK, CMD, int, char *);
 extern void	cmd_validate (MARK, MARK, CMD, int, char *);
#endif
#ifdef SIGTSTP
 extern void	cmd_suspend (MARK, MARK, CMD, int, char *);
#endif

/*----------------------------------------------------------------------*/
/* These are used to handle VI commands 				*/

extern MARK	v_1ex (MARK, char *);	/* : */
extern MARK	v_mark (MARK, long, int);	/* m */
extern MARK	v_quit (void);		/* Q */
extern MARK	v_redraw (void);		/* ^L ^R */
extern MARK	v_ulcase (MARK, long);	/* ~ */
extern MARK	v_undo (MARK);		/* u */
extern MARK	v_xchar (MARK, long, int);	/* x X */
extern MARK	v_replace (MARK, long, int);/* r */
extern MARK	v_overtype (MARK);		/* R */
extern MARK	v_selcut (MARK, long, int);	/* " */
extern MARK	v_paste (MARK, long, int);	/* p P */
extern MARK	v_yank (MARK, MARK);	/* y Y */
extern MARK	v_delete (MARK, MARK);	/* d D */
extern MARK	v_join (MARK, long);	/* J */
extern MARK	v_insert (MARK, long, int);	/* a A i I o O */
extern MARK	v_change (MARK, MARK);	/* c C */
extern MARK	v_subst (MARK, long);	/* s */
extern MARK	v_lshift (MARK, MARK);	/* < */
extern MARK	v_rshift (MARK, MARK);	/* > */
extern MARK	v_reformat (MARK, MARK);	/* = */
extern MARK	v_filter (MARK, MARK);	/* ! */
extern MARK	v_status (void);		/* ^G */
extern MARK	v_switch (void);		/* ^^ */
extern MARK	v_tag (char *, MARK, long);	/* ^] */
extern MARK	v_xit (MARK, long, int);	/* ZZ */
extern MARK	v_undoline (MARK);		/* U */
extern MARK	v_again (MARK, MARK);	/* & */
#ifndef NO_EXTENSIONS
extern MARK	v_keyword (char *, MARK, long);	/* K */
extern MARK	v_increment (char *, MARK, long);	/* # */
#endif
#ifndef NO_ERRLIST
extern MARK	v_errlist (MARK);		/* * */
#endif
#ifndef NO_AT
extern MARK	v_at (MARK, long, int);	/* @ */
#endif
#ifdef SIGTSTP
extern MARK	v_suspend (void);		/* ^Z */
#endif
#ifndef NO_POPUP
extern MARK	v_popup (MARK, MARK);	/* \ */
#endif
#ifndef NO_TAGSTACK
extern MARK	v_pop (MARK, long, int);	/* ^T */
#endif

/*----------------------------------------------------------------------*/
/* These flags describe the quirks of the individual visual commands */
#define NO_FLAGS	0x00
#define	MVMT		0x01	/* this is a movement command */
#define PTMV		0x02	/* this can be *part* of a movement command */
#define FRNT		0x04	/* after move, go to front of line */
#define INCL		0x08	/* include last char when used with c/d/y */
#define LNMD		0x10	/* use line mode of c/d/y */
#define NCOL		0x20	/* this command can't change the column# */
#define NREL		0x40	/* this is "non-relative" -- set the '' mark */
#define SDOT		0x80	/* set the "dot" variables, for the "." cmd */
#define FINL		0x100	/* final testing, more strict! */
#define NWRP		0x200	/* no line-wrap (used for 'w' and 'W') */
#define INPM		0x400	/* input mode -- cursor can be after EOL */
#ifndef NO_VISIBLE
# define VIZ		0x800	/* commands which can be used with 'v' */
#else
# define VIZ		0
#endif

/* This variable is zeroed before a command executes, and later ORed with the
 * command's flags after the command has been executed.  It is used to force
 * certain flags to be TRUE for *some* invocations of a particular command.
 * For example, "/regexp/+offset" forces the LNMD flag, and sometimes a "p"
 * or "P" command will force FRNT.
 */
extern int	force_flags;

/*----------------------------------------------------------------------*/
/* These describe what mode we're in */

#define MODE_EX		1	/* executing ex commands */
#define	MODE_VI		2	/* executing vi commands */
#define	MODE_COLON	3	/* executing an ex command from vi mode */
#define	MODE_QUIT	4
extern int	mode;

#define WHEN_VICMD	1	/* getkey: we're reading a VI command */
#define WHEN_VIINP	2	/* getkey: we're in VI's INPUT mode */
#define WHEN_VIREP	4	/* getkey: we're in VI's REPLACE mode */
#define WHEN_EX		8	/* getkey: we're in EX mode */
#define WHEN_MSG	16	/* getkey: we're at a "more" prompt */
#define WHEN_POPUP	32	/* getkey: we're in the pop-up menu */
#define WHEN_REP1	64	/* getkey: we're getting a single char for 'r' */
#define WHEN_CUT	128	/* getkey: we're getting a cut buffer name */
#define WHEN_MARK	256	/* getkey: we're getting a mark name */
#define WHEN_CHAR	512	/* getkey: we're getting a destination for f/F/t/T */
#define WHEN_INMV	4096	/* in input mode, interpret the key in VICMD mode */
#define WHEN_FREE	8192	/* free the keymap after doing it once */
#define WHENMASK	(WHEN_VICMD|WHEN_VIINP|WHEN_VIREP|WHEN_REP1|WHEN_CUT|WHEN_MARK|WHEN_CHAR)

#ifndef NO_VISIBLE
extern MARK	V_from;
extern int	V_linemd;
extern MARK v_start (MARK m, long cnt, int cmd);
#endif

#ifdef DEBUG
# define malloc(size)	dbmalloc(size, __FILE__, __LINE__)
# define free(ptr)	dbfree(ptr, __FILE__, __LINE__)
# define checkmem()	dbcheckmem(__FILE__, __LINE__)
extern char	*dbmalloc (int, char *, int);
#else
# define checkmem()
#endif
