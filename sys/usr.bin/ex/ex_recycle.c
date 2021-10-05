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

/* recycle.c */

/* This file contains the functions perform garbage collection and allocate
 * reusable blocks.
 */

#include "config.h"
#include "ex_common.h"

#ifndef NO_RECYCLE
/* this whole file would have be skipped if NO_RECYCLE is defined */

#define BTST(bitno, byte)	((byte) & (1 << (bitno)))
#define BSET(bitno, byte)	((byte) |= (1 << (bitno)))
#define BCLR(bitno, byte)	((byte) &= ~(1 << (bitno)))

#define TST(blkno)		((blkno) < MAXBIT ? BTST((blkno) & 7, bitmap[(blkno) >> 3]) : 1)
#define SET(blkno)		if ((blkno) < MAXBIT) BSET((blkno) & 7, bitmap[(blkno) >> 3])
#define CLR(blkno)		if ((blkno) < MAXBIT) BCLR((blkno) & 7, bitmap[(blkno) >> 3])

/* bitmap of free blocks in first 4096k of tmp file */
static unsigned char bitmap[512];
#define MAXBIT	(sizeof bitmap << 3)

/* this function locates all free blocks in the current tmp file */
void garbage(void)
{
	int	i;
	BLK	oldhdr;

	/* start by assuming every block is free */
	for (i = 0; i < sizeof bitmap; i++)
	{
		bitmap[i] = 255;
	}

	/* header blocks aren't free */
#ifndef lint
	CLR(0);
	CLR(1);
#endif

	/* blocks needed for current hdr aren't free */
	for (i = 1; i < MAXBLKS; i++)
	{
		CLR(hdr.n[i]);
	}

	/* blocks needed for undo version aren't free */
	lseek(tmpfd, 0L, 0);
	if (read(tmpfd, &oldhdr, (unsigned)sizeof oldhdr) != sizeof oldhdr)
	{
		msg("garbage() failed to read oldhdr??");
		for (i = 0; i < sizeof bitmap; i++)
		{
			bitmap[i] = 0;
		}
		return;
	}
	for (i = 1; i < MAXBLKS; i++)
	{
		CLR(oldhdr.n[i]);
	}

	/* blocks needed for cut buffers aren't free */
	for (i = cutneeds(&oldhdr) - 1; i >= 0; i--)
	{
		CLR(oldhdr.n[i]);
	}
}

/* This function allocates the first available block in the tmp file */
long allocate(void)
{
	int	i;
	long	offset;

	/* search for the first byte with a free bit set */
	for (i = 0; i < sizeof bitmap && bitmap[i] == 0; i++)
	{
	}

	/* if we hit the end of the bitmap, return the end of the file */
	if (i == sizeof bitmap)
	{
		offset = lseek(tmpfd, 0L, 2);
	}
	else /* compute the offset for the free block */
	{
		for (i <<= 3; TST(i) == 0; i++)
		{
		}
		offset = (long)i * (long)BLKSIZE;

		/* mark the block as "allocated" */
		CLR(i);
	}

	return offset;
}

#endif

#ifdef DEBUG
# include <stdio.h>
# undef malloc
# undef free
# define MEMMAGIC 0x19f72cc0L
# define MAXALLOC 800
static char *allocated[MAXALLOC];
static char *fromfile[MAXALLOC];
static int  fromline[MAXALLOC]; 
static int  sizes[MAXALLOC];

char *dbmalloc(int size, char *file, int line)
{
	char	*ret;
	int	i;

	size = size + sizeof(long) - (size % sizeof(long));
	ret = (char *)malloc(size + 2 * sizeof(long)) + sizeof(long);
	for (i = 0; i < MAXALLOC && allocated[i]; i++)
	{
	}
	if (i == MAXALLOC)
	{
		endwin();
		fprintf(stderr, "\r\n%s(%d): Too many malloc calls!\n", file, line);
		abort();
	}
	sizes[i] = size/sizeof(long);
	allocated[i] = ret;
	fromfile[i] = file;
	fromline[i] = line;
	((long *)ret)[-1] = MEMMAGIC;
	((long *)ret)[sizes[i]] = MEMMAGIC;
	return ret;
}

void dbfree(char *ptr, char *file, int line)
{
	int	i;

	for (i = 0; i < MAXALLOC && allocated[i] != ptr; i++)
	{
	}
	if (i == MAXALLOC)
	{
		endwin();
		fprintf(stderr, "\r\n%s(%d): attempt to free mem that wasn't allocated\n", file, line);
		abort();
	}
	allocated[i] = (char *)0;
	if (((long *)ptr)[-1] != MEMMAGIC)
	{
		endwin();
		fprintf(stderr, "\r\n%s(%d): underflowed malloc space, allocated at %s(%d)\n", file, line, fromfile[i], fromline[i]);
		abort();
	}
	if (((long *)ptr)[sizes[i]] != MEMMAGIC)
	{
		endwin();
		fprintf(stderr, "\r\n%s(%d): overflowed malloc space, allocated at %s(%d)\n", file, line, fromfile[i], fromline[i]);
		abort();
	}
	free(ptr - sizeof(long));
}

void dbcheckmem(char *file, int line)
{
	int	i, j;

	for (i = j = 0; i < MAXALLOC && allocated[i]; i++)
	{
		if (((long *)allocated[i])[-1] != MEMMAGIC)
		{
			if (!j) endwin();
			fprintf(stderr, "\r\n%s(%d): underflowed malloc space, allocated at %s(%d)\n", file, line, fromfile[i], fromline[i]);
			j++;
		}
		if (((long *)allocated[i])[sizes[i]] != MEMMAGIC)
		{
			if (!j) endwin();
			fprintf(stderr, "\r\n%s(%d): overflowed malloc space, allocated at %s(%d)\n", file, line, fromfile[i], fromline[i]);
			j++;
		}
	}
	if (j)
	{
		abort();
	}
}
#endif
