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

#ifndef _CONFIG_H
#define _CONFIG_H

/**************************** Compiler quirks *********************************/

/* as far as I know, all compilers except version 7 support unsigned char */
/* NEWFLASH: the Minix-ST compiler has subtle problems with unsigned char */
#define UCHAR(c)	((unsigned char)(c))
#define uchar		unsigned char

/* UNIX declares lseek in <unistd.h>.  Everybody else (except Amiga) wants
 * lseek declared here.  Note: "off_t" may be preferable to "long".
 */
#include <sys/types.h>
#include <unistd.h>

/* Signal handler functions used to return an int value, which was ignored.
 * On newer systems, signal handlers are void functions.  Here, we try to
 * guess the proper return type for this system.
 */
#ifndef SIGTYPE
# define SIGTYPE void
#endif

/* There are two terminal-related functions that we need: ttyread() and
 * ttywrite().  The ttyread() function implements read-with-timeout and is
 * a true function on all systems.  The ttywrite() function is almost always
 * just a macro...
 */
#define ttywrite(buf, len)	write(1, buf, (unsigned)(len))	/* raw write */

/* BSD uses getwd() instead of getcwd().  The arguments are a little different,
 * but we'll ignore that and hope for the best; adding arguments to the macro
 * would mess up an "extern" declaration of the function.
 *
 * Also, the Coherent-286 uses getwd(), but Coherent-386 uses getcwd()
 */
#ifndef getcwd
extern char *getcwd();
#endif

/* text versus binary mode for read/write */
#define	tread(fd,buf,n)		read(fd,buf,(unsigned)(n))
#define twrite(fd,buf,n)	write(fd,buf,(unsigned)(n))

/******************* Names of files and environment vars **********************/

#ifndef TMPDIR
# define TMPDIR	"/tmp"		/* directory where temp files live */
#endif

#ifndef PRSVDIR
# define PRSVDIR	"/usr/preserve"	/* directory where preserved file live */
#endif

#ifndef PRSVINDEX
# define PRSVINDEX	"/usr/preserve/Index" /* index of files in PRSVDIR */
#endif

#ifndef EXRC
# define EXRC		".exrc"		/* init file in current directory */
#endif

#ifndef EXFILERC
# define EXFILERC	".exfilerc"	/* init file for each file */
#endif

#define SCRATCHOUT	"%s/soXXXXXX"	/* temp file used as input to filter */

#ifndef SHELL
# define SHELL		"/bin/sh"	/* default shell */
#endif

#define gethome(x)	getenv("HOME")

#ifndef	TAGS
# define TAGS		"tags"		/* name of the tags file */
#endif

#ifndef TMPNAME
# define TMPNAME	"%s/ex_%x.%x"	/* format of names for temp files */
#endif

#ifndef EXINIT
# define EXINIT		"EXINIT"	/* name of EXINIT environment variable */
#endif

#ifndef	EXRC
# define EXRC		".exrc"	/* name of ".exrc" file in current dir */
#endif

#ifndef HMEXRC
# define HMEXRC		EXRC		/* name of ".exrc" file in home dir */
#endif

#ifndef EXFILERC
# define EXFILERC	".exfilerc"	/* name of ".exfilerc" file in home dir */
#endif

#ifndef	KEYWORDPRG
# define KEYWORDPRG	"ref"
#endif

#ifndef	SCRATCHOUT
# define SCRATCHIN	"%s/SIXXXXXX"
# define SCRATCHOUT	"%s/SOXXXXXX"
#endif

#ifndef ERRLIST
# define ERRLIST	"errlist"
#endif

/* alternate char to be used in place of spaces in filenames while spaces
 * are being used as delimiters (can be a space if they are not allowed
 * in filenames; else any very unlikely non-white char [not tab, CR, LF]) */
#ifndef	SPACEHOLDER
# define SPACEHOLDER	0xff
#endif

#ifndef	SLASH
# define SLASH		'/'
#endif

#ifndef SHELL
# define SHELL		"shell"
#endif

#ifndef REG
# define REG		register
#endif

#ifndef NEEDSYNC
# define NEEDSYNC	FALSE
#endif

#ifndef FILEPERMS
# define FILEPERMS	0666
#endif

#ifndef PRESERVE
# define PRESERVE	"expreserve"	/* name of the "preserve" program */
#endif

#ifndef CC_COMMAND
# define CC_COMMAND	"cc -c"
#endif

#ifndef MAKE_COMMAND
# define MAKE_COMMAND	"make"
#endif

#ifndef REDIRECT
# define REDIRECT	"2>"
#endif

#ifndef BLKSIZE
# ifdef CRUNCH
#  define BLKSIZE	1024
# else
#  define BLKSIZE	2048
# endif
#endif

#ifndef KEYBUFSIZE
# define KEYBUFSIZE	1000
#endif

#ifndef MAILER
# define MAILER		"mail"
#endif

#ifndef WSRCH_MAX
# define WSRCH_MAX	50
#endif

#ifndef gethome
extern char *gethome (char *);
#endif

#endif  /* ndef _CONFIG_H */
