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

/* curses.c */

/* This file contains the functions & variables needed for a tiny subset of
 * curses.  The principle advantage of this version of curses is its
 * extreme speed.  Disadvantages are potentially larger code, few supported
 * functions, limited compatibility with full curses, and only stdscr.
 */

#include "config.h"
#include "ex_common.h"

#   ifdef AIX
#    define _XOPEN_SOURCE
#    define _ALL_SOURCE
#   endif
#   include	<termios.h>
#   ifdef AIX
#    undef _XOPEN_SOURCE
#    undef _ALL_SOURCE
#   endif
#   ifndef NO_S5WINSIZE
#    ifdef NEED_PTEM
#     include	<sys/stream.h>	/* winsize struct defined in one of these? */
#     include	<sys/ptem.h>
#    endif
#   else
#    undef	TIOCGWINSZ	/* we can't handle it correctly yet */
#   endif

#ifndef _POSIX_VDISABLE
# define _POSIX_VDISABLE 0
#endif

static void	 starttcap (char *);

/* variables, publicly available & used in the macros */
char	*termtype;	/* name of terminal entry */
speed_t	ospeed;		/* speed of the tty, eg B2400 */
char	PC;		/* Pad char */
WINDOW	*stdscr;	/* pointer into kbuf[] */
WINDOW	kbuf[KBSIZ];	/* a very large output buffer */
int	LINES;		/* :li#: number of rows */
int	COLS;		/* :co#: number of columns */
int	AM;		/* :am:  boolean: auto margins? */
int	PT;		/* :pt:  boolean: physical tabs? */
char	*VB;		/* :vb=: visible bell */
char	*UP;		/* :up=: move cursor up */
char	*SO = "";	/* :so=: standout start */
char	*SE = "";	/* :se=: standout end */
char	*US = "";	/* :us=: underline start */
char	*UE = "";	/* :ue=: underline end */
char	*MD = "";	/* :md=: bold start */
char	*ME = "";	/* :me=: bold end */
char	*AS = "";	/* :as=: alternate (italic) start */
char	*AE = "";	/* :ae=: alternate (italic) end */
#ifndef NO_VISIBLE
char	*MV;		/* :mv=: "visible" selection start */
#endif
char	*CM;		/* :cm=: cursor movement */
char	*CE;		/* :ce=: clear to end of line */
char	*CD;		/* :cd=: clear to end of screen */
char	*AL;		/* :al=: add a line */
char	*DL;		/* :dl=: delete a line */
char	*SR;		/* :sr=: scroll reverse */
char	*KS = "";	/* :ks=: switch keypad to application mode */
char	*KE = "";	/* :ke=: switch keypad to system mode */
char	*KU;		/* :ku=: key sequence sent by up arrow */
char	*KD;		/* :kd=: key sequence sent by down arrow */
char	*KL;		/* :kl=: key sequence sent by left arrow */
char	*KR;		/* :kr=: key sequence sent by right arrow */
char	*HM;		/* :HM=: key sequence sent by the <Home> key */
char	*EN;		/* :EN=: key sequence sent by the <End> key */
char	*PU;		/* :PU=: key sequence sent by the <PgUp> key */
char	*PD;		/* :PD=: key sequence sent by the <PgDn> key */
char	*KI;		/* :kI=: key sequence sent by the <Insert> key */
char	*kDel;		/* :kD=: key sequence sent by the <Delete> key */
#ifndef NO_FKEY
char	*FKEY[NFKEYS];	/* :k0=: ... :k9=: sequences sent by function keys */
#endif
char	*IM = "";	/* :im=: insert mode start */
char	*IC = "";	/* :ic=: insert the following character */
char	*EI = "";	/* :ei=: insert mode end */
char	*DC;		/* :dc=: delete a character */
char	*TI = "";	/* :ti=: terminal init */	/* GB */
char	*TE = "";	/* :te=: terminal exit */	/* GB */
#ifndef NO_CURSORSHAPE
#if 1
char	*CQ = (char *)0;/* :cQ=: normal cursor */
char	*CX = (char *)1;/* :cX=: cursor used for EX command/entry */
char	*CV = (char *)2;/* :cV=: cursor used for VI command mode */
char	*CI = (char *)3;/* :cI=: cursor used for VI input mode */
char	*CR = (char *)4;/* :cR=: cursor used for VI replace mode */
#else
char	*CQ = "";	/* :cQ=: normal cursor */
char	*CX = "";	/* :cX=: cursor used for EX command/entry */
char	*CV = "";	/* :cV=: cursor used for VI command mode */
char	*CI = "";	/* :cI=: cursor used for VI input mode */
char	*CR = "";	/* :cR=: cursor used for VI replace mode */
#endif
#endif
char	*aend = "";	/* end an attribute -- either UE or ME */
char	ERASEKEY;	/* backspace key taken from ioctl structure */
#ifndef NO_COLOR
char	normalcolor[24];
char	SOcolor[24];
char	SEcolor[24];
char	UScolor[24];
char	UEcolor[24];
char	MDcolor[24];
char	MEcolor[24];
char	AScolor[24];
char	AEcolor[24];
char	Qcolor[24];
# ifndef NO_POPUP
char	POPUPcolor[24];
# endif
# ifndef NO_VISIBLE
char	VISIBLEcolor[24];
# endif
#endif

