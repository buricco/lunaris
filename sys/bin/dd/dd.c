/*-
 * Copyright (c) 1991, 1993, 1994
 *	The Regents of the University of California.  All rights reserved.
 *
 * timespecsub macro:
 * Copyright (c) 2006-2018 Roy Marples <roy@marples.name>
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Keith Muller of the University of California, San Diego and Lance
 * Visser of Convex Computer Corporation.
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
 *
 *	@(#)dd.h	8.3 (Berkeley) 4/2/94
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#ifdef __OS2__
#include "err.h"
#define SSIZE_MAX 32767 /* POSIX? */
#else
#include <err.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/mtio.h>
#endif

#ifndef O_BINARY
#define O_BINARY 0
#endif

#ifndef DEFFILEMODE
#define DEFFILEMODE 666 /* musl */
#endif

#define MINIMUM(a, b)	(((a) < (b)) ? (a) : (b))
#define MAXIMUM(a, b)	(((a) > (b)) ? (a) : (b))

#define timespecsub(tsp, usp, vsp)                                      \
        do {                                                            \
                (vsp)->tv_sec = (tsp)->tv_sec - (usp)->tv_sec;          \
                (vsp)->tv_nsec = (tsp)->tv_nsec - (usp)->tv_nsec;       \
                if ((vsp)->tv_nsec < 0) {                               \
                        (vsp)->tv_sec--;                                \
                        (vsp)->tv_nsec += 1000000000L;                  \
                }                                                       \
        } while (/* CONSTCOND */ 0)

/* Flags (in ddflags). */
#define	C_ASCII		0x00001
#define	C_BLOCK		0x00002
#define	C_BS		0x00004
#define	C_CBS		0x00008
#define	C_COUNT		0x00010
#define	C_EBCDIC	0x00020
#define	C_FILES		0x00040
#define	C_IBS		0x00080
#define	C_IF		0x00100
#define	C_LCASE		0x00200
#define	C_NOERROR	0x00400
#define	C_NOTRUNC	0x00800
#define	C_OBS		0x01000
#define	C_OF		0x02000
#define	C_SEEK		0x04000
#define	C_SKIP		0x08000
#define	C_SWAB		0x10000
#define	C_SYNC		0x20000
#define	C_UCASE		0x40000
#define	C_UNBLOCK	0x80000
#define	C_OSYNC		0x100000
#define	C_STATUS	0x200000
#define	C_NOXFER	0x400000
#define	C_NOINFO	0x800000
#define	C_FSYNC		0x1000000

static int	c_arg(const void *, const void *);
static void	f_bs(char *);
static void	f_cbs(char *);
static void	f_conv(char *);
static void	f_count(char *);
static void	f_files(char *);
static void	f_ibs(char *);
static void	f_if(char *);
static void	f_obs(char *);
static void	f_of(char *);
static void	f_seek(char *);
static void	f_skip(char *);
static void	f_status(char *);
static size_t	get_bsz(char *);
static off_t	get_off(char *);

/* Input/output stream state. */
typedef struct {
	unsigned char	*db;			/* buffer address */
	unsigned char	*dbp;			/* current buffer I/O address */
	size_t	dbcnt;			/* current buffer byte count */
	size_t	dbrcnt;			/* last read byte count */
	size_t	dbsz;			/* buffer size */

#define	ISCHR		0x01		/* character device (warn on short) */
#define	ISPIPE		0x02		/* pipe (not truncatable) */
#define	ISTAPE		0x04		/* tape (not seekable) */
#define	NOREAD		0x08		/* not readable */
	unsigned	flags;

	char	*name;			/* name */
	int	fd;			/* file descriptor */
	off_t	offset;			/* # of blocks to skip */

	size_t	f_stats;		/* # of full blocks processed */
	size_t	p_stats;		/* # of partial blocks processed */
	size_t	s_stats;		/* # of odd swab blocks */
	size_t	t_stats;		/* # of truncations */
} IO;

typedef struct {
	size_t	in_full;		/* # of full input blocks */
	size_t	in_part;		/* # of partial input blocks */
	size_t	out_full;		/* # of full output blocks */
	size_t	out_part;		/* # of partial output blocks */
	size_t	trunc;			/* # of truncated records */
	size_t	swab;			/* # of odd-length swab blocks */
	off_t	bytes;			/* # of bytes written */
#ifdef __OS2__
	time_t	start;
#else
	struct	timespec start;		/* start time of dd */
#endif
} STAT;

static const struct arg {
	const char *name;
	void (*f)(char *);
	long set, noset;
} args[] = {
	{ "bs",		f_bs,		C_BS,	 C_BS|C_IBS|C_OBS|C_OSYNC },
	{ "cbs",	f_cbs,		C_CBS,	 C_CBS },
	{ "conv",	f_conv,		0,	 0 },
	{ "count",	f_count,	C_COUNT, C_COUNT },
	{ "files",	f_files,	C_FILES, C_FILES },
	{ "ibs",	f_ibs,		C_IBS,	 C_BS|C_IBS },
	{ "if",		f_if,		C_IF,	 C_IF },
	{ "obs",	f_obs,		C_OBS,	 C_BS|C_OBS },
	{ "of",		f_of,		C_OF,	 C_OF },
	{ "seek",	f_seek,		C_SEEK,	 C_SEEK },
	{ "skip",	f_skip,		C_SKIP,	 C_SKIP },
	{ "status",	f_status,	C_STATUS,C_STATUS },
};

static void dd_close(void);
static void dd_in(void);
static void getfdtype(IO *);
static void setup(void);

void block(void);
void block_close(void);
void dd_out(int);
void def(void);
void def_close(void);
void jcl(char **);
void pos_in(void);
void pos_out(void);
void summary(void);
void summaryx(int);
void terminate(int);
void unblock(void);
void unblock_close(void);

IO	in, out;		/* input/output state */
STAT	st;			/* statistics */
void	(*cfunc)(void);		/* conversion function */
size_t	cpy_cnt;		/* # of blocks to copy */
unsigned	ddflags;		/* conversion options */
size_t	cbsz;			/* conversion block size */
size_t	files_cnt = 1;		/* # of files to copy */
const	unsigned char	*ctab;		/* conversion table */

static char *oper;

