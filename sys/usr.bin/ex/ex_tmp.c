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

/* tmp.c */

/* This file contains functions which create & readback a TMPFILE */

#ifdef AIX
# define _XOPEN_SOURCE
# include <sys/mode.h>
# include <sys/stat.h>
#endif
#include "config.h"
#include "ex_common.h"
#   include <sys/stat.h>

#ifndef NO_MODELINES
static void do_modelines(l, stop)
	long	l;	/* line number to start at */
	long	stop;	/* line number to stop at */
{
	char	*str;	/* used to scan through the line */
	char	*start;	/* points to the start of the line */
	char	buf[80];

	/* if modelines are disabled, then do nothing */
	if (!*o_modelines)
	{
		return;
	}

	/* for each line... */
	for (; l <= stop; l++)
	{
		/* for each position in the line.. */
		for (str = fetchline(l); *str; str++)
		{
			/* if it is the start of a modeline command... */
			if ((str[0] == 'e' && str[1] == 'x'
			  || str[0] == 'v' && str[1] == 'i')
			  && str[2] == ':')
			{
				start = str += 3;

				/* find the end */
				for (str = start + strlen(start); *--str != ':'; )
				{
				}

				/* if it is a well-formed modeline, execute it */
				if (str > start && str - start < sizeof buf)
				{
					strncpy(buf, start, (int)(str - start));
					buf[(int)(str - start)] = '\0';
					doexcmd(buf, '\\');
					break;
				}
			}
		}
	}
}
#endif

/* The FAIL() macro prints an error message and then exits. */
#define FAIL(why,arg)	mode = MODE_EX; msg(why, arg); endwin(); exit(9)

/* This is the name of the temp file */
static char	tmpname[80];

/* This function creates the temp file and copies the original file into it.
 * Returns if successful, or stops execution if it fails.
 */
