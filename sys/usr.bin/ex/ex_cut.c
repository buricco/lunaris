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

/* cut.c */

/* This file contains function which manipulate the cut buffers. */

#include "config.h"
#include "ex_common.h"

# define NANONS	9	/* number of anonymous buffers */

static struct cutbuf
{
	short	*phys;	/* pointer to an array of #s of BLKs containing text */
	int	nblks;	/* number of blocks in phys[] array */
	int	start;	/* offset into first block of start of cut */
	int	end;	/* offset into last block of end of cut */
	int	tmpnum;	/* ID number of the temp file */
	char	lnmode;	/* boolean: line-mode cut? (as opposed to char-mode) */
}
	named[27],	/* cut buffers "a through "z and ". */
	anon[NANONS];	/* anonymous cut buffers */

static char	cbname;	/* name chosen for next cut/paste operation */
static char	dotcb;	/* cut buffer to use if "doingdot" is set */

#ifndef NO_RECYCLE
/* This function builds a list of all blocks needed in the current tmp file
 * for the contents of cut buffers.
 * !!! WARNING: if you have more than ~450000 bytes of text in all of the
 * cut buffers, then this will fail disastrously, because buffer overflow
 * is *not* allowed for.
 */
int cutneeds(need)
	BLK		*need;	/* this is where we deposit the list */
{
	struct cutbuf	*cb;	/* used to count through cut buffers */
	int		i;	/* used to count through blocks of a cut buffer */
	int		n;	/* total number of blocks in list */

	n = 0;

	/* first the named buffers... */
	for (cb = named; cb < &named[27]; cb++)
	{
		if (cb->tmpnum != tmpnum)
			continue;

		for (i = cb->nblks; i-- > 0; )
		{
			need->n[n++] = cb->phys[i];
		}
	}

	/* then the anonymous buffers */
	for (cb = anon; cb < &anon[NANONS]; cb++)
	{
		if (cb->tmpnum != tmpnum)
			continue;

		for (i = cb->nblks; i-- > 0; )
		{
			need->n[n++] = cb->phys[i];
		}
	}

	/* return the length of the list */
	return n;
}
#endif

static void maybezap(int num)
{
	char	cutfname[80];
	int	i;

	/* if this is the current tmp file, then we'd better keep it! */
	if (tmpfd >= 0 && num == tmpnum)
	{
		return;
	}

	/* see if anybody else needs this tmp file */
	for (i = 27; --i >= 0; )
	{
		if (named[i].nblks > 0 && named[i].tmpnum == num)
		{
			break;
		}
	}
	if (i < 0)
	{
		for (i = NANONS; --i >= 0 ; )
		{
			if (anon[i].nblks > 0 && anon[i].tmpnum == num)
			{
				break;
			}
		}
	}

	/* if nobody else needs it, then discard the tmp file */
	if (i < 0)
	{
		sprintf(cutfname, TMPNAME, o_directory, getpid(), num);
		unlink(cutfname);
	}
}

/* This function frees a cut buffer.  If it was the last cut buffer that
 * referred to an old temp file, then it will delete the temp file. */
static void cutfree(struct cutbuf *buf)
{
	int	num;

	/* return immediately if the buffer is already empty */
	if (buf->nblks <= 0)
	{
		return;
	}

	/* else free up stuff */
	num = buf->tmpnum;
	buf->nblks = 0;
#ifdef DEBUG
	if (!buf->phys)
		msg("cutfree() tried to free a NULL buf->phys pointer.");
	else
#endif
	free((char *)buf->phys);

	/* maybe delete the temp file */
	maybezap(num);
}

/* This function is called when we are about to abort a tmp file.
 *
 * To minimize the number of extra files lying around, only named cut buffers
 * are preserved in a file switch; the anonymous buffers just go away.
 */
void cutswitch(void)
{
	int	i;

	/* mark the current temp file as being "obsolete", and close it.  */
	storename((char *)0);
	close(tmpfd);
	tmpfd = -1;

	/* discard all anonymous cut buffers */
#ifndef NO_EXTENSIONS
	if (!o_keepanon)
#endif
	{
		for (i = 0; i < NANONS; i++)
		{
			cutfree(&anon[i]);
		}
	}

	/* delete the temp file, if we don't really need it */
	maybezap(tmpnum);
}