/*
 * There are currently three tables:
 *
 *	ebcdic		-> ascii	POSIX/S5	conv=ascii
 *	ascii		-> ebcdic	POSIX/S5	conv=ebcdic
 *	ascii		-> ibm ebcdic	POSIX/S5	conv=ibm
 *
 * Other tables are built from these if multiple conversions are being
 * done.
 *
 * Tables used for conversions to/from IBM and EBCDIC to support an extension
 * to POSIX P1003.2/D11. The tables referencing POSIX contain data extracted
 * from tables 4-3 and 4-4 in P1003.2/Draft 11.
 *
 * More information can be obtained in "Correspondences of 8-Bit and Hollerith
 * Codes for Computer Environments-A USASI Tutorial", Communications of the
 * ACM, Volume 11, Number 11, November 1968, pp. 783-789.
 */

unsigned char casetab[256];

/* EBCDIC to ASCII -- POSIX and System V compatible. */
const unsigned char e2a_POSIX[] = {
	0000, 0001, 0002, 0003, 0234, 0011, 0206, 0177,		/* 0000 */
	0227, 0215, 0216, 0013, 0014, 0015, 0016, 0017,		/* 0010 */
	0020, 0021, 0022, 0023, 0235, 0205, 0010, 0207,		/* 0020 */
	0030, 0031, 0222, 0217, 0034, 0035, 0036, 0037,		/* 0030 */
	0200, 0201, 0202, 0203, 0204, 0012, 0027, 0033,		/* 0040 */
	0210, 0211, 0212, 0213, 0214, 0005, 0006, 0007,		/* 0050 */
	0220, 0221, 0026, 0223, 0224, 0225, 0226, 0004,		/* 0060 */
	0230, 0231, 0232, 0233, 0024, 0025, 0236, 0032,		/* 0070 */
	0040, 0240, 0241, 0242, 0243, 0244, 0245, 0246,		/* 0100 */
	0247, 0250, 0325, 0056, 0074, 0050, 0053, 0174,		/* 0110 */
	0046, 0251, 0252, 0253, 0254, 0255, 0256, 0257,		/* 0120 */
	0260, 0261, 0041, 0044, 0052, 0051, 0073, 0176,		/* 0130 */
	0055, 0057, 0262, 0263, 0264, 0265, 0266, 0267,		/* 0140 */
	0270, 0271, 0313, 0054, 0045, 0137, 0076, 0077,		/* 0150 */
	0272, 0273, 0274, 0275, 0276, 0277, 0300, 0301,		/* 0160 */
	0302, 0140, 0072, 0043, 0100, 0047, 0075, 0042,		/* 0170 */
	0303, 0141, 0142, 0143, 0144, 0145, 0146, 0147,		/* 0200 */
	0150, 0151, 0304, 0305, 0306, 0307, 0310, 0311,		/* 0210 */
	0312, 0152, 0153, 0154, 0155, 0156, 0157, 0160,		/* 0220 */
	0161, 0162, 0136, 0314, 0315, 0316, 0317, 0320,		/* 0230 */
	0321, 0345, 0163, 0164, 0165, 0166, 0167, 0170,		/* 0240 */
	0171, 0172, 0322, 0323, 0324, 0133, 0326, 0327,		/* 0250 */
	0330, 0331, 0332, 0333, 0334, 0335, 0336, 0337,		/* 0260 */
	0340, 0341, 0342, 0343, 0344, 0135, 0346, 0347,		/* 0270 */
	0173, 0101, 0102, 0103, 0104, 0105, 0106, 0107,		/* 0300 */
	0110, 0111, 0350, 0351, 0352, 0353, 0354, 0355,		/* 0310 */
	0175, 0112, 0113, 0114, 0115, 0116, 0117, 0120,		/* 0320 */
	0121, 0122, 0356, 0357, 0360, 0361, 0362, 0363,		/* 0330 */
	0134, 0237, 0123, 0124, 0125, 0126, 0127, 0130,		/* 0340 */
	0131, 0132, 0364, 0365, 0366, 0367, 0370, 0371,		/* 0350 */
	0060, 0061, 0062, 0063, 0064, 0065, 0066, 0067,		/* 0360 */
	0070, 0071, 0372, 0373, 0374, 0375, 0376, 0377,		/* 0370 */
};

/* ASCII to EBCDIC -- POSIX and System V compatible. */
const unsigned char a2e_POSIX[] = {
	0000, 0001, 0002, 0003, 0067, 0055, 0056, 0057,		/* 0000 */
	0026, 0005, 0045, 0013, 0014, 0015, 0016, 0017,		/* 0010 */
	0020, 0021, 0022, 0023, 0074, 0075, 0062, 0046,		/* 0020 */
	0030, 0031, 0077, 0047, 0034, 0035, 0036, 0037,		/* 0030 */
	0100, 0132, 0177, 0173, 0133, 0154, 0120, 0175,		/* 0040 */
	0115, 0135, 0134, 0116, 0153, 0140, 0113, 0141,		/* 0050 */
	0360, 0361, 0362, 0363, 0364, 0365, 0366, 0367,		/* 0060 */
	0370, 0371, 0172, 0136, 0114, 0176, 0156, 0157,		/* 0070 */
	0174, 0301, 0302, 0303, 0304, 0305, 0306, 0307,		/* 0100 */
	0310, 0311, 0321, 0322, 0323, 0324, 0325, 0326,		/* 0110 */
	0327, 0330, 0331, 0342, 0343, 0344, 0345, 0346,		/* 0120 */
	0347, 0350, 0351, 0255, 0340, 0275, 0232, 0155,		/* 0130 */
	0171, 0201, 0202, 0203, 0204, 0205, 0206, 0207,		/* 0140 */
	0210, 0211, 0221, 0222, 0223, 0224, 0225, 0226,		/* 0150 */
	0227, 0230, 0231, 0242, 0243, 0244, 0245, 0246,		/* 0160 */
	0247, 0250, 0251, 0300, 0117, 0320, 0137, 0007,		/* 0170 */
	0040, 0041, 0042, 0043, 0044, 0025, 0006, 0027,		/* 0200 */
	0050, 0051, 0052, 0053, 0054, 0011, 0012, 0033,		/* 0210 */
	0060, 0061, 0032, 0063, 0064, 0065, 0066, 0010,		/* 0220 */
	0070, 0071, 0072, 0073, 0004, 0024, 0076, 0341,		/* 0230 */
	0101, 0102, 0103, 0104, 0105, 0106, 0107, 0110,		/* 0240 */
	0111, 0121, 0122, 0123, 0124, 0125, 0126, 0127,		/* 0250 */
	0130, 0131, 0142, 0143, 0144, 0145, 0146, 0147,		/* 0260 */
	0150, 0151, 0160, 0161, 0162, 0163, 0164, 0165,		/* 0270 */
	0166, 0167, 0170, 0200, 0212, 0213, 0214, 0215,		/* 0300 */
	0216, 0217, 0220, 0152, 0233, 0234, 0235, 0236,		/* 0310 */
	0237, 0240, 0252, 0253, 0254, 0112, 0256, 0257,		/* 0320 */
	0260, 0261, 0262, 0263, 0264, 0265, 0266, 0267,		/* 0330 */
	0270, 0271, 0272, 0273, 0274, 0241, 0276, 0277,		/* 0340 */
	0312, 0313, 0314, 0315, 0316, 0317, 0332, 0333,		/* 0350 */
	0334, 0335, 0336, 0337, 0352, 0353, 0354, 0355,		/* 0360 */
	0356, 0357, 0372, 0373, 0374, 0375, 0376, 0377,		/* 0370 */
};