static struct termios	oldtermio;	/* original tty mode */
static struct termios	newtermio;	/* cbreak/noecho tty mode */

/* This boolean variable indicated whether the termcap description is
 * sufficient to support visual mode.
 */
int	canvi = TRUE;		/* boolean: know enough for visual mode? */

static char	*capbuf;	/* capability string buffer */

/* Initialize the Curses package. */
void initscr(void)
{
	/* create stdscr */
	stdscr = kbuf;

	/* make sure TERM variable is set */
	termtype = getenv("TERM");

	if (!termtype)
	{
		termtype = "unknown";
		starttcap(termtype);
	}
	else
	{
		/* start termcap stuff */
		starttcap(termtype);
	}

	/* change the terminal mode to cbreak/noecho */
	tcgetattr(2, &oldtermio);

	resume_curses(TRUE);
}

/* Shut down the Curses package. */
void endwin(void)
{
	/* change the terminal mode back the way it was */
	suspend_curses();
}

static int curses_active = FALSE;

extern int oldcurs;

/* Send any required termination strings.  Turn off "raw" mode. */
void suspend_curses(void)
{
#ifndef NO_CURSORSHAPE
	if (has_CQ)
	{
		do_CQ();
		oldcurs = 0;
	}
#endif
	if (has_TE)					/* GB */
	{
		do_TE();
	}
	if (has_KE)
	{
		do_KE();
	}
#ifndef NO_COLOR
	quitcolor();
#endif
	refresh();

	/* change the terminal mode back the way it was */
	tcsetattr(2, TCSADRAIN, &oldtermio);

	curses_active = FALSE;
}

/* put the terminal in RAW mode.  If "quietly" is FALSE, then ask the user
 * to hit a key, and wait for keystroke before returning.
 */
