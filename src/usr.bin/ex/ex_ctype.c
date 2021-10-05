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

/* ctype.c */

/* This file contains the tables and initialization function for elvis'
 * version of <ctype.h>.  It should be portable.
 */

#include "config.h"
#include "ex_ctype.h"

void _ct_init (uchar *);

uchar	_ct_toupper[256];
uchar	_ct_tolower[256];
uchar	_ct_ctypes[256];

/* This function initializes the tables used by the ctype macros.  It should
 * be called at the start of the program.  It can be called again anytime you
 * wish to change the non-standard "flipcase" list.  The "flipcase" list is
 * a string of characters which are taken to be lowercase/uppercase pairs.
 * If you don't want to use any special flipcase characters, then pass an
 * empty string.
 */
void _ct_init(uchar *flipcase)
{
	int	i;
	uchar	*scan;

	/* reset all of the tables */
	for (i = 0; i < 256; i++)
	{
		_ct_toupper[i] = _ct_tolower[i] = i;
		_ct_ctypes[i] = 0;
	}

	/* add the digits */
	for (scan = (uchar *)"0123456789"; *scan; scan++)
	{
		_ct_ctypes[*scan] |= _CT_DIGIT | _CT_ALNUM;
	}

	/* add the whitespace */
	for (scan = (uchar *)" \t\n\r\f"; *scan; scan++)
	{
		_ct_ctypes[*scan] |= _CT_SPACE;
	}

	/* add the standard ASCII letters */
	for (scan = (uchar *)"aAbBcCdDeEfFgGhHiIjJkKlLmMnNoOpPqQrRsStTuUvVwWxXyYzZ"; *scan; scan += 2)
	{
		_ct_ctypes[scan[0]] |= _CT_LOWER | _CT_ALNUM;
		_ct_ctypes[scan[1]] |= _CT_UPPER | _CT_ALNUM;
		_ct_toupper[scan[0]] = scan[1];
		_ct_tolower[scan[1]] = scan[0];
	}

	/* add the flipcase letters */
	for (scan = flipcase; scan[0] && scan[1]; scan += 2)
	{
		_ct_ctypes[scan[0]] |= _CT_LOWER | _CT_ALNUM;
		_ct_ctypes[scan[1]] |= _CT_UPPER | _CT_ALNUM;
		_ct_toupper[scan[0]] = scan[1];
		_ct_tolower[scan[1]] = scan[0];
	}

	/* include '_' in the isalnum() list */
	_ct_ctypes[UCHAR('_')] |= _CT_ALNUM;

	/* !!! find the control characters in an ASCII-dependent way */
	for (i = 0; i < ' '; i++)
	{
		_ct_ctypes[i] |= _CT_CNTRL;
	}
	_ct_ctypes[127] |= _CT_CNTRL;
	_ct_ctypes[255] |= _CT_CNTRL;
}