int tmpstart(char *filename)
{
	int		origfd;	/* fd used for reading the original file */
	struct stat	statb;	/* stat buffer, used to examine inode */
	REG BLK		*this;	/* pointer to the current block buffer */
	REG BLK		*next;	/* pointer to the next block buffer */
	int		inbuf;	/* number of characters in a buffer */
	int		nread;	/* number of bytes read */
	REG int		j, k;
	int		i;
	long		nbytes;

	/* switching to a different file certainly counts as a change */
	changes++;
	redraw(MARK_UNSET, FALSE);

	/* open the original file for reading */
	*origname = '\0';
	if (filename && *filename)
	{
		strcpy(origname, filename);
		origfd = open(origname, O_RDONLY);
		if (origfd < 0 && errno != ENOENT)
		{
			msg("Can't open \"%s\"", origname);
			return tmpstart("");
		}
		if (origfd >= 0)
		{
			if (stat(origname, &statb) < 0)
			{
				FAIL("Can't stat \"%s\"", origname);
			}
			if (origfd >= 0 && (statb.st_mode & S_IFMT) != S_IFREG)
			{
				msg("\"%s\" is not a regular file", origname);
				return tmpstart("");
			}
		}
		else
		{
			stat(".", &statb);
		}
		if (origfd >= 0)
		{
			origtime = statb.st_mtime;
			if (*o_readonly || !(statb.st_mode &
				  ((geteuid() == 0) ? 0222 :
				  ((statb.st_uid != geteuid() ? 0022 : 0200)))))
			{
				setflag(file, READONLY);
			}
		}
		else
		{
			origtime = 0L;
		}
	}
	else
	{
		origfd = -1;
		origtime = 0L;
		stat(".", &statb);
	}

	/* make a name for the tmp file */
	do
	{
		tmpnum++;
		sprintf(tmpname, TMPNAME, o_directory, getpid(), tmpnum);
	} while (access(tmpname, 0) == 0);

	/* !!! RACE CONDITION HERE - some other process with the same PID could
	 * create the temp file between the access() call and the creat() call.
	 * This could happen in a couple of ways:
	 * - different workstation may share the same temp dir via NFS.  Each
	 *   workstation could have a process with the same number.
	 * - The DOS version may be running multiple times on the same physical
	 *   machine in different virtual machines.  The DOS pid number will
	 *   be the same on all virtual machines.
	 *
	 * This race condition could be fixed by replacing access(tmpname, 0)
	 * with open(tmpname, O_CREAT|O_EXCL, 0600), if we could only be sure
	 * that open() *always* used modern UNIX semantics.
	 */

	/* create the temp file */
	close(creat(tmpname, 0600));		/* only we can read it */
	tmpfd = open(tmpname, O_RDWR | O_BINARY);
	if (tmpfd < 0)
	{
		FAIL("Can't create temp file... Does directory \"%s\" exist?", o_directory);
		return 1;
	}

	/* allocate space for the header in the file */
	if (write(tmpfd, hdr.c, (unsigned)BLKSIZE) < BLKSIZE
	 || write(tmpfd, tmpblk.c, (unsigned)BLKSIZE) < BLKSIZE)
	{
		FAIL("Error writing headers to \"%s\"", tmpname);
	}

#ifndef NO_RECYCLE
	/* initialize the block allocator */
	/* This must already be done here, before the first attempt
	 * to write to the new file! GB */
	garbage();
#endif

	/* initialize lnum[] */
	for (i = 1; i < MAXBLKS; i++)
	{
		lnum[i] = INFINITY;
	}
	lnum[0] = 0;

	/* if there is no original file, then create a 1-line file */
	if (origfd < 0)
	{
		*o_newfile = TRUE;

		hdr.n[0] = 0;	/* invalid inode# denotes new file */

		this = blkget(1); 	/* get the new text block */
		strcpy(this->c, "\n");	/* put a line in it */

		lnum[1] = 1L;	/* block 1 ends with line 1 */
		nlines = 1L;	/* there is 1 line in the file */
		nbytes = 1L;

		if (*origname)
		{
			msg("\"%s\" [New file]", origname);
		}
		else
		{
			if (mode==MODE_VI) msg("%s", VERSION);
		}
	}
	else /* there is an original file -- read it in */
	{
		nbytes = nlines = 0;
		*o_newfile = FALSE;

		/* preallocate 1 "next" buffer */
		i = 1;
		next = blkget(i);
		inbuf = 0;

		/* loop, moving blocks from orig to tmp */
		for (;;)
		{
			/* "next" buffer becomes "this" buffer */
			this = next;

			/* read [more] text into this block */
			nread = tread(origfd, &this->c[inbuf], BLKSIZE - 1 - inbuf);
			if (nread < 0)
			{
				close(origfd);
				close(tmpfd);
				tmpfd = -1;
				unlink(tmpname);
				FAIL("Error reading \"%s\"", origname);
			}

			/* convert NUL characters to something else */
			for (j = k = inbuf; k < inbuf + nread; k++)
			{
				if (!this->c[k])
				{
					setflag(file, HADNUL);
					this->c[j++] = 0x80;
				}
#ifndef CRUNCH
				else if (*o_beautify && this->c[k] < ' ' && this->c[k] >= 1)
				{
					if (this->c[k] == '\t'
					 || this->c[k] == '\n'
					 || this->c[k] == '\f')
					{
						this->c[j++] = this->c[k];
					}
					else if (this->c[k] == '\b')
					{
						/* delete '\b', but complain */
						setflag(file, HADBS);
					}
					/* else silently delete control char */
				}
#endif
				else
				{
					this->c[j++] = this->c[k];
				}
			}
			inbuf = j;

			/* if the buffer is empty, quit */
			if (inbuf == 0)
			{
				goto FoundEOF;
			}

			/* search backward for last newline */
			for (k = inbuf; --k >= 0 && this->c[k] != '\n';)
			{
			}
			if (k++ < 0)
			{
				if (inbuf >= BLKSIZE - 1)
				{
					k = 80;
				}
				else
				{
					k = inbuf;
				}
			}

			/* allocate next buffer */
			if (i >= MAXBLKS - 2)
			{
				FAIL("File too big.  Limit is approx %ld kbytes.", MAXBLKS * BLKSIZE / 1024L);
			}
			next = blkget(++i);

			/* move fragmentary last line to next buffer */
			inbuf -= k;
			for (j = 0; k < BLKSIZE; j++, k++)
			{
				next->c[j] = this->c[k];
				this->c[k] = 0;
			}

			/* if necessary, add a newline to this buf */
			for (k = BLKSIZE - inbuf; --k >= 0 && !this->c[k]; )
			{
			}
			if (this->c[k] != '\n')
			{
				setflag(file, ADDEDNL);
				this->c[k + 1] = '\n';
			}

			/* count the lines in this block */
			for (k = 0; k < BLKSIZE && this->c[k]; k++)
			{
				if (this->c[k] == '\n')
				{
					nlines++;
				}
				nbytes++;
			}
			lnum[i - 1] = nlines;
		}
FoundEOF:

		/* if this is a zero-length file, add 1 line */
		if (nlines == 0)
		{
			this = blkget(1); 	/* get the new text block */
			strcpy(this->c, "\n");	/* put a line in it */

			lnum[1] = 1;	/* block 1 ends with line 1 */
			nlines = 1;	/* there is 1 line in the file */
			nbytes = 1;
		}

		/* report the number of lines in the file */
		msg("\"%s\" %s%ld line%s, %ld character%s",
			origname,
			(tstflag(file, READONLY) ? "[Read-only] " : ""),
			nlines,
			nlines == 1 ? "" : "s",
			nbytes,
			nbytes == 1 ? "" : "s");
	}

	/* initialize the cursor to start of line 1 */
	cursor = MARK_FIRST;

	/* close the original file */
	close(origfd);

	/* any other messages? */
	if (tstflag(file, HADNUL))
	{
		msg("NULs have been changed to \\x80");
	}
	if (tstflag(file, ADDEDNL))
	{
		msg("Some lines were too long and have been split");
	}
#ifndef CRUNCH
	if (tstflag(file, HADBS))
	{
		msg("Backspace characters deleted due to ':set beautify'");
	}
#endif

	/* stuff the name of this file into the temp file, to support
	 * file preservation & recovery.
	 */
	storename(origname);

#ifndef NO_EXFILERC
	/* execute the .exfilerc file, if any */
	filename = gethome((char *)0);
	if (filename && *filename)
	{
		strcpy(tmpblk.c, filename);
		filename = tmpblk.c + strlen(tmpblk.c);
		if (filename[-1] != SLASH)
		{
			*filename++ = SLASH;
		}
		strcpy(filename, EXFILERC);
		doexrc(tmpblk.c);
		clrflag(file, MODIFIED);
	}
#endif

#ifndef NO_SAFER
	/* don't do anything stupid */
	i = *o_safer;
	*o_safer = TRUE;
#endif

#ifndef NO_MODELINES
	/* search for & execute any "mode lines" in the text */
	if (nlines > 10)
	{
		do_modelines(1L, 5L);
		do_modelines(nlines - 4L, nlines);
	}
	else
	{
		do_modelines(1L, nlines);
	}
#endif

#ifndef NO_SAFER
	/* okay, you can do stupid things again */
	*o_safer = i;
#endif

	/* force all blocks out onto the disk, to support file recovery */
	blksync();

	return 0;
}

