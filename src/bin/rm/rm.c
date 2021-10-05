/*-
 * Copyright (c) 1990, 1993
 *	The Regents of the University of California.  All rights reserved.
 * 
 * Modifications copyright 2021 S. V. Nickolas.
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
"@(#) Copyright (c) 1990, 1993\n\
	The Regents of the University of California.  All rights reserved.\n"
"Modifications copyright 2021 S. V. Nickolas\n";
#endif /* not lint */

#ifndef lint
static char sccsid[] = "@(#)rm.c	8.1 (Berkeley) 6/2/93";
#endif /* not lint */

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/errno.h>

#include <err.h>
#include <fts.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int vflag, dflag, fflag, iflag, eval, stdin_ok;

int	check (char *, char *, struct stat *);
void	checkdot (char **);
void	rmfile (char **);
void	rmtree (char **);
void	usage (void);

const char *user_from_uid(uid_t uid, int noname);
const char *group_from_gid(gid_t gid, int noname);
void strmode(int mode, char *p);

/*
 * rm --
 *	This rm is different from historic rm's, but is expected to match
 *	POSIX 1003.2 behavior.  The most visible difference is that -f
 *	has two specific effects now, ignore non-existent files and force
 * 	file removal.
 */
int
main(int argc, char* argv[])
{
	extern int optind;
	int ch, rflag;

	rflag = 0;
	while ((ch = getopt(argc, argv, "dfiRrv")) != EOF)
		switch(ch) {
		case 'd':
			dflag = 1;
			break;
		case 'f':
			fflag = 1;
			iflag = 0;
			break;
		case 'i':
			fflag = 0;
			iflag = 1;
			break;
		case 'R':
		case 'r':			/* compatibility */
			rflag = 1;
			break;
        case 'v':           /* uso: verbose */
            vflag = 1;
            break;
		case '?':
		default:
			usage();
		}
	argc -= optind;
	argv += optind;

	if (argc < 1)
		usage();

	checkdot(argv);
	if (!*argv)
		exit (eval);

	stdin_ok = isatty(STDIN_FILENO);

	if (rflag)
		rmtree(argv);
	else
		rmfile(argv);
	exit (eval);
}

void
rmtree(char* *argv)
{
	register FTS *fts;
	register FTSENT *p;
	register int needstat;

	/*
	 * Remove a file hierarchy.  If forcing removal (-f), or interactive
	 * (-i) or can't ask anyway (stdin_ok), don't stat the file.
	 */
	needstat = !fflag && !iflag && stdin_ok;

	/*
	 * If the -i option is specified, the user can skip on the pre-order
	 * visit.  The fts_number field flags skipped directories.
	 */
#define	SKIPPED	1

	if (!(fts = fts_open(argv,
	    needstat ? FTS_PHYSICAL : FTS_PHYSICAL|FTS_NOSTAT,
	    (int (*)())NULL)))
		err(1, NULL);
	while (p = fts_read(fts)) {
		switch (p->fts_info) {
		case FTS_DNR:
			if (!fflag || errno != ENOENT) {
				warn("%s", p->fts_path);
				eval = 1;
			}
			continue;
		case FTS_ERR:
			err(1, "%s", p->fts_path);
		case FTS_NS:
			/*
			 * FTS_NS: assume that if can't stat the file, it
			 * can't be unlinked.
			 */
			if (!needstat)
				break;
			if (!fflag || errno != ENOENT) {
				warn("%s", p->fts_path);
				eval = 1;
			}
			continue;
		case FTS_D:
			/* Pre-order: give user chance to skip. */
			if (iflag && !check(p->fts_path, p->fts_accpath,
			    p->fts_statp)) {
				(void)fts_set(fts, p, FTS_SKIP);
				p->fts_number = SKIPPED;
			}
			continue;
		case FTS_DP:
			/* Post-order: see if user skipped. */
			if (p->fts_number == SKIPPED)
				continue;
			break;
		}
		if (!fflag &&
		    !check(p->fts_path, p->fts_accpath, p->fts_statp))
			continue;

		/*
		 * If we can't read or search the directory, may still be
		 * able to remove it.  Don't print out the un{read,search}able
		 * message unless the remove fails.
		 */
		if (p->fts_info == FTS_DP || p->fts_info == FTS_DNR) {
			if (!rmdir(p->fts_accpath))
				continue;
			if (errno == ENOENT) {
				if (fflag)
					continue;
			} else if (p->fts_info != FTS_DP)
				warnx("%s: unable to read", p->fts_path);
		} else if (!unlink(p->fts_accpath) || fflag && errno == ENOENT)
			continue;
		warn("%s", p->fts_path);
		eval = 1;
	}
}

void
rmfile(char **argv)
{
	register int df;
	register char *f;
	struct stat sb;

	df = dflag;
	/*
	 * Remove a file.  POSIX 1003.2 states that, by default, attempting
	 * to remove a directory is an error, so must always stat the file.
	 */
	while (f = *argv++) {
		/* Assume if can't stat the file, can't unlink it. */
		if (lstat(f, &sb)) {
			if (!fflag || errno != ENOENT) {
				warn("%s", f);
				eval = 1;
			}
			continue;
		}
		if (S_ISDIR(sb.st_mode) && !df) {
			warnx("%s: is a directory", f);
			eval = 1;
			continue;
		}
		if (!fflag && !check(f, f, &sb))
			continue;
        if (vflag)
            puts(f); /* uso */
		if ((S_ISDIR(sb.st_mode) ? rmdir(f) : unlink(f)) &&
		    (!fflag || errno != ENOENT)) {
			warn("%s", f);
			eval = 1;
		}
	}
}

int
check(char *path, char *name, struct stat *sp)
{
	register int first, ch;
	char modep[15];

	/* Check -i first. */
	if (iflag)
		(void)fprintf(stderr, "remove %s? ", path);
	else {
		/*
		 * If it's not a symbolic link and it's unwritable and we're
		 * talking to a terminal, ask.  Symbolic links are excluded
		 * because their permissions are meaningless.
		 */
		if (S_ISLNK(sp->st_mode) || !stdin_ok || !access(name, W_OK))
			return (1);
		strmode(sp->st_mode, modep);
		(void)fprintf(stderr, "override %s%s%s/%s for %s? ",
		    modep + 1, modep[9] == ' ' ? "" : " ",
		    user_from_uid(sp->st_uid, 0),
		    group_from_gid(sp->st_gid, 0), path);
	}
	(void)fflush(stderr);

	first = ch = getchar();
	while (ch != '\n' && ch != EOF)
		ch = getchar();
	return (first == 'y');
}

#define ISDOT(a)	((a)[0] == '.' && (!(a)[1] || (a)[1] == '.' && !(a)[2]))
void
checkdot(char **argv)
{
	register char *p, **t, **save;
	int complained;

	complained = 0;
	for (t = argv; *t;) {
		if (p = strrchr(*t, '/'))
			++p;
		else
			p = *t;
		if (ISDOT(p)) {
			if (!complained++)
				warnx("\".\" and \"..\" may not be removed");
			eval = 1;
			for (save = t; t[0] = t[1]; ++t);
			t = save;
		} else
			++t;
	}
}

void
usage()
{
	(void)fprintf(stderr, "usage: rm [-dfiRrv] file ...\n");
	exit(1);
}