/* ASCII to IBM EBCDIC -- POSIX and System V compatible. */
const unsigned char a2ibm_POSIX[] = {
	0000, 0001, 0002, 0003, 0067, 0055, 0056, 0057,		/* 0000 */
	0026, 0005, 0045, 0013, 0014, 0015, 0016, 0017,		/* 0010 */
	0020, 0021, 0022, 0023, 0074, 0075, 0062, 0046,		/* 0020 */
	0030, 0031, 0077, 0047, 0034, 0035, 0036, 0037,		/* 0030 */
	0100, 0132, 0177, 0173, 0133, 0154, 0120, 0175,		/* 0040 */
	0115, 0135, 0134, 0116, 0153, 0140, 0113, 0141,		/* 0050 */
	0360, 0361, 0362, 0363, 0364, 0365, 0366, 0367,		/* 0060 */
	0370, 0371, 0172, 0136, 0114, 0176, 0156, 0157,		/* 0070 */
	0174, 0301, 0302, 0303, 0304, 0305, 0306, 0307,		/* 0100 */
	0310, 0311, 0321, 0322, 0323, 0324, 0325, 0326,		/* 0110 */
	0327, 0330, 0331, 0342, 0343, 0344, 0345, 0346,		/* 0120 */
	0347, 0350, 0351, 0255, 0340, 0275, 0137, 0155,		/* 0130 */
	0171, 0201, 0202, 0203, 0204, 0205, 0206, 0207,		/* 0140 */
	0210, 0211, 0221, 0222, 0223, 0224, 0225, 0226,		/* 0150 */
	0227, 0230, 0231, 0242, 0243, 0244, 0245, 0246,		/* 0160 */
	0247, 0250, 0251, 0300, 0117, 0320, 0241, 0007,		/* 0170 */
	0040, 0041, 0042, 0043, 0044, 0025, 0006, 0027,		/* 0200 */
	0050, 0051, 0052, 0053, 0054, 0011, 0012, 0033,		/* 0210 */
	0060, 0061, 0032, 0063, 0064, 0065, 0066, 0010,		/* 0220 */
	0070, 0071, 0072, 0073, 0004, 0024, 0076, 0341,		/* 0230 */
	0101, 0102, 0103, 0104, 0105, 0106, 0107, 0110,		/* 0240 */
	0111, 0121, 0122, 0123, 0124, 0125, 0126, 0127,		/* 0250 */
	0130, 0131, 0142, 0143, 0144, 0145, 0146, 0147,		/* 0260 */
	0150, 0151, 0160, 0161, 0162, 0163, 0164, 0165,		/* 0270 */
	0166, 0167, 0170, 0200, 0212, 0213, 0214, 0215,		/* 0300 */
	0216, 0217, 0220, 0232, 0233, 0234, 0235, 0236,		/* 0310 */
	0237, 0240, 0252, 0253, 0254, 0255, 0256, 0257,		/* 0320 */
	0260, 0261, 0262, 0263, 0264, 0265, 0266, 0267,		/* 0330 */
	0270, 0271, 0272, 0273, 0274, 0275, 0276, 0277,		/* 0340 */
	0312, 0313, 0314, 0315, 0316, 0317, 0332, 0333,		/* 0350 */
	0334, 0335, 0336, 0337, 0352, 0353, 0354, 0355,		/* 0360 */
	0356, 0357, 0372, 0373, 0374, 0375, 0376, 0377,		/* 0370 */
};

/*
 * def --
 * Copy input to output.  Input is buffered until reaches obs, and then
 * output until less than obs remains.  Only a single buffer is used.
 * Worst case buffer calculation is (ibs + obs - 1).
 */
void
def(void)
{
	size_t cnt;
	unsigned char *inp;
	const unsigned char *t;

	if ((t = ctab) != NULL)
		for (inp = in.dbp - (cnt = in.dbrcnt); cnt--; ++inp)
			*inp = t[*inp];

	/* Make the output buffer look right. */
	out.dbp = in.dbp;
	out.dbcnt = in.dbcnt;

	if (in.dbcnt >= out.dbsz) {
		/* If the output buffer is full, write it. */
		dd_out(0);

		/*
		 * Ddout copies the leftover output to the beginning of
		 * the buffer and resets the output buffer.  Reset the
		 * input buffer to match it.
		 */
		in.dbp = out.dbp;
		in.dbcnt = out.dbcnt;
	}
}

void
def_close(void)
{
	/* Just update the count, everything is already in the buffer. */
	if (in.dbcnt)
		out.dbcnt = in.dbcnt;
}

#ifdef	NO_CONV
/* Build a smaller version (i.e. for a miniroot) */
/* These can not be called, but just in case...  */
static char no_block[] = "unblock and -DNO_CONV?";
void block()       { errx(1, "%s", no_block + 2); }
void block_close() { errx(1, "%s", no_block + 2); }
void unblock()       { errx(1, "%s", no_block); }
void unblock_close() { errx(1, "%s", no_block); }
#else	/* NO_CONV */

/*
 * Copy variable length newline terminated records with a max size cbsz
 * bytes to output.  Records less than cbs are padded with spaces.
 *
 * max in buffer:  MAX(ibs, cbsz)
 * max out buffer: obs + cbsz
 */
