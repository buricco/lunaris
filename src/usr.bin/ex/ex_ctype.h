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

/* ctype.h */

/* This file contains macros definitions and extern declarations for a
 * version of <ctype.h> which is aware of the o_flipcase letters used in
 * elvis.
 *
 * This file uses the "uchar" data type and "UCHAR" conversion macro which
 * are defined in "config.h".  Consequently, any file that includes this
 * header must include config.h first.
 */

#ifndef _CT_UPPER

#define _CT_UPPER	0x01
#define _CT_LOWER	0x02
#define _CT_SPACE	0x04
#define _CT_DIGIT	0x08
#define _CT_ALNUM	0x10
#define _CT_CNTRL	0x20

#define isalnum(c)	(_ct_ctypes[UCHAR(c)] & _CT_ALNUM)
#define isalpha(c)	(_ct_ctypes[UCHAR(c)] & (_CT_LOWER|_CT_UPPER))
#define isdigit(c)	(_ct_ctypes[UCHAR(c)] & _CT_DIGIT)
#define islower(c)	(_ct_ctypes[UCHAR(c)] & _CT_LOWER)
#define isspace(c)	(_ct_ctypes[UCHAR(c)] & _CT_SPACE)
#define isupper(c)	(_ct_ctypes[UCHAR(c)] & _CT_UPPER)
#define iscntrl(c)	(_ct_ctypes[UCHAR(c)] & _CT_CNTRL)
#define ispunct(c)	(!_ct_ctypes[UCHAR(c)]) /* punct = "none of the above" */

#define isascii(c)	(!((c) & 0x80))

#define toupper(c)	_ct_toupper[UCHAR(c)]
#define tolower(c)	_ct_tolower[UCHAR(c)]

extern uchar	_ct_toupper[];
extern uchar	_ct_tolower[];
extern uchar	_ct_ctypes[];

extern void	_ct_init (uchar *);

#endif /* ndef _CT_UPPER */