/* This function should be called just before termination of vi */
void cutend(void)
{
	int	i;

	/* free the anonymous buffers, if they aren't already free */
	cutswitch();

	/* free all named cut buffers, since they might be forcing an older
	 * tmp file to be retained.
	 */
	for (i = 0; i < 27; i++)
	{
		cutfree(&named[i]);
	}

#ifndef NO_EXTENSIONS
	/* free all anonymous buffers, same reason */
	for (i = 0; i < NANONS; i++)
	{
		cutfree(&anon[i]);
	}
#endif

	/* delete the temp file */
	maybezap(tmpnum);
}

/* This function is used to select the cut buffer to be used next */
void cutname(int name)
{
	cbname = name;
}

#ifndef NO_LEARN
/* This function appends a single character to a cut buffer; it is used
 * during "learn" mode to record a keystroke.  The buffer to use is determined
 * by an external variable, `learnbuf'; this variable contains the buffer's
 * name while learning, or '\0' if not learning.
 */
void learnkey(char key)
{
	static char	prevlearn;	/* previously learned buffer name */
	static char	buf[BLKSIZE];	/* used for storing keystrokes */
	static int	nkeys;		/* number of keystrokes in buf[] */
	struct cutbuf	*cb;		/* ptr to buffer being saved */
	long		seekpos;	/* where saved cutbuf's text goes */

	/* if we're ending a learn operation, then save keystokes in a cutbuf */
	if (learn != prevlearn && prevlearn)
	{
		/* choose the cutbuffer to use; free its old contents, if any */
		cb = &named[prevlearn - 'a'];
		cutfree(cb);

		/* delete the final "]a" (or whatever) from the keystoke buffer */
		nkeys -= 2;

		/* allocate a BLK for storage of the keystrokes */
		cb->phys = (short *)malloc(sizeof(short));
		cb->nblks = 1;
		cb->start = 0;
		cb->end = nkeys;
		cb->tmpnum = tmpnum;
#ifndef NO_RECYCLE
		seekpos = allocate();
		lseek(tmpfd, seekpos, 0);
#else
		seekpos = lseek(tmpfd, 0L, 2);
#endif
		cb->phys[0] = (short)(seekpos / BLKSIZE);

		/* write the keystokes there */
		if (write(tmpfd, buf, (unsigned)BLKSIZE) != BLKSIZE)
		{
			msg("Trouble writing to tmp file");
			deathtrap(0);
		}

		/* saving complete */
		prevlearn = '\0';
		nkeys = 0;
	}

	/* if we're learning a buffer now, save the keystroke */
	if (learn)
	{
		prevlearn = learn;
		buf[nkeys++] = key;
		if (nkeys >= BLKSIZE - 2)
		{
			msg("Learn buffer full");
			learn = 0;
			nkeys += 2; /* <- to fake "]a" in keystroke buffer */
		}
	}
}
#endif /* !NO_LEARN */