void resume_curses(int quietly)
{
	char	inbuf[20];

	if (!curses_active)
	{
		/* change the terminal mode to cbreak/noecho */
		ospeed = cfgetospeed(&oldtermio);
		ERASEKEY = oldtermio.c_cc[VERASE];
		newtermio = oldtermio;
#  ifdef IXANY
		newtermio.c_iflag &= (IXON|IXOFF|IXANY|ISTRIP|IGNBRK);
#  else
		newtermio.c_iflag &= (IXON|IXOFF|ISTRIP|IGNBRK);
#  endif
		newtermio.c_oflag &= ~OPOST;
		newtermio.c_lflag &= ISIG;
		newtermio.c_cc[VINTR] = ctrl('C'); /* always use ^C for interrupts */
		newtermio.c_cc[VMIN] = 1;
		newtermio.c_cc[VTIME] = 0;
#  ifdef VSWTCH
		newtermio.c_cc[VSWTCH] = _POSIX_VDISABLE;
#  endif
#  ifdef VSUSP
		newtermio.c_cc[VSUSP] = _POSIX_VDISABLE;
#  endif
		tcsetattr(2, TCSADRAIN, &newtermio);

		curses_active = TRUE;
	}

	/* If we're supposed to quit quietly, then we're done */
	if (quietly)
	{
		if (has_TI)					/* GB */
		{
			do_TI();
		}
		if (has_KS)
		{
			do_KS();
		}

		return;
	}

	signal(SIGINT, SIG_IGN);

	move(LINES - 1, 0);
	do_SO();
	qaddstr("[Press <RETURN> to continue]");
	do_SE();
	refresh();
	ttyread(inbuf, 20, 0); /* in RAW mode, so <20 is very likely */
	qaddch('\r');
	refresh();
	if (has_TI)
	{
		do_TI();
	}
	if (has_KS)
	{
		do_KS();
	}
	if (inbuf[0] == ':')
	{
		mode = MODE_COLON;
		addch('\n');
		refresh();
	}
	else
	{
		clrtoeol();
		refresh();
		mode = MODE_VI;
		redraw(MARK_UNSET, FALSE);
	}	
	exwrote = FALSE;

	signal(SIGINT, trapint);
}

/* This function fetches an optional string from termcap */
static void mayhave(char **T, char *s)
{
	char	*val;

	val = tgetstr(s, &capbuf);
	if (val)
	{
		*T = val;
	}
}

/* This function fetches a required string from termcap */
static void musthave(T, s)
	char	**T;	/* where to store the returned pointer */
	char	*s;	/* name of the capability */
{
	mayhave(T, s);
	if (!*T)
	{
		msg("This termcap entry lacks the :%.2s=: capability", s);
		canvi = FALSE;
		mode = MODE_EX;
		*T = "";
	}
}

/* This function fetches a pair of strings from termcap.  If one of them is
 * missing, then the other one is ignored.
 */
static void pair(char **T, char **U, char *sT, char *sU)
{
	mayhave(T, sT);
	mayhave(U, sU);
	if (!**T || !**U)
	{
		*T = *U = "";
	}
}

