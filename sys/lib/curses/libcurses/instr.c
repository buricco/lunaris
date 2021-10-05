/*
 * Copyright 2020 S. V. Nickolas.
 * 
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions are met:
 * 
 *   1. Redistributions of source code must retain the above copyright notice, 
 *      this list of conditions and the following disclaimer.
 *   2. Redistributions in binary form must reproduce the above copyright 
 *      notice, this list of conditions and the following disclaimer in the 
 *      documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS "AS IS" AND ANY 
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
 * DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE FOR ANY 
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT 
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY 
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF 
 * SUCH DAMAGE.
 */

#include "curses.h"
#include "curses_private.h"

#ifndef _CURSES_USE_MACROS
/* These are reimplementations from the manual -uso. */

int instr(char *str)
{
 return winstr(stdscr, str);
}

int innstr(char *str, int n)
{
 return winnstr(stdscr, str, n);
}

int mvinstr(int y, int x, char *str)
{
 return mvwinstr(stdscr, y, x, str);
}

int mvinnstr(int y, int x, char *str, int n)
{
 return mvwinnstr(stdscr, y, x, str, n);
}

int mvwinstr(WINDOW *win, int y, int x, char *str)
{
 return mvwinnstr(win, y, x, str, -1);
}

int mvwinnstr(WINDOW *win, int y, int x, char *str, int n)
{
 return (wmove(win,y,x)==ERR)?ERR:winnstr(win,str,n);
}

int winstr(WINDOW *win, char *str)
{
 return winnstr(win, str, -1);
}
#endif

/*
 * Just a quick and dirty rewrite around a 4-clause BSD license...
 * This is limited to 1-byte characters, as is the version I wrote this to
 * replace. XXX that should really be worked on.
 */
int winnstr(WINDOW *win, char *str, int n)
{
 int t;
 
 if (!win) return ERR;
 if (!str) return ERR;
 
 for (t=0; (n==-1)?((t+(win)->curx)<(win)->maxx):(t<(n-1)); t++)
 {
  str[t]=
    (chtype)((win)->alines[(win)->cury]->line[((win)->curx)+t]).ch&__CHARTEXT;
 }
 str[t]=0;
 
 return OK;
}
