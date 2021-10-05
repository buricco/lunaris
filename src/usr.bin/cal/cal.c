/*
 * Copyright (c) 1989, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * egetopt Copyright (c) 1991 Keith Muller.
 * Copyright (c) 1993
 *	  The Regents of the University of California.  All rights reserved.
 *
 * Adaptations copyright (c) 2020 S. V. Nickolas.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Kim Letkeman.
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

#ifndef lint
static char copyright[] =
"@(#) Copyright (c) 1989, 1993\n\
	The Regents of the University of California.  All rights reserved.\n";
#endif /* not lint */

#ifndef lint
static char sccsid[] = "@(#)cal.c	8.1 (Berkeley) 6/6/93";
#endif /* not lint */

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define	THURSDAY		4		/* for reformation */
#define	SATURDAY 		6		/* 1 Jan 1 was a Saturday */

#define	FIRST_MISSING_DAY 	639787		/* 3 Sep 1752 */
#define	NUMBER_MISSING_DAYS 	11		/* 11 day correction */

#define	MAXDAYS			42		/* max slots in a month array */
#define	SPACE			-1		/* used in day array */

static int days_in_month[2][13] = {
	{0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
	{0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
};

int sep1752[MAXDAYS] = {
	SPACE,	SPACE,	1,	2,	14,	15,	16,
	17,	18,	19,	20,	21,	22,	23,
	24,	25,	26,	27,	28,	29,	30,
	SPACE,	SPACE,	SPACE,	SPACE,	SPACE,	SPACE,	SPACE,
	SPACE,	SPACE,	SPACE,	SPACE,	SPACE,	SPACE,	SPACE,
	SPACE,	SPACE,	SPACE,	SPACE,	SPACE,	SPACE,	SPACE,
}, j_sep1752[MAXDAYS] = {
	SPACE,	SPACE,	245,	246,	258,	259,	260,
	261,	262,	263,	264,	265,	266,	267,
	268,	269,	270,	271,	272,	273,	274,
	SPACE,	SPACE,	SPACE,	SPACE,	SPACE,	SPACE,	SPACE,
	SPACE,	SPACE,	SPACE,	SPACE,	SPACE,	SPACE,	SPACE,
	SPACE,	SPACE,	SPACE,	SPACE,	SPACE,	SPACE,	SPACE,
}, empty[MAXDAYS] = {
	SPACE,	SPACE,	SPACE,	SPACE,	SPACE,	SPACE,	SPACE,
	SPACE,	SPACE,	SPACE,	SPACE,	SPACE,	SPACE,	SPACE,
	SPACE,	SPACE,	SPACE,	SPACE,	SPACE,	SPACE,	SPACE,
	SPACE,	SPACE,	SPACE,	SPACE,	SPACE,	SPACE,	SPACE,
	SPACE,	SPACE,	SPACE,	SPACE,	SPACE,	SPACE,	SPACE,
	SPACE,	SPACE,	SPACE,	SPACE,	SPACE,	SPACE,	SPACE,
};

char *month_names[12] = {
	"January", "February", "March", "April", "May", "June",
	"July", "August", "September", "October", "November", "December",
};

char *day_headings = " S  M Tu  W Th  F  S";
char *j_day_headings = "  S   M  Tu   W  Th   F   S";

/* leap year -- account for gregorian reformation in 1752 */
#define	leap_year(yr) \
	((yr) <= 1752 ? !((yr) % 4) : \
	!((yr) % 4) && ((yr) % 100) || !((yr) % 400))

/* number of centuries since 1700, not inclusive */
#define	centuries_since_1700(yr) \
	((yr) > 1700 ? (yr) / 100 - 17 : 0)

/* number of centuries since 1700 whose modulo of 400 is 0 */
#define	quad_centuries_since_1700(yr) \
	((yr) > 1600 ? ((yr) - 1600) / 400 : 0)

/* number of leap years between year 1 and this year, not inclusive */
#define	leap_years_since_year_1(yr) \
	((yr) / 4 - centuries_since_1700(yr) + quad_centuries_since_1700(yr))

int julian;

/* new prototypes */
void monthly(int month, int year);
void yearly(int year);
void j_yearly(int year);
int day_in_week(int day, int month, int year);
int day_in_year(register int day, register int month, int year);
void day_array(int month, int year, register int *days);
void ascii_day(register char *p, register int day);
void trim_trailing_spaces(register char *s);
void center(char *str, register int len, int separate);
void usage(void);

/*
 * egetopt:	get option letter from argument vector (an extended
 *		version of getopt).
 *
 * Non standard additions to the ostr specs are:
 * 1) '?': immediate value following arg is optional (no white space
 *    between the arg and the value)
 * 2) '#': +/- followed by a number (with an optional sign but
 *    no white space between the arg and the number). The - may be
 *    combined with other options, but the + cannot.
 */

int	opterr = 1;		/* if error message should be printed */
int	optind = 1;		/* index into parent argv vector */
int	optopt;		/* character checked for validity */
char	*optarg;		/* argument associated with option */

#define	BADCH	(int)'?'
#define	EMSG	""

int getopt(int nargc, char *const *nargv, const char *ostr)
{
	static char *place = EMSG;	/* option letter processing */
	char *oli;			/* option letter list index */
	static int delim;		/* which option delimiter */
	char *p;
	static char savec = '\0';

	if (savec != '\0') {
		*place = savec;
		savec = '\0';
	}

	if (!*place) {
		/*
		 * update scanning pointer
		 */
		if ((optind >= nargc) ||
		    ((*(place = nargv[optind]) != '-') && (*place != '+'))) {
			place = EMSG;
			return (-1);
		}

		delim = (int)*place;
		if (place[1] && *++place == '-' && !place[1]) {
			/*
			 * found "--"
			 */
			++optind;
			place = EMSG;
			return (-1);
		}
	}

	/*
	 * check option letter
	 */
	if ((optopt = (int)*place++) == (int)':' || (optopt == (int)'?') ||
	    !(oli = strchr(ostr, optopt))) {
		/*
		 * if the user didn't specify '-' as an option,
		 * assume it means -1 when by itself.
		 */
		if ((optopt == (int)'-') && !*place)
			return (-1);
		if (strchr(ostr, '#') && (isdigit(optopt) ||
		    (((optopt == (int)'-') || (optopt == (int)'+')) &&
		      isdigit((unsigned char)*place)))) {
			/*
			 * # option: +/- with a number is ok
			 */
			for (p = place; *p != '\0'; ++p) {
				if (!isdigit((unsigned char)*p))
					break;
			}
			optarg = place-1;

			if (*p == '\0') {
				place = EMSG;
				++optind;
			} else {
				place = p;
				savec = *p;
				*place = '\0';
			}
			return (delim);
		}

		if (!*place)
			++optind;
		if (opterr) {
			if (!(p = strrchr(*nargv, '/')))
				p = *nargv;
			else
				++p;
			(void)fprintf(stderr, "%s: illegal option -- %c\n",
			    p, optopt);
		}
		return (BADCH);
	}
	if (delim == (int)'+') {
		/*
		 * '+' is only allowed with numbers
		 */
		if (!*place)
			++optind;
		if (opterr) {
			if (!(p = strrchr(*nargv, '/')))
				p = *nargv;
			else
				++p;
			(void)fprintf(stderr,
				"%s: illegal '+' delimiter with option -- %c\n",
				p, optopt);
		}
		return (BADCH);
	}
	++oli;
	if ((*oli != ':') && (*oli != '?')) {
		/*
		 * don't need argument
		 */
		optarg = NULL;
		if (!*place)
			++optind;
		return (optopt);
	}

	if (*place) {
		/*
		 * no white space
		 */
		optarg = place;
	} else if (*oli == '?') {
		/*
		 * no arg, but NOT required
		 */
		optarg = NULL;
	} else if (nargc <= ++optind) {
		/*
		 * no arg, but IS required
		 */
		place = EMSG;
		if (opterr) {
			if (!(p = strrchr(*nargv, '/')))
				p = *nargv;
			else
				++p;
			(void)fprintf(stderr,
			    "%s: option requires an argument -- %c\n", p,
			    optopt);
		}
		return (BADCH);
	} else {
		/*
		 * arg has white space
		 */
		optarg = nargv[optind];
	}
	place = EMSG;
	++optind;
	return (optopt);
}
/* end of getopt code */

int main(int argc, char **argv)
{
	extern char *optarg;
	extern int optind;
	struct tm *local_time;
	time_t now, time();
	int ch, month, year, yflag;

	yflag = 0;
	while ((ch = getopt(argc, argv, "jy")) != EOF)
		switch(ch) {
		case 'j':
			julian = 1;
			break;
		case 'y':
			yflag = 1;
			break;
		case '?':
		default:
			usage();
		}
	argc -= optind;
	argv += optind;

	month = 0;
	switch(argc) {
	case 2:
		if ((month = atoi(*argv++)) <= 0 || month > 12) {
			(void)fprintf(stderr,
			    "cal: illegal month value: use 0-12\n");
			return(1);
		}
		/* FALLTHROUGH */
	case 1:
		if ((year = atoi(*argv)) <= 0 || year > 9999) {
			(void)fprintf(stderr,
			    "cal: illegal year value: use 0-9999\n");
			return(1);
		}
		break;
	case 0:
		(void)time(&now);
		local_time = localtime(&now);
		year = local_time->tm_year + 1900;
		if (!yflag)
			month = local_time->tm_mon + 1;
		break;
	default:
		usage();
	}

	if (month)
		monthly(month, year);
	else if (julian)
		j_yearly(year);
	else
		yearly(year);
	return(0);
}

#define	DAY_LEN		3		/* 3 spaces per day */
#define	J_DAY_LEN	4		/* 4 spaces per day */
#define	WEEK_LEN	20		/* 7 * 3 - one space at the end */
#define	J_WEEK_LEN	27		/* 7 * 4 - one space at the end */
#define	HEAD_SEP	2		/* spaces between day headings */
#define	J_HEAD_SEP	2

void monthly(int month, int year)
{
	register int col, row;
	register char *p;
	int len, days[MAXDAYS];
	char lineout[30];

	day_array(month, year, days);
	len = sprintf(lineout, "%s %d", month_names[month - 1], year);
	(void)printf("%*s%s\n%s\n",
	    ((julian ? J_WEEK_LEN : WEEK_LEN) - len) / 2, "",
	    lineout, julian ? j_day_headings : day_headings);
	for (row = 0; row < 6; row++) {
		for (col = 0, p = lineout; col < 7; col++,
		    p += julian ? J_DAY_LEN : DAY_LEN)
			ascii_day(p, days[row * 7 + col]);
		trim_trailing_spaces(lineout);
		(void)printf("%s\n", lineout);
	}
}

void j_yearly(int year)
{
	register int col, *dp, i, month, row, which_cal;
	register char *p;
	int days[12][MAXDAYS];
	char lineout[80];

	(void)sprintf(lineout, "%d", year);
	center(lineout, J_WEEK_LEN * 2 + J_HEAD_SEP, 0);
	(void)printf("\n\n");
	for (i = 0; i < 12; i++)
		day_array(i + 1, year, days[i]);
	(void)memset(lineout, ' ', sizeof(lineout) - 1);
	lineout[sizeof(lineout) - 1] = '\0';
	for (month = 0; month < 12; month += 2) {
		center(month_names[month], J_WEEK_LEN, J_HEAD_SEP);
		center(month_names[month + 1], J_WEEK_LEN, 0);
		(void)printf("\n%s%*s%s\n", j_day_headings, J_HEAD_SEP, "",
		    j_day_headings);
		for (row = 0; row < 6; row++) {
			for (which_cal = 0; which_cal < 2; which_cal++) {
				p = lineout + which_cal * (J_WEEK_LEN + 2);
				dp = &days[month + which_cal][row * 7];
				for (col = 0; col < 7; col++, p += J_DAY_LEN)
					ascii_day(p, *dp++);
			}
			trim_trailing_spaces(lineout);
			(void)printf("%s\n", lineout);
		}
	}
	(void)printf("\n");
}

void yearly(int year)
{
	register int col, *dp, i, month, row, which_cal;
	register char *p;
	int days[12][MAXDAYS];
	char lineout[80];

	(void)sprintf(lineout, "%d", year);
	center(lineout, WEEK_LEN * 3 + HEAD_SEP * 2, 0);
	(void)printf("\n\n");
	for (i = 0; i < 12; i++)
		day_array(i + 1, year, days[i]);
	(void)memset(lineout, ' ', sizeof(lineout) - 1);
	lineout[sizeof(lineout) - 1] = '\0';
	for (month = 0; month < 12; month += 3) {
		center(month_names[month], WEEK_LEN, HEAD_SEP);
		center(month_names[month + 1], WEEK_LEN, HEAD_SEP);
		center(month_names[month + 2], WEEK_LEN, 0);
		(void)printf("\n%s%*s%s%*s%s\n", day_headings, HEAD_SEP,
		    "", day_headings, HEAD_SEP, "", day_headings);
		for (row = 0; row < 6; row++) {
			for (which_cal = 0; which_cal < 3; which_cal++) {
				p = lineout + which_cal * (WEEK_LEN + 2);
				dp = &days[month + which_cal][row * 7];
				for (col = 0; col < 7; col++, p += DAY_LEN)
					ascii_day(p, *dp++);
			}
			trim_trailing_spaces(lineout);
			(void)printf("%s\n", lineout);
		}
	}
	(void)printf("\n");
}

/*
 * day_array --
 *	Fill in an array of 42 integers with a calendar.  Assume for a moment
 *	that you took the (maximum) 6 rows in a calendar and stretched them
 *	out end to end.  You would have 42 numbers or spaces.  This routine
 *	builds that array for any month from Jan. 1 through Dec. 9999.
 */
void day_array(int month, int year, register int *days)
{
	register int i, day, dw, dm;

	if (month == 9 && year == 1752) {
		bcopy(julian ? j_sep1752 : sep1752,
		    days, MAXDAYS * sizeof(int));
		return;
	}
	bcopy(empty, days, MAXDAYS * sizeof(int));
	dm = days_in_month[leap_year(year)][month];
	dw = day_in_week(1, month, year);
	day = julian ? day_in_year(1, month, year) : 1;
	while (dm--)
		days[dw++] = day++;
}

/*
 * day_in_year --
 *	return the 1 based day number within the year
 */
int day_in_year(register int day, register int month, int year)
{
	register int i, leap;

	leap = leap_year(year);
	for (i = 1; i < month; i++)
		day += days_in_month[leap][i];
	return(day);
}

/*
 * day_in_week
 *	return the 0 based day number for any date from 1 Jan. 1 to
 *	31 Dec. 9999.  Assumes the Gregorian reformation eliminates
 *	3 Sep. 1752 through 13 Sep. 1752.  Returns Thursday for all
 *	missing days.
 */
int day_in_week(int day, int month, int year)
{
	long temp;

	temp = (long)(year - 1) * 365 + leap_years_since_year_1(year - 1)
	    + day_in_year(day, month, year);
	if (temp < FIRST_MISSING_DAY)
		return((temp - 1 + SATURDAY) % 7);
	if (temp >= (FIRST_MISSING_DAY + NUMBER_MISSING_DAYS))
		return(((temp - 1 + SATURDAY) - NUMBER_MISSING_DAYS) % 7);
	return(THURSDAY);
}

void ascii_day(register char *p, register int day)
{
	register int display, val;
	static char *aday[] = {
		"",
		" 1", " 2", " 3", " 4", " 5", " 6", " 7",
		" 8", " 9", "10", "11", "12", "13", "14",
		"15", "16", "17", "18", "19", "20", "21",
		"22", "23", "24", "25", "26", "27", "28",
		"29", "30", "31",
	};

	if (day == SPACE) {
		memset(p, ' ', julian ? J_DAY_LEN : DAY_LEN);
		return;
	}
	if (julian) {
		if (val = day / 100) {
			day %= 100;
			*p++ = val + '0';
			display = 1;
		} else {
			*p++ = ' ';
			display = 0;
		}
		val = day / 10;
		if (val || display)
			*p++ = val + '0';
		else
			*p++ = ' ';
		*p++ = day % 10 + '0';
	} else {
		*p++ = aday[day][0];
		*p++ = aday[day][1];
	}
	*p = ' ';
}

void trim_trailing_spaces(register char *s)
{
	register char *p;

	for (p = s; *p; ++p);
	while (p > s && isspace(*--p));
	if (p > s)
		++p;
	*p = '\0';
}

void center(char *str, register int len, int separate)
{
	len -= strlen(str);
	(void)printf("%*s%s%*s", len / 2, "", str, len / 2 + len % 2, "");
	if (separate)
		(void)printf("%*s", separate, "");
}

void usage(void)
{
	(void)fprintf(stderr, "usage: cal [-jy] [[month] year]\n");
	exit(1);
}