/* This function copies the temp file back onto an original file.
 * Returns TRUE if successful, or FALSE if the file could NOT be saved.
 */
int tmpsave(char *filename, int bang)
{
	int		fd;	/* fd of the file we're writing to */
	REG int		len;	/* length of a text block */
	REG BLK		*this;	/* a text block */
	long		bytes;	/* byte counter */
	REG int		i;
	struct stat     statb;  /* stat buffer, used to examine inode */

	/* if no filename is given, assume the original file name */
	if (!filename || !*filename)
	{
		filename = origname;
	}

	/* if still no file name, then fail */
	if (!*filename)
	{
		msg("No current filename");
		return FALSE;
	}

	/* can't rewrite a READONLY file */
	if (!strcmp(filename, origname) && tstflag(file, READONLY) && !bang)
	{
		msg("\"%s\" File is read-only", filename);
		return FALSE;
	}

	/* open the file */
	if (*filename == '>' && filename[1] == '>')
	{
		filename += 2;
		while (*filename == ' ' || *filename == '\t')
		{
			filename++;
		}
#ifdef O_APPEND
		fd = open(filename, O_WRONLY|O_APPEND);
#else
		fd = open(filename, O_WRONLY);
		lseek(fd, 0L, 2);
#endif
	}
	else
	{
		/* either the file must not exist, or it must be the original
		 * file, or we must have a bang, or "writeany" must be set.
		 */
		if (strcmp(filename, origname) && access(filename, 0) == 0 && !bang
#ifndef CRUNCH
		    && !*o_writeany
#endif
				   )
		{
			msg("File exists - use \":w!\" to overwrite");
			return FALSE;
		}
		fd = creat(filename, FILEPERMS);
	}
	if (fd < 0)
	{
		msg("Could not write to \"%s\"", filename);
		return FALSE;
	}

	/* write each text block to the file */
	bytes = 0L;
	for (i = 1; i < MAXBLKS && (this = blkget(i)) && this->c[0]; i++)
	{
		for (len = 0; len < BLKSIZE && this->c[len]; len++)
		{
		}
		if (twrite(fd, this->c, len) < len)
		{
			msg("Trouble writing to \"%s\"", filename);
			if (!strcmp(filename, origname))
			{
				setflag(file, MODIFIED);
			}
			close(fd);
			return FALSE;
		}
		bytes += len;
	}

	/* reset the "modified" flag, but not the "undoable" flag */
	clrflag(file, MODIFIED);
	significant = FALSE;
	if (!strcmp(origname, filename))
	{
		exitcode &= ~1;
		clrflag(file, NOTEDITED);
	}
	else /* make # refer to the file we just wrote */
	{
		strcpy(prevorig, filename);
	}

	/* report lines & characters */
	msg("\"%s\" %ld lines, %ld characters", filename, nlines, bytes);

	/* close the file */
	close(fd);
	return TRUE;
}