void
block(void)
{
	static int intrunc;
	int ch = -1;
	size_t cnt, maxlen;
	unsigned char *inp, *outp;
	const unsigned char *t;

	/*
	 * Record truncation can cross block boundaries.  If currently in a
	 * truncation state, keep tossing characters until reach a newline.
	 * Start at the beginning of the buffer, as the input buffer is always
	 * left empty.
	 */
	if (intrunc) {
		for (inp = in.db, cnt = in.dbrcnt;
		    cnt && *inp++ != '\n'; --cnt);
		if (!cnt) {
			in.dbcnt = 0;
			in.dbp = in.db;
			return;
		}
		intrunc = 0;
		/* Adjust the input buffer numbers. */
		in.dbcnt = cnt - 1;
		in.dbp = inp + cnt - 1;
	}

	/*
	 * Copy records (max cbsz size chunks) into the output buffer.  The
	 * translation is done as we copy into the output buffer.
	 */
	for (inp = in.dbp - in.dbcnt, outp = out.dbp; in.dbcnt;) {
		maxlen = MINIMUM(cbsz, in.dbcnt);
		if ((t = ctab) != NULL)
			for (cnt = 0;
			    cnt < maxlen && (ch = *inp++) != '\n'; ++cnt)
				*outp++ = t[ch];
		else
			for (cnt = 0;
			    cnt < maxlen && (ch = *inp++) != '\n'; ++cnt)
				*outp++ = ch;
		/*
		 * Check for short record without a newline.  Reassemble the
		 * input block.
		 */
		if (ch != '\n' && in.dbcnt < cbsz) {
			(void)memmove(in.db, in.dbp - in.dbcnt, in.dbcnt);
			break;
		}

		/* Adjust the input buffer numbers. */
		in.dbcnt -= cnt;
		if (ch == '\n')
			--in.dbcnt;

		/* Pad short records with spaces. */
		if (cnt < cbsz)
			(void)memset(outp, ctab ? ctab[' '] : ' ', cbsz - cnt);
		else {
			/*
			 * If the next character wouldn't have ended the
			 * block, it's a truncation.
			 */
			if (!in.dbcnt || *inp != '\n')
				++st.trunc;

			/* Toss characters to a newline. */
			for (; in.dbcnt && *inp++ != '\n'; --in.dbcnt);
			if (!in.dbcnt)
				intrunc = 1;
			else
				--in.dbcnt;
		}

		/* Adjust output buffer numbers. */
		out.dbp += cbsz;
		if ((out.dbcnt += cbsz) >= out.dbsz)
			dd_out(0);
		outp = out.dbp;
	}
	in.dbp = in.db + in.dbcnt;
}

void
block_close(void)
{
	/*
	 * Copy any remaining data into the output buffer and pad to a record.
	 * Don't worry about truncation or translation, the input buffer is
	 * always empty when truncating, and no characters have been added for
	 * translation.  The bottom line is that anything left in the input
	 * buffer is a truncated record.  Anything left in the output buffer
	 * just wasn't big enough.
	 */
	if (in.dbcnt) {
		++st.trunc;
		(void)memmove(out.dbp, in.dbp - in.dbcnt, in.dbcnt);
		(void)memset(out.dbp + in.dbcnt,
		    ctab ? ctab[' '] : ' ', cbsz - in.dbcnt);
		out.dbcnt += cbsz;
	}
}

/*
 * Convert fixed length (cbsz) records to variable length.  Deletes any
 * trailing blanks and appends a newline.
 *
 * max in buffer:  MAX(ibs, cbsz) + cbsz
 * max out buffer: obs + cbsz
 */
void
unblock(void)
{
	size_t cnt;
	unsigned char *inp;
	const unsigned char *t;

	/* Translation and case conversion. */
	if ((t = ctab) != NULL)
		for (cnt = in.dbrcnt, inp = in.dbp - 1; cnt--; inp--)
			*inp = t[*inp];
	/*
	 * Copy records (max cbsz size chunks) into the output buffer.  The
	 * translation has to already be done or we might not recognize the
	 * spaces.
	 */
	for (inp = in.db; in.dbcnt >= cbsz; inp += cbsz, in.dbcnt -= cbsz) {
		for (t = inp + cbsz - 1; t >= inp && *t == ' '; --t);
		if (t >= inp) {
			cnt = t - inp + 1;
			(void)memmove(out.dbp, inp, cnt);
			out.dbp += cnt;
			out.dbcnt += cnt;
		}
		++out.dbcnt;
		*out.dbp++ = '\n';
		if (out.dbcnt >= out.dbsz)
			dd_out(0);
	}
	if (in.dbcnt)
		(void)memmove(in.db, in.dbp - in.dbcnt, in.dbcnt);
	in.dbp = in.db + in.dbcnt;
}

void
unblock_close(void)
{
	size_t cnt;
	unsigned char *t;

	if (in.dbcnt) {
		warnx("%s: short input record", in.name);
		for (t = in.db + in.dbcnt - 1; t >= in.db && *t == ' '; --t);
		if (t >= in.db) {
			cnt = t - in.db + 1;
			(void)memmove(out.dbp, in.db, cnt);
			out.dbp += cnt;
			out.dbcnt += cnt;
		}
		++out.dbcnt;
		*out.dbp++ = '\n';
	}
}

#endif	/* NO_CONV */

/*
 * Position input/output data streams before starting the copy.  Device type
 * dependent.  Seekable devices use lseek, and the rest position by reading.
 * Seeking past the end of file can cause null blocks to be written to the
 * output.
 */
void
pos_in(void)
{
	size_t bcnt;
	ssize_t nr;
	off_t cnt;
	int warned;

	/* If not a pipe, tape or tty device, try to seek on it. */
	if (!(in.flags & (ISPIPE|ISTAPE)) && !isatty(in.fd)) {
		if (lseek(in.fd, in.offset * in.dbsz, SEEK_CUR) == -1)
			err(1, "%s", in.name);
		return;
	}

	/*
	 * Read the data.  If a pipe, read until satisfy the number of bytes
	 * being skipped.  No differentiation for reading complete and partial
	 * blocks for other devices.
	 */
	for (bcnt = in.dbsz, cnt = in.offset, warned = 0; cnt;) {
		if ((nr = read(in.fd, in.db, bcnt)) > 0) {
			if (in.flags & ISPIPE) {
				if (!(bcnt -= nr)) {
					bcnt = in.dbsz;
					--cnt;
				}
			} else
				--cnt;
			continue;
		}

		if (nr == 0) {
			if (files_cnt > 1) {
				--files_cnt;
				continue;
			}
			errx(1, "skip reached end of input");
		}

		/*
		 * Input error -- either EOF with no more files, or I/O error.
		 * If noerror not set die.  POSIX requires that the warning
		 * message be followed by an I/O display.
		 */
		if (ddflags & C_NOERROR) {
			if (!warned) {
				warn("%s", in.name);
				warned = 1;
				summary();
			}
			continue;
		}
		err(1, "%s", in.name);
	}
}