/* This function copies a selected segment of text to a cut buffer */
void cut(MARK from, MARK to)
{
	int		first;	/* logical number of first block in cut */
	int		last;	/* logical number of last block used in cut */
	long		line;	/* a line number */
	int		lnmode;	/* boolean: will this be a line-mode cut? */
	MARK		delthru;/* end of text temporarily inserted for apnd */
	REG struct cutbuf *cb;
	REG long	l;
	REG int		i;
	REG char	*scan;
	char		*blkc;

	/* detect whether this must be a line-mode cut or char-mode cut */
	if (markidx(from) == 0 && markidx(to) == 0)
		lnmode = TRUE;
	else
		lnmode = FALSE;

	/* by default, we don't "delthru" anything */
	delthru = MARK_UNSET;

	/* handle the "doingdot" quirks */
	if (doingdot)
	{
		if (!cbname)
		{
			cbname = dotcb;
		}
	}
	else if (cbname != '.')
	{
		dotcb = cbname;
	}

	/* decide which cut buffer to use */
	if (!cbname)
	{
		/* free up the last anonymous cut buffer */
		cutfree(&anon[NANONS - 1]);

		/* shift the anonymous cut buffers */
		for (i = NANONS - 1; i > 0; i--)
		{
			anon[i] = anon[i - 1];
		}

		/* use the first anonymous cut buffer */
		cb = anon;
		cb->nblks = 0;
	}
	else if (cbname >= 'a' && cbname <= 'z')
	{
		cb = &named[cbname - 'a'];
		cutfree(cb);
	}
#ifndef CRUNCH
	else if (cbname >= 'A' && cbname <= 'Z')
	{
		cb = &named[cbname - 'A'];
		if (cb->nblks > 0)
		{
			/* resolve linemode/charmode differences */
			if (!lnmode && cb->lnmode)
			{
				from &= ~(BLKSIZE - 1);
				if (markidx(to) != 0 || to == from)
				{
					to = to + BLKSIZE - markidx(to);
				}
				lnmode = TRUE;
			}

			/* insert the old cut-buffer before the new text */
			mark[28] = to;
			delthru = paste(from, FALSE, TRUE);
			if (delthru == MARK_UNSET)
			{
				return;
			}
			delthru++;
			to = mark[28];
		}
		cutfree(cb);
	}
#endif /* not CRUNCH */
	else if (cbname == '.')
	{
		cb = &named[26];
		cutfree(cb);
	}
	else
	{
		msg("Invalid cut buffer name: \"%c", cbname);
		dotcb = cbname = '\0';
		return;
	}
	cbname = '\0';
	cb->tmpnum = tmpnum;

	/* detect whether we're doing a line mode cut */
	cb->lnmode = lnmode;

	/* ---------- */

	/* Reporting... */	
	if (markidx(from) == 0 && markidx(to) == 0)
	{
		rptlines = markline(to) - markline(from);
		rptlabel = "yanked";
	}

	/* ---------- */

	/* make sure each block has a physical disk address */
	blksync();

	/* find the first block in the cut */
	line = markline(from);
	for (first = 1; line > lnum[first]; first++)
	{
	}

	/* fetch text of the block containing that line */
	blkc = scan = blkget(first)->c;

	/* find the mark in the block */
	for (l = lnum[first - 1]; ++l < line; )
	{
		while (*scan++ != '\n')
		{
		}
	}
	scan += markidx(from);

	/* remember the offset of the start */
	cb->start = scan - blkc;

	/* ---------- */

	/* find the last block in the cut */
	line = markline(to);
	for (last = first; line > lnum[last]; last++)
	{
	}

	/* fetch text of the block containing that line */
	if (last != first)
	{
		blkc = scan = blkget(last)->c;
	}
	else
	{
		scan = blkc;
	}

	/* find the mark in the block */
	for (l = lnum[last - 1]; ++l < line; )
	{
		while (*scan++ != '\n')
		{
		}
	}
	if (markline(to) <= nlines)
	{
		scan += markidx(to);
	}

	/* remember the offset of the end */
	cb->end = scan - blkc;

	/* ------- */

	/* remember the physical block numbers of all included blocks */
	cb->nblks = last - first;
	if (cb->end > 0)
	{
		cb->nblks++;
	}
	else /*!!!*/
	{
		cb->end = BLKSIZE - 1;
	}
#ifdef lint
	cb->phys = (short *)0;
#else
	cb->phys = (short *)malloc((unsigned)(cb->nblks * sizeof(short)));
#endif
	for (i = 0; i < cb->nblks; i++)
	{
		cb->phys[i] = hdr.n[first++];
	}

#ifndef CRUNCH
	/* if we temporarily inserted text for appending, then delete that
	 * text now -- before the user sees it.
	 */
	if (delthru)
	{
		line = rptlines;
		delete(from, delthru);
		rptlines = line;
		rptlabel = "yanked";
	}
#endif /* not CRUNCH */
}

static void readcutblk(struct cutbuf *cb, int blkno)
{
	char		cutfname[50];/* name of an old temp file */
	int		fd;	/* either tmpfd or the result of open() */

	/* decide which fd to use */
	if (cb->tmpnum == tmpnum)
	{
		fd = tmpfd;
	}
	else
	{
		sprintf(cutfname, TMPNAME, o_directory, getpid(), cb->tmpnum);
		fd = open(cutfname, O_RDONLY);
	}

	/* get the block */
	lseek(fd, (long)cb->phys[blkno] * (long)BLKSIZE, 0);
	if (read(fd, tmpblk.c, (unsigned)BLKSIZE) != BLKSIZE)
	{
		msg("Error reading back from tmp file for pasting!");
	}

	/* close the fd, if it isn't tmpfd */
	if (fd != tmpfd)
	{
		close(fd);
	}
}

/* This function inserts text from a cut buffer, and returns the MARK where
 * insertion ended.  Return MARK_UNSET on errors.
 */