/* This function deletes the temporary file.  If the file has been modified
 * and "bang" is FALSE, then it returns FALSE without doing anything; else
 * it returns TRUE.
 *
 * If the "autowrite" option is set, then instead of returning FALSE when
 * the file has been modified and "bang" is false, it will call tmpend().
 */
int tmpabort(int bang)
{
	/* if there is no file, return successfully */
	if (tmpfd < 0)
	{
		return TRUE;
	}

	/* see if we must return FALSE -- can't quit */
	if (!bang && tstflag(file, MODIFIED))
	{
		/* if "autowrite" is set, then act like tmpend() */
		if (*o_autowrite)
			return tmpend(bang);
		else
			return FALSE;
	}

	/* delete the tmp file */
	cutswitch();
	strcpy(prevorig, origname);
	prevline = markline(cursor);
	*origname = '\0';
	origtime = 0L;
	blkinit();
	nlines = 0;
	initflags();
	return TRUE;
}

/* This function saves the file if it has been modified, and then deletes
 * the temporary file. Returns TRUE if successful, or FALSE if the file
 * needs to be saved but can't be.  When it returns FALSE, it will not have
 * deleted the tmp file, either.
 */
int tmpend(int bang)
{
	/* save the file if it has been modified */
	if (tstflag(file, MODIFIED) && !tmpsave((char *)0, FALSE) && !bang)
	{
		return FALSE;
	}

	/* delete the tmp file */
	tmpabort(TRUE);

	return TRUE;
}