/* Read everything from termcap */
static void starttcap(char *term)
{
	static char	cbmem[800];

	/* allocate memory for capbuf */
	capbuf = cbmem;

	/* get the termcap entry */
	switch (tgetent(kbuf, term))
	{
	  case -1:
		write(2, "Can't read /etc/termcap\n", (unsigned)24);
		exit(2);

	  case 0:
		write(2, "Unrecognized TERM type\n", (unsigned)23);
		exit(3);
	}

	/* get strings */
	musthave(&UP, "up");
	mayhave(&VB, "vb");
	musthave(&CM, "cm");
	pair(&SO, &SE, "so", "se");
	mayhave(&TI, "ti");
	mayhave(&TE, "te");
	if (tgetnum("ug") <= 0)
	{
		pair(&US, &UE, "us", "ue");
		pair(&MD, &ME, "md", "me");

		/* get italics, or have it default to underline */
		pair(&AS, &AE, "as", "ae");
		if (!*AS)
		{
			AS = US;
			AE = UE;
		}
	}
#ifndef NO_VISIBLE
	MV = SO; /* by default */
	mayhave(&MV, "mv");
#endif
	mayhave(&AL, "al");
	mayhave(&DL, "dl");
	musthave(&CE, "ce");
	mayhave(&CD, "cd");
	mayhave(&SR, "sr");
	pair(&IM, &EI, "im", "ei");
	mayhave(&IC, "ic");
	mayhave(&DC, "dc");

	/* other termcap stuff */
	AM = (tgetflag("am") && !tgetflag("xn"));
	PT = tgetflag("pt");
	getsize(0);

	/* Key sequences */
	pair(&KS, &KE, "ks", "ke");	/* keypad enable/disable */
	mayhave(&KU, "ku");		/* up */
	mayhave(&KD, "kd");		/* down */
	mayhave(&KR, "kr");		/* right */
	mayhave(&KL, "kl");		/* left */
	if (KL && KL[0]=='\b' && !KL[1])
	{
		/* never use '\b' as a left arrow! */
		KL = (char *)0;
	}
	mayhave(&PU, "kP");		/* PgUp */
	mayhave(&PD, "kN");		/* PgDn */
	mayhave(&HM, "kh");		/* Home */
	mayhave(&EN, "kH");		/* End */
	mayhave(&KI, "kI");		/* Insert */
	mayhave(&kDel, "kD");		/* Delete */
#ifndef CRUNCH
	if (!PU) mayhave(&PU, "K2");	/* "3x3 pad" names for PgUp, etc. */
	if (!PD) mayhave(&PD, "K5");
	if (!HM) mayhave(&HM, "K1");
	if (!EN) mayhave(&EN, "K4");

	mayhave(&PU, "PU");		/* old XENIX names for PgUp, etc. */
	mayhave(&PD, "PD");		/* (overrides others, if used.) */
	mayhave(&HM, "HM");
	mayhave(&EN, "EN");
#endif
#ifndef NO_FKEY
	mayhave(&FKEY[0], "k0");		/* function key codes */
	mayhave(&FKEY[1], "k1");
	mayhave(&FKEY[2], "k2");
	mayhave(&FKEY[3], "k3");
	mayhave(&FKEY[4], "k4");
	mayhave(&FKEY[5], "k5");
	mayhave(&FKEY[6], "k6");
	mayhave(&FKEY[7], "k7");
	mayhave(&FKEY[8], "k8");
	mayhave(&FKEY[9], "k9");
# ifndef NO_SHIFT_FKEY
	mayhave(&FKEY[10], "s0");		/* shift function key codes */
	mayhave(&FKEY[11], "s1");
	mayhave(&FKEY[12], "s2");
	mayhave(&FKEY[13], "s3");
	mayhave(&FKEY[14], "s4");
	mayhave(&FKEY[15], "s5");
	mayhave(&FKEY[16], "s6");
	mayhave(&FKEY[17], "s7");
	mayhave(&FKEY[18], "s8");
	mayhave(&FKEY[19], "s9");
#  ifndef NO_CTRL_FKEY
	mayhave(&FKEY[20], "c0");		/* control function key codes */
	mayhave(&FKEY[21], "c1");
	mayhave(&FKEY[22], "c2");
	mayhave(&FKEY[23], "c3");
	mayhave(&FKEY[24], "c4");
	mayhave(&FKEY[25], "c5");
	mayhave(&FKEY[26], "c6");
	mayhave(&FKEY[27], "c7");
	mayhave(&FKEY[28], "c8");
	mayhave(&FKEY[29], "c9");
#   ifndef NO_ALT_FKEY
	mayhave(&FKEY[30], "a0");		/* alt function key codes */
	mayhave(&FKEY[31], "a1");
	mayhave(&FKEY[32], "a2");
	mayhave(&FKEY[33], "a3");
	mayhave(&FKEY[34], "a4");
	mayhave(&FKEY[35], "a5");
	mayhave(&FKEY[36], "a6");
	mayhave(&FKEY[37], "a7");
	mayhave(&FKEY[38], "a8");
	mayhave(&FKEY[39], "a9");
#   endif
#  endif
# endif
#endif

#ifndef NO_CURSORSHAPE
	/* cursor shapes */
	CQ = tgetstr("cQ", &capbuf);
	if (has_CQ)
	{
		CX = tgetstr("cX", &capbuf);
		if (!CX) CX = CQ;
		CV = tgetstr("cV", &capbuf);
		if (!CV) CV = CQ;
		CI = tgetstr("cI", &capbuf);
		if (!CI) CI = CQ;
		CR = tgetstr("cR", &capbuf);
		if (!CR) CR = CQ;
	}
# ifndef CRUNCH
	else
	{
		CQ = CV = "";
		pair(&CQ, &CV, "ve", "vs");
		CX = CI = CR = CQ;
	}
# endif /* !CRUNCH */
#endif /* !NO_CURSORSHAPE */

#ifndef NO_COLOR
	strcpy(SOcolor, has_SO ? SO : "");
	strcpy(SEcolor, has_SE ? SE : "");
	strcpy(AScolor, has_AS ? AS : "");
	strcpy(AEcolor, has_AE ? AE : "");
	strcpy(MDcolor, has_MD ? MD : "");
	strcpy(MEcolor, has_ME ? ME : "");
	strcpy(UScolor, has_US ? US : "");
	strcpy(UEcolor, has_UE ? UE : "");
# ifndef NO_POPUP
	strcpy(POPUPcolor, SO);
# endif
# ifndef NO_VISIBLE
	strcpy(VISIBLEcolor, MV);
# endif
	strcpy(normalcolor, SEcolor);
#endif

}

