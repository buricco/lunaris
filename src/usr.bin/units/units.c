/*	$OpenBSD: units.c,v 1.22 2015/10/09 01:37:09 deraadt Exp $	*/
/*	$NetBSD: units.c,v 1.6 1996/04/06 06:01:03 thorpej Exp $	*/

/*
 * units.c   Copyright (c) 1993 by Adrian Mariano (adrian@cam.cornell.edu)
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 * Disclaimer:  This software is provided by the author "as is".  The author
 * shall not be liable for any damages caused in any way by this software.
 *
 * I would appreciate (though I do not require) receiving a copy of any
 * improvements you might make to this program.
 */

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <err.h>

#define UNITSFILE "/usr/share/misc/units.lib"

#define VERSION "1.0"

#define MAXUNITS 1000
#define MAXPREFIXES 100

#define MAXSUBUNITS 500

#define PRIMITIVECHAR '!'

char *powerstring = "^";

struct {
	char *uname;
	char *uval;
} unittable[MAXUNITS];

struct unittype {
	char *numerator[MAXSUBUNITS];
	char *denominator[MAXSUBUNITS];
	double factor;
};

struct {
	char *prefixname;
	char *prefixval;
} prefixtable[MAXPREFIXES];


char *NULLUNIT = "";

int unitcount;
int prefixcount;

char *dupstr(char *);
void readunits(char *);
void initializeunit(struct unittype *);
int addsubunit(char *[], char *);
void showunit(struct unittype *);
void zeroerror(void);
int addunit(struct unittype *, char *, int);
int compare(const void *, const void *);
void sortunit(struct unittype *);
void cancelunit(struct unittype *);
char *lookupunit(char *);
int reduceproduct(struct unittype *, int);
int reduceunit(struct unittype *);
int compareproducts(char **, char **);
int compareunits(struct unittype *, struct unittype *);
int completereduce(struct unittype *);
void showanswer(struct unittype *, struct unittype *);
void usage(void);

/* uso */
size_t
int_strlcpy(char *d, char *s, size_t n)
{
	size_t t;

	for (t=0; t<n; t++) {
		d[t]=s[t];
		if (!s[t])
			return t;
	}
	d[n-1]=0;
	return n;
}

/* uso */
size_t
int_strlcat(char *d, char *s, size_t n)
{
    size_t t;
    char *nd;
    
    nd=d+strlen(d);
    
    t=int_strlcpy(nd, s, n-strlen(d));
    
    return t+strlen(d);
}

char *
dupstr(char *str)
{
	char *ret;

	ret = strdup(str);
	if (!ret) {
		fprintf(stderr, "Memory allocation error\n");
		exit(3);
	}
	return (ret);
}


void
readunits(char *userfile)
{
	char line[512], *lineptr;
	int len, linenum, i;
	FILE *unitfile;

	unitcount = 0;
	linenum = 0;

	if (userfile) {
		unitfile = fopen(userfile, "r");
		if (!unitfile) {
			fprintf(stderr, "Unable to open units file '%s'\n",
			    userfile);
			exit(1);
		}
	} else {
		unitfile = fopen(UNITSFILE, "r");
		if (!unitfile) {
			fprintf(stderr, "Can't find units file '%s'\n",
			    UNITSFILE);
			exit(1);
		}
	}
	while (!feof(unitfile)) {
		if (!fgets(line, sizeof(line), unitfile))
			break;
		linenum++;
		lineptr = line;
		if (*lineptr == '/')
			continue;
		lineptr += strspn(lineptr, " \n\t");
		len = strcspn(lineptr, " \n\t");
		lineptr[len] = 0;
		if (!strlen(lineptr))
			continue;
		if (lineptr[strlen(lineptr) - 1] == '-') { /* it's a prefix */
			if (prefixcount == MAXPREFIXES) {
				fprintf(stderr,
				    "Memory for prefixes exceeded in line %d\n",
				    linenum);
				continue;
			}

			lineptr[strlen(lineptr) - 1] = 0;
			for (i = 0; i < prefixcount; i++) {
				if (!strcmp(prefixtable[i].prefixname, lineptr))
					break;
			}
			if (i < prefixcount) {
				fprintf(stderr, "Redefinition of prefix '%s' "
				    "on line %d ignored\n", lineptr, linenum);
				continue;	/* skip duplicate prefix */
			}

			prefixtable[prefixcount].prefixname = dupstr(lineptr);
			lineptr += len + 1;
			lineptr += strspn(lineptr, " \n\t");
			len = strcspn(lineptr, "\n\t");
			if (len == 0) {
				fprintf(stderr, "Unexpected end of prefix on "
				    "line %d\n", linenum);
				free(prefixtable[prefixcount].prefixname);
				continue;
			}
			lineptr[len] = 0;
			prefixtable[prefixcount++].prefixval = dupstr(lineptr);
		} else {		/* it's not a prefix */
			if (unitcount == MAXUNITS) {
				fprintf(stderr,
				    "Memory for units exceeded in line %d\n",
				    linenum);
				continue;
			}

			for (i = 0; i < unitcount; i++) {
				if (!strcmp(unittable[i].uname, lineptr))
					break;
			}
			if (i < unitcount) {
				fprintf(stderr, "Redefinition of unit '%s' "
				    "on line %d ignored\n", lineptr, linenum);
				continue;	/* skip duplicate unit */
			}

			unittable[unitcount].uname = dupstr(lineptr);
			lineptr += len + 1;
			lineptr += strspn(lineptr, " \n\t");
			if (!strlen(lineptr)) {
				fprintf(stderr, "Unexpected end of unit on "
				    "line %d\n", linenum);
				free(unittable[unitcount].uname);
				continue;
			}
			len = strcspn(lineptr, "\n\t");
			lineptr[len] = 0;
			unittable[unitcount++].uval = dupstr(lineptr);
		}
	}
	fclose(unitfile);
}

