#include "curses.h"
#include "curses_private.h"

#ifndef _CURSES_USE_MACROS
int inchstr(chtype *chstr)
{
 return winchstr(stdscr, chstr);
}

int inchnstr(chtype *chstr, int n)
{
 return winchnstr(stdscr, chstr, n);
}

int mvinchstr(int y, int x, chtype *chstr)
{
 return mvwinchstr(stdscr, y, x, chstr);
}

int mvinchnstr(int y, int x, chtype *chstr, int n)
{
 return mvwinchnstr(stdscr, y, x, chstr, n);
}

int mvwinchstr(WINDOW *win, int y, int x, chtype *chstr)
{
 return mvwinchnstr(win, y, x, chstr, -1);
}

int mvwinchnstr(WINDOW *win, int y, int x, chtype *chstr, int n)
{
 return (wmove(win,y,x)==ERR)?ERR:winchnstr(win,chstr,n);
}
#endif

int winchstr(WINDOW *win, chtype *chstr)
{
 return winchnstr(win, chstr, -1);
}

int winchnstr(WINDOW *win, chtype *chstr, int n)
{
 int t;
 
 if (!win) return ERR;
 if (!chstr) return ERR;
 
 for (t=0; (n==-1)?((t+(win)->curx)<(win)->maxx):(t<(n-1)); t++)
 {
  __LDATA x;
  
  x=((win)->alines[(win)->cury]->line[((win)->curx)+t]);
  chstr[t]=(x.ch) | (x.attr);
 }
 chstr[t]=0;
 
 return OK;
}