MARK paste(MARK at, int after, int retend)
{
	REG struct cutbuf	*cb;
	REG int			i;

	/* handle the "doingdot" quirks */
	if (doingdot)
	{
		if (!cbname)
		{
			if (dotcb >= '1' && dotcb < '1' + NANONS - 1)
			{
				dotcb++;
			}
			cbname = dotcb;
		}
	}
	else if (cbname != '.')
	{
		dotcb = cbname;
	}

	/* decide which cut buffer to use */
	if (cbname >= 'A' && cbname <= 'Z')
	{
		cb = &named[cbname - 'A'];
	}
	else if (cbname >= 'a' && cbname <= 'z')
	{
		cb = &named[cbname - 'a'];
	}
	else if (cbname >= '1' && cbname <= '9')
	{
		cb = &anon[cbname - '1'];
	}
	else if (cbname == '.')
	{
		cb = &named[26];
	}
	else if (!cbname)
	{
		cb = anon;
	}
	else
	{
		msg("Invalid cut buffer name: \"%c", cbname);
		cbname = '\0';
		return MARK_UNSET;
	}

	/* make sure it isn't empty */
	if (cb->nblks == 0)
	{
		if (cbname)
			msg("Cut buffer \"%c is empty", cbname);
		else
			msg("Cut buffer is empty");
		cbname = '\0';
		return MARK_UNSET;
	}
	cbname = '\0';

	/* adjust the insertion MARK for "after" and line-mode cuts */
	if (cb->lnmode)
	{
		at &= ~(BLKSIZE - 1);
		if (after)
		{
			at += BLKSIZE;
		}
#if 1
		/* screen updates still have a glitch.  Force total redraw */
		if (!o_optimize)
		{
			redrawrange(at, INFINITY, INFINITY);
		}
#endif
	}
	else if (after)
	{
		/* careful! if markidx(at) == 0 we might be pasting into an
		 * empty line -- so we can't blindly increment "at".
		 */
		if (markidx(at) == 0)
		{
			pfetch(markline(at));
			if (plen != 0)
			{
				at++;
			}
		}
		else
		{
			at++;
		}
	}

	/* put a copy of the "at" mark in the mark[] array, so it stays in
	 * sync with changes made via add().
	 */
	mark[27] = at;

	/* simple one-block paste? */
	if (cb->nblks == 1)
	{
		/* get the block */
		readcutblk(cb, 0);

		/* isolate the text we need within it */
		if (cb->end)
		{
			tmpblk.c[cb->end] = '\0';
		}

		/* insert it */
		ChangeText
		{
			add(at, &tmpblk.c[cb->start]);
		}
	}
	else
	{
		/* multi-block paste */

		ChangeText
		{
			i = cb->nblks - 1;

			/* add text from the last block first */
			if (cb->end > 0)
			{
				readcutblk(cb, i);
				tmpblk.c[cb->end] = '\0';
				add(at, tmpblk.c);
				i--;
			}

			/* add intervening blocks */
			while (i > 0)
			{
				readcutblk(cb, i);
				add(at, tmpblk.c);
				i--;
			}

			/* add text from the first cut block */
			readcutblk(cb, 0);
			add(at, &tmpblk.c[cb->start]);
		}
	}

	/* Reporting... */
	rptlines = markline(mark[27]) - markline(at);
	rptlabel = "pasted";

	/* return the mark at the beginning/end of inserted text */
	if (retend)
	{
		return mark[27] - 1L;
	}
	return at;
}

#ifndef NO_AT

/* This function copies characters from a cut buffer into a string.
 * It returns the number of characters in the cut buffer.  If the cut
 * buffer is too large to fit in the string (i.e. if cb2str() returns
 * a number >= size) then the characters will not have been copied.
 * It returns 0 if the cut buffer is empty, and -1 for invalid cut buffers.
 */
int cb2str(int name, char *buf, unsigned size)
{
	REG struct cutbuf	*cb;
	REG char		*src;
	REG char		*dest;

	/* decide which cut buffer to use */
	if (name >= 'a' && name <= 'z')
	{
		cb = &named[name - 'a'];
	}
#if 1 /* [sdw] allow selection of anonymous buffer */
	else if (!name)
	{
		cb = anon;
	}
#endif
	else
	{
		return -1;
	}

	/* if the buffer is empty, return 0 */
	if (cb->nblks == 0)
	{
		return 0;
	}

	/* !!! if not a single-block cut, then fail */
	if (cb->nblks != 1)
	{
		return size;
	}

	/* if too big, return the size now, without doing anything */
	if ((unsigned)(cb->end - cb->start) >= size)
	{
		return cb->end - cb->start;
	}

	/* get the block */
	readcutblk(cb, 0);

	/* isolate the string within that blk */
	if (cb->start == 0)
	{
		tmpblk.c[cb->end] = '\0';
	}
	else
	{
		for (dest = tmpblk.c, src = dest + cb->start; src < tmpblk.c + cb->end; )
		{
			*dest++ = *src++;
		}
		*dest = '\0';
	}

	/* copy the string into the buffer */
	if (buf != tmpblk.c)
	{
		strcpy(buf, tmpblk.c);
	}

	/* return the length */
	return cb->end - cb->start;
}
#endif