void
initializeunit(struct unittype *theunit)
{
	theunit->factor = 1.0;
	theunit->numerator[0] = theunit->denominator[0] = NULL;
}


int
addsubunit(char *product[], char *toadd)
{
	char **ptr;

	for (ptr = product; *ptr && *ptr != NULLUNIT; ptr++);
	if (ptr >= product + MAXSUBUNITS) {
		fprintf(stderr, "Memory overflow in unit reduction\n");
		return 1;
	}
	if (!*ptr)
		*(ptr + 1) = 0;
	*ptr = dupstr(toadd);
	return 0;
}


void
showunit(struct unittype *theunit)
{
	char **ptr;
	int printedslash;
	int counter = 1;

	printf("\t%.8g", theunit->factor);
	for (ptr = theunit->numerator; *ptr; ptr++) {
		if (ptr > theunit->numerator && **ptr &&
		    !strcmp(*ptr, *(ptr - 1)))
			counter++;
		else {
			if (counter > 1)
				printf("%s%d", powerstring, counter);
			if (**ptr)
				printf(" %s", *ptr);
			counter = 1;
		}
	}
	if (counter > 1)
		printf("%s%d", powerstring, counter);
	counter = 1;
	printedslash = 0;
	for (ptr = theunit->denominator; *ptr; ptr++) {
		if (ptr > theunit->denominator && **ptr &&
		    !strcmp(*ptr, *(ptr - 1)))
			counter++;
		else {
			if (counter > 1)
				printf("%s%d", powerstring, counter);
			if (**ptr) {
				if (!printedslash)
					printf(" /");
				printedslash = 1;
				printf(" %s", *ptr);
			}
			counter = 1;
		}
	}
	if (counter > 1)
		printf("%s%d", powerstring, counter);
	printf("\n");
}


void
zeroerror(void)
{
	fprintf(stderr, "Unit reduces to zero\n");
}

/*
   Adds the specified string to the unit.
   Flip is 0 for adding normally, 1 for adding reciprocal.

   Returns 0 for successful addition, nonzero on error.
*/

int
addunit(struct unittype *theunit, char *toadd, int flip)
{
	char *scratch, *savescr;
	char *item;
	char *divider, *slash;
	int doingtop;

	savescr = scratch = dupstr(toadd);
	for (slash = scratch + 1; *slash; slash++)
		if (*slash == '-' &&
		    (tolower((unsigned char)*(slash - 1)) != 'e' ||
		    !strchr(".0123456789", *(slash + 1))))
			*slash = ' ';
	slash = strchr(scratch, '/');
	if (slash)
		*slash = 0;
	doingtop = 1;
	do {
		item = strtok(scratch, " *\t\n/");
		while (item) {
			if (strchr("0123456789.", *item)) { /* item is a number */
				double num;

				divider = strchr(item, '|');
				if (divider) {
					*divider = 0;
					num = atof(item);
					if (!num) {
						zeroerror();
						free(savescr);
						return 1;
					}
					if (doingtop ^ flip)
						theunit->factor *= num;
					else
						theunit->factor /= num;
					num = atof(divider + 1);
					if (!num) {
						zeroerror();
						free(savescr);
						return 1;
					}
					if (doingtop ^ flip)
						theunit->factor /= num;
					else
						theunit->factor *= num;
				} else {
					num = atof(item);
					if (!num) {
						zeroerror();
						free(savescr);
						return 1;
					}
					if (doingtop ^ flip)
						theunit->factor *= num;
					else
						theunit->factor /= num;

				}
			} else {	/* item is not a number */
				int repeat = 1;

				if (strchr("23456789",
				    item[strlen(item) - 1])) {
					repeat = item[strlen(item) - 1] - '0';
					item[strlen(item) - 1] = 0;
				}
				for (; repeat; repeat--)
					if (addsubunit(doingtop ^ flip
					    ? theunit->numerator
					    : theunit->denominator, item)) {
						free(savescr);
						return 1;
					}
			}
			item = strtok(NULL, " *\t/\n");
		}
		doingtop--;
		if (slash) {
			scratch = slash + 1;
		} else
			doingtop--;
	} while (doingtop >= 0);
	free(savescr);
	return 0;
}


