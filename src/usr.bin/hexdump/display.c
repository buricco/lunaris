/*
 * Copyright (c) 1989, 1993
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

#include <sys/param.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <ctype.h>
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "hexdump.h"

enum _vflag vflag = FIRST;

static off_t address;			/* address/offset in stream */
static off_t eaddress;			/* end address */

static __inline void print(PR *, u_char *);

void
display(void)
{
	FS *fs;
	FU *fu;
	PR *pr;
	int cnt;
	u_char *bp;
	off_t saveaddress;
	u_char savech, *savebp;

	while ((bp = get()))
	    for (fs = fshead, savebp = bp, saveaddress = address; fs;
		fs = fs->nextfs, bp = savebp, address = saveaddress)
		    for (fu = fs->nextfu; fu; fu = fu->nextfu) {
			if (fu->flags&F_IGNORE)
				break;
			for (cnt = fu->reps; cnt; --cnt)
			    for (pr = fu->nextpr; pr; address += pr->bcnt,
				bp += pr->bcnt, pr = pr->nextpr) {
				    if (eaddress && address >= eaddress &&
					!(pr->flags & (F_TEXT|F_BPAD)))
					    bpad(pr);
				    if (cnt == 1 && pr->nospace) {
					savech = *pr->nospace;
					*pr->nospace = '\0';
				    }
				    print(pr, bp);
				    if (cnt == 1 && pr->nospace)
					*pr->nospace = savech;
			    }
		    }
	if (endfu) {
		/*
		 * If eaddress not set, error or file size was multiple of
		 * blocksize, and no partial block ever found.
		 */
		if (!eaddress) {
			if (!address)
				return;
			eaddress = address;
		}
		for (pr = endfu->nextpr; pr; pr = pr->nextpr)
			switch(pr->flags) {
			case F_ADDRESS:
				(void)printf(pr->fmt, (quad_t)eaddress);
				break;
			case F_TEXT:
				(void)printf("%s", pr->fmt);
				break;
			}
	}
}

static __inline void
print(PR *pr, u_char *bp)
{
	long double ldbl;
	   double f8;
	    float f4;
	  int16_t s2;
	   int8_t s8;
	  int32_t s4;
	u_int16_t u2;
	u_int32_t u4;
	u_int64_t u8;

	switch(pr->flags) {
	case F_ADDRESS:
		(void)printf(pr->fmt, (quad_t)address);
		break;
	case F_BPAD:
		(void)printf(pr->fmt, "");
		break;
	case F_C:
		conv_c(pr, bp);
		break;
	case F_CHAR:
		(void)printf(pr->fmt, *bp);
		break;
	case F_DBL:
		switch(pr->bcnt) {
		case 4:
			bcopy(bp, &f4, sizeof(f4));
			(void)printf(pr->fmt, f4);
			break;
		case 8:
			bcopy(bp, &f8, sizeof(f8));
			(void)printf(pr->fmt, f8);
			break;
		default:
			if (pr->bcnt == sizeof(long double)) {
				bcopy(bp, &ldbl, sizeof(ldbl));
				(void)printf(pr->fmt, ldbl);
			}
			break;
		}
		break;
	case F_INT:
		switch(pr->bcnt) {
		case 1:
			(void)printf(pr->fmt, (quad_t)(signed char)*bp);
			break;
		case 2:
			bcopy(bp, &s2, sizeof(s2));
			(void)printf(pr->fmt, (quad_t)s2);
			break;
		case 4:
			bcopy(bp, &s4, sizeof(s4));
			(void)printf(pr->fmt, (quad_t)s4);
			break;
		case 8:
			bcopy(bp, &s8, sizeof(s8));
			(void)printf(pr->fmt, s8);
			break;
		}
		break;
	case F_P:
		(void)printf(pr->fmt, isprint(*bp) ? *bp : '.');
		break;
	case F_STR:
		(void)printf(pr->fmt, (char *)bp);
		break;
	case F_TEXT:
		(void)printf("%s", pr->fmt);
		break;
	case F_U:
		conv_u(pr, bp);
		break;
	case F_UINT:
		switch(pr->bcnt) {
		case 1:
			(void)printf(pr->fmt, (u_quad_t)*bp);
			break;
		case 2:
			bcopy(bp, &u2, sizeof(u2));
			(void)printf(pr->fmt, (u_quad_t)u2);
			break;
		case 4:
			bcopy(bp, &u4, sizeof(u4));
			(void)printf(pr->fmt, (u_quad_t)u4);
			break;
		case 8:
			bcopy(bp, &u8, sizeof(u8));
			(void)printf(pr->fmt, u8);
			break;
		}
		break;
	}
}