void
pos_out(void)
{
#ifndef __OS2__
	struct mtop t_op;
#endif
	off_t cnt;
	ssize_t n;

	/*
	 * If not a tape, try seeking on the file.  Seeking on a pipe is
	 * going to fail, but don't protect the user -- they shouldn't
	 * have specified the seek operand.
	 */
	if (!(out.flags & ISTAPE)) {
		if (lseek(out.fd, out.offset * out.dbsz, SEEK_SET) == -1)
			err(1, "%s", out.name);
		return;
	}

	/* If no read access, try using mtio. */
	if (out.flags & NOREAD) {
#ifdef __OS2__
        err(1, "%s", out.name);
#else
		t_op.mt_op = MTFSR;
		t_op.mt_count = out.offset;

		if (ioctl(out.fd, MTIOCTOP, &t_op) == -1)
			err(1, "%s", out.name);
		return;
#endif
	}

	/* Read it. */
	for (cnt = 0; cnt < out.offset; ++cnt) {
		if ((n = read(out.fd, out.db, out.dbsz)) > 0)
			continue;

		if (n == -1)
			err(1, "%s", out.name);

		/*
		 * If reach EOF, fill with NUL characters; first, back up over
		 * the EOF mark.  Note, cnt has not yet been incremented, so
		 * the EOF read does not count as a seek'd block.
		 */
#ifndef __OS2__
		t_op.mt_op = MTBSR;
		t_op.mt_count = 1;
		if (ioctl(out.fd, MTIOCTOP, &t_op) == -1)
			err(1, "%s", out.name);
#endif

		while (cnt++ < out.offset)
			if ((n = write(out.fd, out.db, out.dbsz)) != out.dbsz)
				err(1, "%s", out.name);
		break;
	}
}

void
summary(void)
{
#ifdef __OS2__
	time_t elapsed, now;
	double nanosecs;

	if (ddflags & C_NOINFO)
		return;

	now=time(0);
    elapsed=now-st.start;

	/* Be async safe: use dprintf(3). */
	fprintf(stderr, "%zu+%zu records in\n%zu+%zu records out\n",
	    st.in_full, st.in_part, st.out_full, st.out_part);

	if (st.swab) {
		fprintf(stderr, "%zu odd length swab %s\n",
		    st.swab, (st.swab == 1) ? "block" : "blocks");
	}
	if (st.trunc) {
		fprintf(stderr, "%zu truncated %s\n",
		    st.trunc, (st.trunc == 1) ? "block" : "blocks");
	}
#else
	struct timespec elapsed, now;
	double nanosecs;

	if (ddflags & C_NOINFO)
		return;

	clock_gettime(CLOCK_MONOTONIC, &now);
	timespecsub(&now, &st.start, &elapsed);
	nanosecs = ((double)elapsed.tv_sec * 1000000000) + elapsed.tv_nsec;
	if (nanosecs == 0)
		nanosecs = 1;

	/* Be async safe: use dprintf(3). */
	dprintf(STDERR_FILENO, "%zu+%zu records in\n%zu+%zu records out\n",
	    st.in_full, st.in_part, st.out_full, st.out_part);

	if (st.swab) {
		dprintf(STDERR_FILENO, "%zu odd length swab %s\n",
		    st.swab, (st.swab == 1) ? "block" : "blocks");
	}
	if (st.trunc) {
		dprintf(STDERR_FILENO, "%zu truncated %s\n",
		    st.trunc, (st.trunc == 1) ? "block" : "blocks");
	}
	if (!(ddflags & C_NOXFER)) {
		dprintf(STDERR_FILENO,
		    "%lld bytes transferred in %lld.%03ld secs "
		    "(%0.0f bytes/sec)\n", (long long)st.bytes,
		    (long long)elapsed.tv_sec, elapsed.tv_nsec / 1000000,
		    ((double)st.bytes * 1000000000) / nanosecs);
	}
#endif
}

void
summaryx(int notused)
{
	int save_errno = errno;

	summary();
	errno = save_errno;
}

void
terminate(int signo)
{
	summary();
	_exit(128 + signo);
}

/*
 * args -- parse JCL syntax of dd.
 */
void
jcl(char **argv)
{
	struct arg *ap, tmp;
	char *arg;

	in.dbsz = out.dbsz = 512;

	while (*++argv != NULL) {
		if ((oper = strdup(*argv)) == NULL)
			err(1, NULL);
		if ((arg = strchr(oper, '=')) == NULL)
			errx(1, "unknown operand %s", oper);
		*arg++ = '\0';
		if (!*arg)
			errx(1, "no value specified for %s", oper);
		tmp.name = oper;
		if (!(ap = (struct arg *)bsearch(&tmp, args,
		    sizeof(args)/sizeof(struct arg), sizeof(struct arg),
		    c_arg)))
			errx(1, "unknown operand %s", tmp.name);
		if (ddflags & ap->noset)
			errx(1, "%s: illegal argument combination or already set",
			    tmp.name);
		ddflags |= ap->set;
		ap->f(arg);
		free(oper);
	}

	/* Final sanity checks. */

	if (ddflags & C_BS) {
		/*
		 * Bs is turned off by any conversion -- we assume the user
		 * just wanted to set both the input and output block sizes
		 * and didn't want the bs semantics, so we don't warn.
		 */
		if (ddflags & (C_BLOCK|C_LCASE|C_SWAB|C_UCASE|C_UNBLOCK))
			ddflags &= ~C_BS;

		/* Bs supersedes ibs and obs. */
		if (ddflags & C_BS && ddflags & (C_IBS|C_OBS))
			warnx("bs supersedes ibs and obs");
	}

	/*
	 * Ascii/ebcdic and cbs implies block/unblock.
	 * Block/unblock requires cbs and vice-versa.
	 */
	if (ddflags & (C_BLOCK|C_UNBLOCK)) {
		if (!(ddflags & C_CBS))
			errx(1, "record operations require cbs");
		if (cbsz == 0)
			errx(1, "cbs cannot be zero");
		cfunc = ddflags & C_BLOCK ? block : unblock;
	} else if (ddflags & C_CBS) {
		if (ddflags & (C_ASCII|C_EBCDIC)) {
			if (ddflags & C_ASCII) {
				ddflags |= C_UNBLOCK;
				cfunc = unblock;
			} else {
				ddflags |= C_BLOCK;
				cfunc = block;
			}
		} else
			errx(1, "cbs meaningless if not doing record operations");
		if (cbsz == 0)
			errx(1, "cbs cannot be zero");
	} else
		cfunc = def;

	if (in.dbsz == 0 || out.dbsz == 0)
		errx(1, "buffer sizes cannot be zero");

	/*
	 * Read and write take size_t's as arguments.  Lseek, however,
	 * takes an off_t.
	 */
	if (cbsz > SSIZE_MAX || in.dbsz > SSIZE_MAX || out.dbsz > SSIZE_MAX)
		errx(1, "buffer sizes cannot be greater than %zd",
		    (ssize_t)SSIZE_MAX);
	if (in.offset > LLONG_MAX / in.dbsz || out.offset > LLONG_MAX / out.dbsz)
		errx(1, "seek offsets cannot be larger than %lld", LLONG_MAX);
}