int
compare(const void *item1, const void *item2)
{
	return strcmp(*(char **) item1, *(char **) item2);
}


void
sortunit(struct unittype *theunit)
{
	char **ptr;
	int count;

	for (count = 0, ptr = theunit->numerator; *ptr; ptr++, count++);
	qsort(theunit->numerator, count, sizeof(char *), compare);
	for (count = 0, ptr = theunit->denominator; *ptr; ptr++, count++);
	qsort(theunit->denominator, count, sizeof(char *), compare);
}


void
cancelunit(struct unittype *theunit)
{
	char **den, **num;
	int comp;

	den = theunit->denominator;
	num = theunit->numerator;

	while (*num && *den) {
		comp = strcmp(*den, *num);
		if (!comp) {
			*den++ = NULLUNIT;
			*num++ = NULLUNIT;
		} else if (comp < 0)
			den++;
		else
			num++;
	}
}




/*
   Looks up the definition for the specified unit.
   Returns a pointer to the definition or a null pointer
   if the specified unit does not appear in the units table.
*/

static char buffer[500];	/* buffer for lookupunit answers with
				   prefixes */

char *
lookupunit(char *unit)
{
	size_t len;
	int i;
	char *copy;

	for (i = 0; i < unitcount; i++) {
		if (!strcmp(unittable[i].uname, unit))
			return unittable[i].uval;
	}

	len = strlen(unit);
	if (len == 0)
		return NULL;
	if (unit[len - 1] == '^') {
		copy = dupstr(unit);
		copy[len - 1] = '\0';
		for (i = 0; i < unitcount; i++) {
			if (!strcmp(unittable[i].uname, copy)) {
				int_strlcpy(buffer, copy, sizeof(buffer));
				free(copy);
				return buffer;
			}
		}
		free(copy);
	}
	if (unit[len - 1] == 's') {
		copy = dupstr(unit);
		copy[len - 1] = '\0';
		--len;
		for (i = 0; i < unitcount; i++) {
			if (!strcmp(unittable[i].uname, copy)) {
				int_strlcpy(buffer, copy, sizeof(buffer));
				free(copy);
				return buffer;
			}
		}
		if (len != 0 && copy[len - 1] == 'e') {
			copy[len - 1] = 0;
			for (i = 0; i < unitcount; i++) {
				if (!strcmp(unittable[i].uname, copy)) {
					int_strlcpy(buffer, copy, sizeof(buffer));
					free(copy);
					return buffer;
				}
			}
		}
		free(copy);
	}
	for (i = 0; i < prefixcount; i++) {
		len = strlen(prefixtable[i].prefixname);
		if (!strncmp(prefixtable[i].prefixname, unit, len)) {
			if (!strlen(unit + len) || lookupunit(unit + len)) {
				snprintf(buffer, sizeof(buffer), "%s %s",
				    prefixtable[i].prefixval, unit + len);
				return buffer;
			}
		}
	}
	return NULL;
}



/*
   reduces a product of symbolic units to primitive units.
   The three low bits are used to return flags:

     bit 0 (1) set on if reductions were performed without error.
     bit 1 (2) set on if no reductions are performed.
     bit 2 (4) set on if an unknown unit is discovered.
*/


#define ERROR 4