void
bpad(PR *pr)
{
	static char const *spec = " -0+#";
	char *p1, *p2;

	/*
	 * Remove all conversion flags; '-' is the only one valid
	 * with %s, and it's not useful here.
	 */
	pr->flags = F_BPAD;
	pr->cchar[0] = 's';
	pr->cchar[1] = '\0';
	for (p1 = pr->fmt; *p1 != '%'; ++p1);
	for (p2 = ++p1; *p1 && index(spec, *p1); ++p1);
	while ((*p2++ = *p1++));
}

static char **_argv;

u_char *
get(void)
{
	static int ateof = 1;
	static u_char *curp, *savp;
	int n;
	int need, nread;
	int valid_save = 0;
	u_char *tmpp;

	if (!curp) {
		if ((curp = calloc(1, blocksize)) == NULL)
			err(1, NULL);
		if ((savp = calloc(1, blocksize)) == NULL)
			err(1, NULL);
	} else {
		tmpp = curp;
		curp = savp;
		savp = tmpp;
		address += blocksize;
		valid_save = 1;
	}
	for (need = blocksize, nread = 0;;) {
		/*
		 * if read the right number of bytes, or at EOF for one file,
		 * and no other files are available, zero-pad the rest of the
		 * block and set the end flag.
		 */
		if (!length || (ateof && !next((char **)NULL))) {
			if (odmode && address < skip)
				errx(1, "cannot skip past end of input");
			if (need == blocksize)
				return((u_char *)NULL);
			if (vflag != ALL && 
			    valid_save && 
			    nread == blocksize &&	/* fixes #110370 */
			    bcmp(curp, savp, nread) == 0) {
				if (vflag != DUP)
					(void)printf("*\n");
				return((u_char *)NULL);
			}
			bzero((char *)curp + nread, need);
			eaddress = address + nread;
			return(curp);
		}
		n = read(0, curp + nread, length == -1 ? need : MIN(length, need));
		if (n <= 0) {
			ateof = 1;
			continue;
			if (n < 0)
				warn("%s", _argv[-1]);
		}
		ateof = 0;
		if (length != -1)
			length -= n;
		if (!(need -= n)) {
			if (vflag == ALL || vflag == FIRST ||
			    valid_save == 0 ||
			    bcmp(curp, savp, blocksize) != 0) {
				if (vflag == DUP || vflag == FIRST)
					vflag = WAIT;
				return(curp);
			}
			if (vflag == WAIT)
				(void)printf("*\n");
			vflag = DUP;
			address += blocksize;
			need = blocksize;
			nread = 0;
		}
		else
			nread += n;
	}
}

int
next(char **argv)
{
	static int done;
	int statok;
	int fd;

	if (argv) {
		_argv = argv;
		return(1);
	}
	for (;;) {
		if (*_argv) {
			if ((fd = open(*_argv, O_RDONLY)) == -1 ||
			    close(0) == -1 ||
			    dup2(fd, 0) == -1 ||
			    close(fd) == -1) {
				warn("%s", *_argv);
				exitval = 1;
				++_argv;
				continue;
			}
			statok = done = 1;
		} else {
			if (done++)
				return(0);
			statok = 0;
		}
		if (skip)
			doskip(statok ? *_argv : "stdin", statok);
		if (*_argv)
			++_argv;
		if (!skip)
			return(1);
	}
	/* NOTREACHED */
}

void
doskip(const char *fname, int statok)
{
	int cnt;
	struct stat sb;
	char ch;

	if (statok) {
		if (fstat(0, &sb))
			err(1, "%s", fname);
		if (S_ISREG(sb.st_mode) && skip >= sb.st_size) {
			address += sb.st_size;
			skip -= sb.st_size;
			return;
		}
	}
	if (S_ISREG(sb.st_mode)) {
		if ((lseek(0, skip, SEEK_SET)) == -1)
			err(1, "%s", fname);
		address += skip;
		skip = 0;
	} else {
		for (cnt = 0; cnt < skip; ++cnt)
			if (read(0, &ch, 1) != 1)
				break;
		address += cnt;
		skip -= cnt;
	}
}