static int
c_arg(const void *a, const void *b)
{

	return (strcmp(((struct arg *)a)->name, ((struct arg *)b)->name));
}

static void
f_bs(char *arg)
{

	in.dbsz = out.dbsz = get_bsz(arg);
}

static void
f_cbs(char *arg)
{

	cbsz = get_bsz(arg);
}

static void
f_count(char *arg)
{

	if ((cpy_cnt = get_bsz(arg)) == 0)
		cpy_cnt = (size_t)-1;
}

static void
f_files(char *arg)
{

	files_cnt = get_bsz(arg);
}

static void
f_ibs(char *arg)
{

	if (!(ddflags & C_BS))
		in.dbsz = get_bsz(arg);
}

static void
f_if(char *arg)
{
	if ((in.name = strdup(arg)) == NULL)
		err(1, NULL);
}

static void
f_obs(char *arg)
{

	if (!(ddflags & C_BS))
		out.dbsz = get_bsz(arg);
}

static void
f_of(char *arg)
{
	if ((out.name = strdup(arg)) == NULL)
		err(1, NULL);
}

static void
f_seek(char *arg)
{

	out.offset = get_off(arg);
}

static void
f_skip(char *arg)
{

	in.offset = get_off(arg);
}

static void
f_status(char *arg)
{

	if (strcmp(arg, "none") == 0)
		ddflags |= C_NOINFO;
	else if (strcmp(arg, "noxfer") == 0)
		ddflags |= C_NOXFER;
	else
		errx(1, "unknown status %s", arg);
}


static const struct conv {
	const char *name;
	unsigned set, noset;
	const unsigned char *ctab;
} clist[] = {
#ifndef	NO_CONV
	{ "ascii",	C_ASCII,	C_EBCDIC,	e2a_POSIX },
	{ "block",	C_BLOCK,	C_UNBLOCK,	NULL },
	{ "ebcdic",	C_EBCDIC,	C_ASCII,	a2e_POSIX },
	{ "fsync",	C_FSYNC,	0,		NULL },
	{ "ibm",	C_EBCDIC,	C_ASCII,	a2ibm_POSIX },
	{ "lcase",	C_LCASE,	C_UCASE,	NULL },
	{ "osync",	C_OSYNC,	C_BS,		NULL },
	{ "swab",	C_SWAB,		0,		NULL },
	{ "sync",	C_SYNC,		0,		NULL },
	{ "ucase",	C_UCASE,	C_LCASE,	NULL },
	{ "unblock",	C_UNBLOCK,	C_BLOCK,	NULL },
#endif
	{ "noerror",	C_NOERROR,	0,		NULL },
	{ "notrunc",	C_NOTRUNC,	0,		NULL },
	{ NULL,		0,		0,		NULL }
};

static void
f_conv(char *arg)
{
	const struct conv *cp;
	const char *name;

	while (arg != NULL) {
#ifdef __OS2__
		name = strtok(arg, ",");
#else
		name = strsep(&arg, ",");
#endif
		for (cp = &clist[0]; cp->name; cp++)
			if (strcmp(name, cp->name) == 0)
				break;
		if (!cp->name)
			errx(1, "unknown conversion %s", name);
		if (ddflags & cp->noset)
			errx(1, "%s: illegal conversion combination", name);
		ddflags |= cp->set;
		if (cp->ctab)
			ctab = cp->ctab;
	}
}

/*
 * Convert an expression of the following forms to a size_t
 *	1) A positive decimal number, optionally followed by
 *		b - multiply by 512.
 *		k, m or g - multiply by 1024 each.
 *		w - multiply by sizeof int
 *	2) Two or more of the above, separated by x
 *	   (or * for backwards compatibility), specifying
 *	   the product of the indicated values.
 */
static size_t
get_bsz(char *val)
{
	size_t num, t;
	char *expr;

	if (strchr(val, '-'))
		errx(1, "%s: illegal numeric value", oper);

	errno = 0;
	num = strtoul(val, &expr, 0);
	if (num == ULONG_MAX && errno == ERANGE)	/* Overflow. */
		err(1, "%s", oper);
	if (expr == val)			/* No digits. */
		errx(1, "%s: illegal numeric value", oper);

	switch(*expr) {
	case 'b':
		t = num;
		num *= 512;
		if (t > num)
			goto erange;
		++expr;
		break;
	case 'g':
	case 'G':
		t = num;
		num *= 1024;
		if (t > num)
			goto erange;
		/* fallthrough */
	case 'm':
	case 'M':
		t = num;
		num *= 1024;
		if (t > num)
			goto erange;
		/* fallthrough */
	case 'k':
	case 'K':
		t = num;
		num *= 1024;
		if (t > num)
			goto erange;
		++expr;
		break;
	case 'w':
		t = num;
		num *= sizeof(int);
		if (t > num)
			goto erange;
		++expr;
		break;
	}

	switch(*expr) {
		case '\0':
			break;
		case '*':			/* Backward compatible. */
		case 'x':
			t = num;
			num *= get_bsz(expr + 1);
			if (t > num)
				goto erange;
			break;
		default:
			errx(1, "%s: illegal numeric value", oper);
	}
	return (num);
erange:
	errx(1, "%s: math result not representable", oper);
}

