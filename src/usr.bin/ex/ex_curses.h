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

/* curses.h */

#include <termios.h> /* uso */

/* This is the header file for a small, fast, fake curses package */

/* termcap stuff */
extern char	*tgoto (char *, int, int);
extern char	*tgetstr (char*, char**);
extern void	tputs (char *, int, int (*)());

/* faddch() is a function.  a pointer to it is passed to tputs() */
extern int	faddch (int);

/* data types */
#define WINDOW	char

/* CONSTANTS & SYMBOLS */
#define TRUE		1
#define FALSE		0
#define A_NORMAL	0
#define A_STANDOUT	1
#define A_BOLD		2
#define A_QUIT		3
#define A_UNDERLINE	4
#define A_ALTCHARSET	5
#define A_POPUP		6
#define A_VISIBLE	7
#define KBSIZ		4096

/* figure out how many function keys we need to allow. */
#ifndef NO_FKEY
# ifdef NO_SHIFT_FKEY
#  define	NFKEYS	10
# else
#  ifdef NO_CTRL_FKEY
#   define	NFKEYS	20
#  else
#   ifdef NO_ALT_FKEY
#    define	NFKEYS	30
#   else
#    define	NFKEYS	40
#   endif
#  endif
# endif
extern char	*FKEY[NFKEYS];	/* :k0=:...:k9=: codes sent by function keys */
#endif

/* extern variables, defined in curses.c */
extern char	*termtype;	/* name of terminal entry */
extern speed_t	ospeed;		/* tty speed, eg B2400 */
extern char	PC;		/* Pad char */
extern WINDOW	*stdscr;	/* pointer into kbuf[] */
extern WINDOW	kbuf[KBSIZ];	/* a very large output buffer */
extern int	LINES;		/* :li#: number of rows */
extern int	COLS;		/* :co#: number of columns */
extern int	AM;		/* :am:  boolean: auto margins? */
extern int	PT;		/* :pt:  boolean: physical tabs? */
extern char	*VB;		/* :vb=: visible bell */
extern char	*UP;		/* :up=: move cursor up */
extern char	*SO;		/* :so=: standout start */
extern char	*SE;		/* :se=: standout end */
extern char	*US;		/* :us=: underline start */
extern char	*UE;		/* :ue=: underline end */
extern char	*MD;		/* :md=: bold start */
extern char	*ME;		/* :me=: bold end */
extern char	*AS;		/* :as=: alternate (italic) start */
extern char	*AE;		/* :ae=: alternate (italic) end */
#ifndef NO_VISIBLE
extern char	*MV;		/* :mv=: "visible" selection start */
#endif
extern char	*CM;		/* :cm=: cursor movement */
extern char	*CE;		/* :ce=: clear to end of line */
extern char	*CD;		/* :cd=: clear to end of screen */
extern char	*AL;		/* :al=: add a line */
extern char	*DL;		/* :dl=: delete a line */
extern char	*SR;		/* :sr=: scroll reverse */
extern char	*KS;		/* :ks=: init string for cursor */
extern char	*KE;		/* :ke=: restore string for cursor */
extern char	*KU;		/* :ku=: sequence sent by up key */
extern char	*KD;		/* :kd=: sequence sent by down key */
extern char	*KL;		/* :kl=: sequence sent by left key */
extern char	*KR;		/* :kr=: sequence sent by right key */
extern char	*PU;		/* :PU=: key sequence sent by PgUp key */
extern char	*PD;		/* :PD=: key sequence sent by PgDn key */
extern char	*HM;		/* :HM=: key sequence sent by Home key */
extern char	*EN;		/* :EN=: key sequence sent by End key */
extern char	*KI;		/* :kI=: key sequence sent by Insert key */
extern char	*kDel;		/* :kD=: key sequence sent by Delete key */
extern char	*IM;		/* :im=: insert mode start */
extern char	*IC;		/* :ic=: insert following char */
extern char	*EI;		/* :ei=: insert mode end */
extern char	*DC;		/* :dc=: delete a character */
extern char	*TI;		/* :ti=: terminal init */	/* GB */
extern char	*TE;		/* :te=: terminal exit */	/* GB */
#ifndef NO_CURSORSHAPE
extern char	*CQ;		/* :cQ=: normal cursor */
extern char	*CX;		/* :cX=: cursor used for EX command/entry */
extern char	*CV;		/* :cV=: cursor used for VI command mode */
extern char	*CI;		/* :cI=: cursor used for VI input mode */
extern char	*CR;		/* :cR=: cursor used for VI replace mode */
#endif
extern char	*aend;		/* end an attribute -- either UE or ME */
extern char	ERASEKEY;	/* taken from the ioctl structure */
#ifndef NO_COLOR
extern char	SOcolor[];
extern char	SEcolor[];
extern char	UScolor[];
extern char	UEcolor[];
extern char	MDcolor[];
extern char	MEcolor[];
extern char	AScolor[];
extern char	AEcolor[];
extern char	Qcolor[];
#ifndef NO_POPUP
extern char	POPUPcolor[];
#endif
#ifndef NO_VISIBLE
extern char	VISIBLEcolor[];
#endif
extern char	normalcolor[];
#endif /* undef NO_COLOR */

extern int canvi;	/* boolean: know enough to support visual mode? */

/* Msdos-versions may use bios; others always termcap.
 * Will emit some 'code has no effect' warnings in unix.
 */
 