/* This function gets the window size.  It uses the TIOCGWINSZ ioctl call if
 * your system has it, or tgetnum("li") and tgetnum("co") if it doesn't.
 * This function is called once during initialization, and thereafter it is
 * called whenever the SIGWINCH signal is sent to this process.
 */
SIGTYPE getsize(int signo)
{
	int	lines;
	int	cols;
#ifdef TIOCGWINSZ
	struct winsize size;
#endif

#ifdef SIGWINCH
	/* reset the signal vector */
#  ifdef linux
	signal(SIGWINCH, (void *)getsize);
#  else
	signal(SIGWINCH, getsize);
#  endif
#endif

	/* get the window size, one way or another. */
	lines = cols = 0;
#ifdef TIOCGWINSZ
	if (ioctl(2, TIOCGWINSZ, &size) >= 0)
	{
		lines = size.ws_row;
		cols = size.ws_col;
	}
#endif
	if ((lines == 0 || cols == 0) && signo == 0)
	{
		LINES = tgetnum("li");
		if (LINES <= 0) LINES = 24;
		COLS = tgetnum("co");
		if (COLS <= 0) COLS = 80;
	}
	if (lines >= 2 && cols >= 30)
	{
		LINES = lines;
		COLS = cols;
	}

	/* Make sure we got values that we can live with */
	if (LINES < 2 || COLS < 30)
	{
		write(2, "Screen too small\n", (unsigned)17);
		endwin();
		exit(2);
	}

	/* The following is for the benefit of `lint' on systems where signal
	 * handlers are supposed to return a meaningless int value.
	 */
	/*NOTREACHED*/
}

/* This is a function version of addch() -- it is used by tputs() */
int faddch(int ch)
{
	addch(ch);

	return 0;
}

/* This function quickly adds a string to the output queue.  It does *NOT*
 * convert \n into <CR><LF>.
 */
void qaddstr(char *str)
{
	REG char *s_, *d_;

	for (s_=(str), d_=stdscr; *d_++ = *s_++; ) /* yes, ASSIGNMENT! */
	{
	}
	stdscr = d_ - 1;
}

/* Output the ESC sequence needed to go into any video mode, if supported */
void attrset(int a)
{
	do_aend();
	if (a == A_BOLD)
	{
		do_MD();
		aend = ME;
	}
	else if (a == A_UNDERLINE)
	{
		do_US();
		aend = UE;
	}
	else if (a == A_ALTCHARSET)
	{
		do_AS();
		aend = AE;
	}
	else
	{
		aend = "";
	}
}

/* Insert a single character into the display */
void insch(int ch)
{
	if (has_IM)
		do_IM();
	do_IC();
	qaddch(ch);
	if (has_EI)
		do_EI();
}