/* If the tmp file has been changed, then this function will force those
 * changes to be written to the disk, so that the tmp file will survive a
 * system crash or power failure.
 */

/* This function stores the file's name in the second block of the temp file.
 * SLEAZE ALERT!  SLEAZE ALERT!  The "tmpblk" buffer is probably being used
 * to store the arguments to a command, so we can't use it here.  Instead,
 * we'll borrow the buffer that is used for "shift-U".
 */
int
storename(char *name)
{
#ifndef CRUNCH
	int	len;
	char	*ptr;
#endif

	/* we're going to clobber the U_text buffer, so reset U_line */
	U_line = 0L;

	if (!name)
	{
		strncpy(U_text, "", BLKSIZE);
		U_text[1] = 127;
	}
#ifndef CRUNCH
	else if (*name != SLASH)
	{
		/* get the directory name */
		ptr = getcwd(U_text, BLKSIZE);
		if (ptr != U_text)
		{
			strcpy(U_text, ptr);
		}

		/* append a slash to the directory name */
		len = strlen(U_text);
		U_text[len++] = SLASH;

		/* append the filename, padded with heaps o' NULs */
		strncpy(U_text + len, *name ? name : "foo", BLKSIZE - len);
	}
#endif
	else
	{
		/* copy the filename into U_text */
		strncpy(U_text, *name ? name : "foo", BLKSIZE);
	}

	if (tmpfd >= 0)
	{
		/* write the name out to second block of the temp file */
		lseek(tmpfd, (long)BLKSIZE, 0);
		if (write(tmpfd, U_text, (unsigned)BLKSIZE) < BLKSIZE)
		{
			FAIL("Error stuffing name \"%s\" into temp file", U_text);
		}
	}
	return 0;
}

/* This function handles deadly signals.  It restores sanity to the terminal
 * preserves the current temp file, and deletes any old temp files.
 */
SIGTYPE deathtrap(int sig)
{
	char	*why;

	/* restore the terminal's sanity */
	endwin();

#ifdef CRUNCH
	why = "-ex died";
#else
	/* give a more specific description of how Elvis died */
	switch (sig)
	{
# ifdef SIGHUP
	  case SIGHUP:	why = "-hangup";                 break;
# endif
# ifndef DEBUG
#  ifdef SIGILL
	  case SIGILL:	why = "-illegal instruction";    break;
#  endif
#  ifdef SIGBUS
	  case SIGBUS:	why = "-bus error";              break;
#  endif
#  ifdef SIGSEGV
	  case SIGSEGV:	why = "-segmentation fault";     break;
#  endif
#  ifdef SIGSYS
	  case SIGSYS:	why = "-bad system call";        break;
#  endif
# endif /* !DEBUG */
# ifdef SIGPIPE
	  case SIGPIPE:	why = "-broken pipe";            break;
# endif
# ifdef SIGTERM
	  case SIGTERM:	why = "-terminated";             break;
# endif
#  ifdef SIGUSR1
	  case SIGUSR1:	why = "-user-defined signal 1";	 break;
#  endif
#  ifdef SIGUSR2
	  case SIGUSR2:	why = "-user-defined signal 2";	 break;
#  endif
	  default:	why = "-ex died";                    break;
	}
#endif

	/* if we had a temp file going, then preserve it */
	if (tmpnum > 0 && tmpfd >= 0)
	{
		close(tmpfd);
		tmpfd = -1;
		sprintf(tmpblk.c, "%s \"%s\" %s", PRESERVE, why, tmpname);
		system(tmpblk.c);
	}

	/* delete any old temp files */
	cutend();

	/* exit with the proper exit status */
	exit(sig);
}