/*
 * Convert an expression of the following forms to an off_t
 *	1) A positive decimal number, optionally followed by
 *		b - multiply by 512.
 *		k, m or g - multiply by 1024 each.
 *		w - multiply by sizeof int
 *	2) Two or more of the above, separated by x
 *	   (or * for backwards compatibility), specifying
 *	   the product of the indicated values.
 */
static off_t
get_off(char *val)
{
	off_t num, t;
	char *expr;

	errno = 0;
	num = strtoll(val, &expr, 0);
	if (num == LLONG_MAX && errno == ERANGE)	/* Overflow. */
		err(1, "%s", oper);
	if (expr == val)			/* No digits. */
		errx(1, "%s: illegal numeric value", oper);

	switch(*expr) {
	case 'b':
		t = num;
		num *= 512;
		if (t > num)
			goto erange;
		++expr;
		break;
	case 'g':
	case 'G':
		t = num;
		num *= 1024;
		if (t > num)
			goto erange;
		/* fallthrough */
	case 'm':
	case 'M':
		t = num;
		num *= 1024;
		if (t > num)
			goto erange;
		/* fallthrough */
	case 'k':
	case 'K':
		t = num;
		num *= 1024;
		if (t > num)
			goto erange;
		++expr;
		break;
	case 'w':
		t = num;
		num *= sizeof(int);
		if (t > num)
			goto erange;
		++expr;
		break;
	}

	switch(*expr) {
		case '\0':
			break;
		case '*':			/* Backward compatible. */
		case 'x':
			t = num;
			num *= get_off(expr + 1);
			if (t > num)
				goto erange;
			break;
		default:
			errx(1, "%s: illegal numeric value", oper);
	}
	return (num);
erange:
	errx(1, "%s: math result not representable", oper);
}

#ifdef __OS2__
char *getprogname(void) {return __argv[0];}
#endif

int
main(int argc, char *argv[])
{
	jcl(argv);
	setup();

    /* uso: Linux doesn't have SIGINFO.
            GNU dd(1) uses SIGUSR1 instead, so do that here. */
	(void)signal(SIGUSR1, summaryx);
	(void)signal(SIGINT, terminate);

	atexit(summary);

	if (cpy_cnt != (size_t)-1) {
		while (files_cnt--)
			dd_in();
	}

	dd_close();
	exit(0);
}

static void
setup(void)
{
	if (in.name == NULL) {
		in.name = "stdin";
		in.fd = STDIN_FILENO;
#ifdef __OS2__
        setmode(fileno(stdin),O_BINARY);
#endif
	} else {
		in.fd = open(in.name, O_RDONLY | O_BINARY, 0);
		if (in.fd == -1)
			err(1, "%s", in.name);
	}

	getfdtype(&in);

	if (files_cnt > 1 && !(in.flags & ISTAPE))
		errx(1, "files is not supported for non-tape devices");

	if (out.name == NULL) {
		/* No way to check for read access here. */
		out.fd = STDOUT_FILENO;
		out.name = "stdout";
#ifdef __OS2__
        setmode(fileno(stdout),O_BINARY);
#endif
	} else {
#define	OFLAGS \
    (O_CREAT | (ddflags & (C_SEEK | C_NOTRUNC) ? 0 : O_TRUNC))
		out.fd = open(out.name, O_RDWR | OFLAGS | O_BINARY, DEFFILEMODE);
		/*
		 * May not have read access, so try again with write only.
		 * Without read we may have a problem if output also does
		 * not support seeks.
		 */
		if (out.fd == -1) {
			out.fd = open(out.name, O_WRONLY | OFLAGS | O_BINARY, DEFFILEMODE);
			out.flags |= NOREAD;
		}
		if (out.fd == -1)
			err(1, "%s", out.name);
	}

	getfdtype(&out);

	/*
	 * Allocate space for the input and output buffers.  If not doing
	 * record oriented I/O, only need a single buffer.
	 */
	if (!(ddflags & (C_BLOCK|C_UNBLOCK))) {
		if ((in.db = malloc(out.dbsz + in.dbsz - 1)) == NULL)
			err(1, "input buffer");
		out.db = in.db;
	} else {
		in.db = malloc(MAXIMUM(in.dbsz, cbsz) + cbsz);
		if (in.db == NULL)
			err(1, "input buffer");
		out.db = malloc(out.dbsz + cbsz);
		if (out.db == NULL)
			err(1, "output buffer");
	}
	in.dbp = in.db;
	out.dbp = out.db;

	/* Position the input/output streams. */
	if (in.offset)
		pos_in();
	if (out.offset)
		pos_out();

	/*
	 * Truncate the output file; ignore errors because it fails on some
	 * kinds of output files, tapes, for example.
	 */
#ifndef __OS2__
	if ((ddflags & (C_OF | C_SEEK | C_NOTRUNC)) == (C_OF | C_SEEK))
		(void)ftruncate(out.fd, out.offset * out.dbsz);
#endif

	/*
	 * If converting case at the same time as another conversion, build a
	 * table that does both at once.  If just converting case, use the
	 * built-in tables.
	 */
	if (ddflags & (C_LCASE|C_UCASE)) {
#ifdef	NO_CONV
		/* Should not get here, but just in case... */
		errx(1, "case conv and -DNO_CONV");
#else	/* NO_CONV */
		unsigned cnt;
		if (ddflags & C_ASCII || ddflags & C_EBCDIC) {
			if (ddflags & C_LCASE) {
				for (cnt = 0; cnt < 0377; ++cnt)
					casetab[cnt] = tolower(ctab[cnt]);
			} else {
				for (cnt = 0; cnt < 0377; ++cnt)
					casetab[cnt] = toupper(ctab[cnt]);
			}
		} else {
			if (ddflags & C_LCASE) {
				for (cnt = 0; cnt < 0377; ++cnt)
					casetab[cnt] = tolower(cnt);
			} else {
				for (cnt = 0; cnt < 0377; ++cnt)
					casetab[cnt] = toupper(cnt);
			}
		}

		ctab = casetab;
#endif	/* NO_CONV */
	}

	/* Statistics timestamp. */
#ifdef __OS2__
	st.start=time(0);
#else
	clock_gettime(CLOCK_MONOTONIC, &st.start);
#endif
}