int
reduceproduct(struct unittype *theunit, int flip)
{
	char *toadd, **product;
	int didsomething = 2;

	if (flip)
		product = theunit->denominator;
	else
		product = theunit->numerator;

	for (; *product; product++) {

		for (;;) {
			if (!strlen(*product))
				break;
			toadd = lookupunit(*product);
			if (!toadd) {
				printf("unknown unit '%s'\n", *product);
				return ERROR;
			}
			if (strchr(toadd, PRIMITIVECHAR))
				break;
			didsomething = 1;
			if (*product != NULLUNIT) {
				free(*product);
				*product = NULLUNIT;
			}
			if (addunit(theunit, toadd, flip))
				return ERROR;
		}
	}
	return didsomething;
}


/*
   Reduces numerator and denominator of the specified unit.
   Returns 0 on success, or 1 on unknown unit error.
*/

int
reduceunit(struct unittype *theunit)
{
	int ret;

	ret = 1;
	while (ret & 1) {
		ret = reduceproduct(theunit, 0) | reduceproduct(theunit, 1);
		if (ret & 4)
			return 1;
	}
	return 0;
}


int
compareproducts(char **one, char **two)
{
	while (*one || *two) {
		if (!*one && *two != NULLUNIT)
			return 1;
		if (!*two && *one != NULLUNIT)
			return 1;
		if (*one == NULLUNIT)
			one++;
		else if (*two == NULLUNIT)
			two++;
		else if (strcmp(*one, *two))
			return 1;
		else
			one++, two++;
	}
	return 0;
}


/* Return zero if units are compatible, nonzero otherwise */

int
compareunits(struct unittype *first, struct unittype *second)
{
	return compareproducts(first->numerator, second->numerator) ||
	    compareproducts(first->denominator, second->denominator);
}


int
completereduce(struct unittype *unit)
{
	if (reduceunit(unit))
		return 1;
	sortunit(unit);
	cancelunit(unit);
	return 0;
}


void
showanswer(struct unittype *have, struct unittype *want)
{
	if (compareunits(have, want)) {
		printf("conformability error\n");
		showunit(have);
		showunit(want);
	} else
		printf("\t* %.8g\n\t/ %.8g\n", have->factor / want->factor,
		    want->factor / have->factor);
}


void
usage(void)
{
	fprintf(stderr,
	    "usage: units [-qv] [-f filename] [[count] from-unit to-unit]\n");
	exit(3);
}


int
main(int argc, char **argv)
{

	struct unittype have, want;
	char havestr[81], wantstr[81];
	int optchar;
	char *userfile = 0;
	int quiet = 0;

	extern char *optarg;
	extern int optind;

	while ((optchar = getopt(argc, argv, "vqf:")) != -1) {
		switch (optchar) {
		case 'f':
			userfile = optarg;
			break;
		case 'q':
			quiet = 1;
			break;
		case 'v':
			fprintf(stderr,
			    "units version %s Copyright (c) 1993 by Adrian Mariano\n",
			    VERSION);
			fprintf(stderr,
			    "This program may be freely distributed\n");
			usage();
		default:
			usage();
			break;
		}
	}

	argc -= optind;
	argv += optind;

	if (argc != 3 && argc != 2 && argc != 0)
		usage();

	readunits(userfile);

	if (argc == 3) {
		int_strlcpy(havestr, argv[0], sizeof(havestr));
		int_strlcat(havestr, " ", sizeof(havestr));
		int_strlcat(havestr, argv[1], sizeof(havestr));
		argc--;
		argv++;
		argv[0] = havestr;
	}

	if (argc == 2) {
		int_strlcpy(havestr, argv[0], sizeof(havestr));
		int_strlcpy(wantstr, argv[1], sizeof(wantstr));
		initializeunit(&have);
		addunit(&have, havestr, 0);
		completereduce(&have);
		initializeunit(&want);
		addunit(&want, wantstr, 0);
		completereduce(&want);
		showanswer(&have, &want);
	} else {
		if (!quiet)
			printf("%d units, %d prefixes\n", unitcount,
			    prefixcount);
		for (;;) {
			do {
				initializeunit(&have);
				if (!quiet)
					printf("You have: ");
				if (!fgets(havestr, sizeof(havestr), stdin)) {
					if (!quiet)
						putchar('\n');
					exit(0);
				}
			} while (addunit(&have, havestr, 0) ||
			    completereduce(&have));
			do {
				initializeunit(&want);
				if (!quiet)
					printf("You want: ");
				if (!fgets(wantstr, sizeof(wantstr), stdin)) {
					if (!quiet)
						putchar('\n');
					exit(0);
				}
			} while (addunit(&want, wantstr, 0) ||
			    completereduce(&want));
			showanswer(&have, &want);
		}
	}
	return (0);
}
