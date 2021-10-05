/*	$OpenBSD: calendar.h,v 1.9 2003/06/03 02:56:06 millert Exp $	*/

/*
 * Copyright (c) 1989, 1993, 1994
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

#include <sys/uio.h>

extern struct passwd *pw;
extern int doall;
extern int bodun_always;
extern time_t f_time;
extern struct iovec header[];
extern struct tm *tp;
extern char *calendarFile;
extern char *calendarHome;
extern char *calendarPath;
extern char *optarg;

struct fixs {
	wchar_t *name;
	int len;
};

struct event {
	time_t	when;
	struct tm *tm;
	wchar_t	var;
	wchar_t	**desc;
	wchar_t	*ldesc;
	struct event	*next;
};

struct match {
	time_t	when;
	struct tm *tm;
	int	bodun;
	int	var;
	struct match	*next;
};

struct specialev {
	wchar_t *name;
	int nlen;
	wchar_t *uname;
	int ulen;
	int (*getev)(int);
};

wchar_t	 *wcsdup(const wchar_t *);
int	 wcscasecmp(const wchar_t *, const wchar_t *);
int	 wcsncasecmp(const wchar_t *, const wchar_t *, size_t);
wchar_t	 *myfgetws(wchar_t *, int, FILE *);

void	 cal(void);
void	 closecal(FILE *);
int	 getday(wchar_t *);
int	 getdayvar(wchar_t *);
int	 getfield(wchar_t *, wchar_t **, int *);
int	 getmonth(wchar_t *);
int	 easter(int);
int	 paskha(int);
void	 insert(struct event **, struct event *);
struct match	*isnow(wchar_t *, int);
FILE	*opencal(void);
void	 settime(time_t *);
time_t	 Mktime(char *);
void	 usage(void);
int	 foy(int);
void	 variable_weekday(int *, int, int);
void	 setnnames(void);
void	 spev_init(void);

/* some flags */
#define	F_ISMONTH	0x01 /* month (Januar ...) */
#define	F_ISDAY		0x02 /* day of week (Sun, Mon, ...) */
/*#define	F_ISDAYVAR	0x04  variables day of week, like SundayLast */
#define	F_SPECIAL	0x08 /* Events that occur once a year but don't track
			      * calendar time--e.g.  Easter or easter depending
			      * days */

extern unsigned short lookahead;	/* how many days to look ahead */
extern unsigned short weekend;		/* how many days to look ahead if today is Friday */

/* Special events; see also setnnames() in day.c */
/* '=' is not a valid character in a special event name */
#define EASTER L"easter"
#define EASTERNAMELEN ((sizeof(EASTER) / sizeof(wchar_t)) - 1)
#define PASKHA L"paskha"
#define PASKHALEN ((sizeof(PASKHA) / sizeof(wchar_t)) - 1)

#define NUMEV 2	/* Total number of such special events */
extern struct specialev spev[NUMEV];

/* For calendar -a, specify a maximum time (in seconds) to spend parsing
 * each user's calendar files.  This prevents them from hanging calendar
 * (e.g. by using named pipes)
 */
#define USERTIMEOUT 20