static void
getfdtype(IO *io)
{
#ifndef __OS2__
	struct mtget mt;
#endif
	struct stat sb;

	if (fstat(io->fd, &sb))
		err(1, "%s", io->name);
#ifndef __OS2__
	if (S_ISCHR(sb.st_mode))
		io->flags |= ioctl(io->fd, MTIOCGET, &mt) ? ISCHR : ISTAPE;
	if (S_ISFIFO(sb.st_mode) || S_ISSOCK(sb.st_mode))
		io->flags |= ISPIPE;
#endif
}

static void
swapbytes(void *v, size_t len)
{
	unsigned char *p = v;
	unsigned char t;

	while (len > 1) {
		t = p[0];
		p[0] = p[1];
		p[1] = t;
		p += 2;
		len -= 2;
	}
}


static void
dd_in(void)
{
	ssize_t n;

	for (;;) {
		if (cpy_cnt && (st.in_full + st.in_part) >= cpy_cnt)
			return;

		/*
		 * Zero the buffer first if sync; if doing block operations
		 * use spaces.
		 */
		if (ddflags & C_SYNC) {
			if (ddflags & (C_BLOCK|C_UNBLOCK))
				(void)memset(in.dbp, ' ', in.dbsz);
			else
				(void)memset(in.dbp, 0, in.dbsz);
		}

		n = read(in.fd, in.dbp, in.dbsz);
		if (n == 0) {
			in.dbrcnt = 0;
			return;
		}

		/* Read error. */
		if (n == -1) {
			/*
			 * If noerror not specified, die.  POSIX requires that
			 * the warning message be followed by an I/O display.
			 */
			if (!(ddflags & C_NOERROR))
				err(1, "%s", in.name);
			warn("%s", in.name);
			summary();

			/*
			 * If it's not a tape drive or a pipe, seek past the
			 * error.  If your OS doesn't do the right thing for
			 * raw disks this section should be modified to re-read
			 * in sector size chunks.
			 */
			if (!(in.flags & (ISPIPE|ISTAPE)) &&
			    lseek(in.fd, (off_t)in.dbsz, SEEK_CUR))
				warn("%s", in.name);

			/* If sync not specified, omit block and continue. */
			if (!(ddflags & C_SYNC))
				continue;

			/* Read errors count as full blocks. */
			in.dbcnt += in.dbrcnt = in.dbsz;
			++st.in_full;

		/* Handle full input blocks. */
		} else if (n == in.dbsz) {
			in.dbcnt += in.dbrcnt = n;
			++st.in_full;

		/* Handle partial input blocks. */
		} else {
			/* If sync, use the entire block. */
			if (ddflags & C_SYNC)
				in.dbcnt += in.dbrcnt = in.dbsz;
			else
				in.dbcnt += in.dbrcnt = n;
			++st.in_part;
		}

		/*
		 * POSIX states that if bs is set and no other conversions
		 * than noerror, notrunc or sync are specified, the block
		 * is output without buffering as it is read.
		 */
		if (ddflags & C_BS) {
			out.dbcnt = in.dbcnt;
			dd_out(1);
			in.dbcnt = 0;
			continue;
		}

		if (ddflags & C_SWAB) {
			if ((n = in.dbrcnt) & 1) {
				++st.swab;
				--n;
			}
			swapbytes(in.dbp, n);
		}

		in.dbp += in.dbrcnt;
		(*cfunc)();
	}
}

/*
 * Cleanup any remaining I/O and flush output.  If necessary, output file
 * is truncated.
 */
static void
dd_close(void)
{
	if (cfunc == def)
		def_close();
	else if (cfunc == block)
		block_close();
	else if (cfunc == unblock)
		unblock_close();
	if (ddflags & C_OSYNC && out.dbcnt && out.dbcnt < out.dbsz) {
		if (ddflags & (C_BLOCK|C_UNBLOCK))
			memset(out.dbp, ' ', out.dbsz - out.dbcnt);
		else
			memset(out.dbp, 0, out.dbsz - out.dbcnt);
		out.dbcnt = out.dbsz;
	}
	if (out.dbcnt)
		dd_out(1);
	if (ddflags & C_FSYNC) {
		if (fsync(out.fd) == -1)
			err(1, "fsync %s", out.name);
	}
}

void
dd_out(int force)
{
	static int warned;
	size_t cnt, n;
	ssize_t nw;
	unsigned char *outp;

	/*
	 * Write one or more blocks out.  The common case is writing a full
	 * output block in a single write; increment the full block stats.
	 * Otherwise, we're into partial block writes.  If a partial write,
	 * and it's a character device, just warn.  If a tape device, quit.
	 *
	 * The partial writes represent two cases.  1: Where the input block
	 * was less than expected so the output block was less than expected.
	 * 2: Where the input block was the right size but we were forced to
	 * write the block in multiple chunks.  The original versions of dd(1)
	 * never wrote a block in more than a single write, so the latter case
	 * never happened.
	 *
	 * One special case is if we're forced to do the write -- in that case
	 * we play games with the buffer size, and it's usually a partial write.
	 */
	outp = out.db;
	for (n = force ? out.dbcnt : out.dbsz;; n = out.dbsz) {
		for (cnt = n;; cnt -= nw) {
			nw = write(out.fd, outp, cnt);
			if (nw == 0)
				errx(1, "%s: end of device", out.name);
			if (nw == -1) {
				if (errno != EINTR)
					err(1, "%s", out.name);
				nw = 0;
			}
			outp += nw;
			st.bytes += nw;
			if (nw == n) {
				if (n != out.dbsz)
					++st.out_part;
				else
					++st.out_full;
				break;
			}
			++st.out_part;
			if (nw == cnt)
				break;
			if (out.flags & ISCHR && !warned) {
				warned = 1;
				warnx("%s: short write on character device",
				    out.name);
			}
			if (out.flags & ISTAPE)
				errx(1, "%s: short write on tape device", out.name);
		}
		if ((out.dbcnt -= n) < out.dbsz)
			break;
	}

	/* Reassemble the output block. */
	if (out.dbcnt)
		(void)memmove(out.db, out.dbp - out.dbcnt, out.dbcnt);
	out.dbp = out.db + out.dbcnt;
}