void wrefresh(void)
{
	if (stdscr != kbuf)
	{
		ttywrite(kbuf, (unsigned)(stdscr - kbuf));
		stdscr = kbuf;
	}
}

void wqrefresh(void)
{
	if (stdscr - kbuf > 2000)
	{
		ttywrite(kbuf, (unsigned)(stdscr - kbuf)); 
		stdscr = kbuf;
	}
}

#ifndef NO_COLOR
/* This function is called during termination.  It resets color modes */
int ansiquit(void)
{
	/* if ANSI terminal & color 'q' was set, then reset the colors */
	if (!strcmp(UP, "\033[A") && *Qcolor)
	{
		tputs("\033[m", 1, faddch);
		tputs(Qcolor, 1, faddch);
		clrtoeol();
		return 1;
	}
	return 0;
}

/* This sets the color strings that work for ANSI terminals.  If the TERMCAP
 * doesn't look like an ANSI terminal, then it returns FALSE.  If the colors
 * aren't understood, it also returns FALSE.  If all goes well, it returns TRUE
 */
int ansicolor(int cmode, int attrbyte)
{
	char	temp[24];	/* hold the new mode string */

	/* if not ANSI-ish, then fail */
	if (strcmp(UP, "\033[A") && strcmp(UP, "\033OA"))
	{
		/* Only give an error message if we're editing a file.
		 * (I.e., if we're *NOT* currently doing a ".exrc")
		 */
		if (tmpfd >= 0)
			msg("Don't know how to set colors for this terminal");
		return FALSE;
	}

	/* construct the color string */
	sprintf(temp, "\033[m\033[3%c;4%c%s%sm",
		"04261537"[attrbyte & 0x07],
		"04261537"[(attrbyte >> 4) & 0x07],
		(attrbyte & 0x08) ? ";1" : "",
		(attrbyte & 0x80) ? ";5" : "");

	/* stick it in the right place */
	switch (cmode)
	{
	  case A_NORMAL:
		if (!strcmp(MEcolor, normalcolor))
			strcpy(MEcolor, temp);
		if (!strcmp(UEcolor, normalcolor))
			strcpy(UEcolor, temp);
		if (!strcmp(AEcolor, normalcolor))
			strcpy(AEcolor, temp);
		if (!strcmp(SEcolor, normalcolor))
			strcpy(SEcolor, temp);

		strcpy(normalcolor, temp);
		tputs(normalcolor, 1, faddch);
		break;

	  case A_BOLD:
		strcpy(MDcolor, temp);
		strcpy(MEcolor, normalcolor);
		break;

	  case A_UNDERLINE:
		strcpy(UScolor, temp);
		strcpy(UEcolor, normalcolor);
		break;

	  case A_ALTCHARSET:
		strcpy(AScolor, temp);
		strcpy(AEcolor, normalcolor);
		break;

	  case A_STANDOUT:
		strcpy(SOcolor, temp);
		strcpy(SEcolor, normalcolor);
		break;

	  case A_QUIT:
		strcpy(Qcolor, temp);
		break;

#ifndef NO_POPUP
	  case A_POPUP:
		strcpy(POPUPcolor, temp);
		break;
#endif

#ifndef NO_VISIBLE
	  case A_VISIBLE:
		strcpy(VISIBLEcolor, temp);
		break;
#endif
	}

	return TRUE;
}

/* This function outputs the ESC sequence needed to switch the screen back
 * to "normal" mode.  On color terminals which haven't had their color set
 * yet, this is one of the termcap strings; for color terminals that really
 * have had colors defined, we just the "normal color" escape sequence.
 */
int endcolor(void)
{
	if (aend == ME)
		tputs(MEcolor, 1, faddch);
	else if (aend == UE)
		tputs(UEcolor, 1, faddch);
	else if (aend == AE)
		tputs(AEcolor, 1, faddch);
	else if (aend == SE)
		tputs(SEcolor, 1, faddch);
	aend = "";
	return 0;
}

#endif /* !NO_COLOR */