#ifndef NO_COLOR
extern int	bioscolor (int,int);
extern int	biosquit (void);
# define setcolor(m,a)	ansicolor(m,a)
# define fixcolor()	{tputs(normalcolor, 1, faddch);}
# define quitcolor()	ansiquit()
# define do_SO()	{ tputs(SOcolor, 1, faddch); }
# define do_SE()	{ tputs(SEcolor, 1, faddch); }
# define do_US()	{ tputs(UScolor, 1, faddch); }
# define do_UE()	{ tputs(UEcolor, 1, faddch); }
# define do_MD()	{ tputs(MDcolor, 1, faddch); }
# define do_ME()	{ tputs(MEcolor, 1, faddch); }
# define do_AS()	{ tputs(AScolor, 1, faddch); }
# define do_AE()	{ tputs(AEcolor, 1, faddch); }
# define do_POPUP()	{ tputs(POPUPcolor, 1, faddch); }
# define do_VISIBLE()	{ tputs(VISIBLEcolor, 1, faddch); }
#else
# define do_SO()	{ tputs(SO, 1, faddch); }
# define do_SE()	{ tputs(SE, 1, faddch); }
# define do_US()	{ tputs(US, 1, faddch); }
# define do_UE()	{ tputs(UE, 1, faddch); }
# define do_MD()	{ tputs(MD, 1, faddch); }
# define do_ME()	{ tputs(ME, 1, faddch); }
# define do_AS()	{ tputs(AS, 1, faddch); }
# define do_AE()	{ tputs(AE, 1, faddch); }
# define do_POPUP()	{ tputs(SO, 1, faddch); }
# define do_VISIBLE()	{ tputs(MV, 1, faddch); }
#endif

#define	do_VB()		{ tputs(VB, 1, faddch); }
#define	do_UP()		{ tputs(UP, 1, faddch); }
#undef	do_CM		/* move */
#define	do_CE()		{ tputs(CE, 1, faddch); }
#define	do_CD()		{ tputs(CD, 1, faddch); }
#define	do_AL()		{ tputs(AL, LINES, faddch); }
#define	do_DL()		{ tputs(DL, LINES, faddch); }
#define do_SR()	{ tputs(SR, 1, faddch); }
#define do_KS()		{ tputs(KS, 1, faddch); }
#define do_KE()		{ tputs(KE, 1, faddch); }
#define	do_IM()		{ tputs(IM, 1, faddch); }
#define	do_IC()		{ tputs(IC, 1, faddch); }
#define	do_EI()		{ tputs(EI, 1, faddch); }
#define	do_DC()		{ tputs(DC, COLS, faddch); }
#define	do_TI()		{ (void)ttywrite(TI, (unsigned)strlen(TI)); }
#define	do_TE()		{ (void)ttywrite(TE, (unsigned)strlen(TE)); }
#ifndef NO_CURSORSHAPE
# define do_CQ()	{ tputs(CQ, 1, faddch); }
# define do_CX()	{ tputs(CX, 1, faddch); }
# define do_CV()	{ tputs(CV, 1, faddch); }
# define do_CI()	{ tputs(CI, 1, faddch); }
# define do_CR()	{ tputs(CR, 1, faddch); }
#endif
#ifndef NO_COLOR
# define do_aend()	{ endcolor(); }
#else
# define do_aend()	{ tputs(aend, 1, faddch); }
#endif

#define	has_AM		AM
#define	has_PT		PT
#define	has_VB		VB
#define	has_UP		UP
#define	has_SO		(*SO)
#define	has_SE		(*SE)
#define	has_US		(*US)
#define	has_UE		(*UE)
#define	has_MD		(*MD)
#define	has_ME		(*ME)
#define	has_AS		(*AS)
#define	has_AE		(*AE)
#undef	has_CM		/* cursor move: don't need */
#define	has_CB		0
#define	has_CS		0
#define	has_CE		CE
#define	has_CD		CD
#define	has_AL		AL
#define	has_DL		DL
#define has_SR	 SR
#define has_KS		(*KS)
#define has_KE		(*KE)
#define	has_KU		KU
#define	has_KD		KD
#define	has_KL		KL
#define	has_KR		KR
#define has_HM		HM
#define has_EN		EN
#define has_PU		PU
#define has_PD		PD
#define has_KI		KI
#define has_kD		kDel
#define	has_IM		(*IM)
#define	has_IC		(*IC)
#define	has_EI		(*EI)
#define	has_DC		DC
#define	has_TI		(*TI)
#define	has_TE		(*TE)
#ifndef NO_CURSORSHAPE
# define has_CQ	CQ
#endif

#define do_beep()	ttywrite("\007",1)

/* (pseudo)-Curses-functions */

#ifdef lint
# define _addCR		{(stdscr[-1] == '\n' ? qaddch('\r') : (stdscr[-1] = '\n'));}
#else
# define _addCR		{(stdscr[-1] == '\n' ? qaddch('\r') : 0);}
#endif

extern	int	v_put (int);

#define qaddch(ch)	(*stdscr = (ch), *stdscr++)
#define addch(ch)	if (qaddch(ch) == '\n') qaddch('\r'); else

extern void initscr (void);
extern void endwin (void);
extern void suspend_curses (void);
extern void resume_curses (int);
extern void attrset (int);
extern void insch (int);
extern void qaddstr (char *);
extern void wrefresh (void);
extern void wqrefresh (void);
extern SIGTYPE getsize();

#define addstr(str)	{qaddstr(str); _addCR;}
#define move(y,x)	{tputs(tgoto(CM, x, y), 1, faddch);}
#define mvaddch(y,x,ch)	{move(y,x); addch(ch);}
#define refresh()	{wrefresh();}
#define standout()	do_SO()
#define standend()	do_SE()
#define clrtoeol()	do_CE()
#define clrtobot()	do_CD()
#define insertln()	do_AL()
#define deleteln()	do_DL()
#define delch()		do_DC()
#define scrollok(w,b)
#define raw()
#define echo()
#define cbreak()
#define noraw()
#define noecho()
#define nocbreak()
