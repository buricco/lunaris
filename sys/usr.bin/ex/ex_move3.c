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

/* move3.c */

/* This file contains movement functions that perform character searches */

#include "config.h"
#include "ex_common.h"

#ifndef NO_CHARSEARCH
static MARK	(*prevfwdfn) (MARK,long,int);	/* function to search in same direction */
static MARK	(*prevrevfn) (MARK,long,int);	/* function to search in opposite direction */
static char	prev_key;	/* sought cvhar from previous [fFtT] */

MARK	m__ch(MARK m, long cnt, int cmd)
{
	MARK	(*tmp)();

	if (!prevfwdfn)
	{
		msg("No previous f, F, t, or T command");
		return MARK_UNSET;
	}

	if (cmd == ',')
	{
		m =  (*prevrevfn)(m, cnt, prev_key);

		/* Oops! we didn't want to change the prev*fn vars! */
		tmp = prevfwdfn;
		prevfwdfn = prevrevfn;
		prevrevfn = tmp;

		return m;
	}
	else
	{
		return (*prevfwdfn)(m, cnt, prev_key);
	}
}

/* move forward within this line to next occurrence of key */
MARK	m_fch(MARK m, long cnt, int key)
{
	REG char	*text;

	DEFAULT(1);

	prevfwdfn = m_fch;
	prevrevfn = m_Fch;
	prev_key = key;

	pfetch(markline(m));
	text = ptext + markidx(m);
	while (cnt-- > 0)
	{
		do
		{
			m++;
			text++;
		} while (*text && *text != key);
	}
	if (!*text)
	{
		return MARK_UNSET;
	}
	return m;
}

/* move backward within this line to previous occurrence of key */
MARK	m_Fch(MARK m, long cnt, int key)
{
	REG char	*text;

	DEFAULT(1);

	prevfwdfn = m_Fch;
	prevrevfn = m_fch;
	prev_key = key;

	pfetch(markline(m));
	text = ptext + markidx(m);
	while (cnt-- > 0)
	{
		do
		{
			m--;
			text--;
		} while (text >= ptext && *text != key);
	}
	if (text < ptext)
	{
		return MARK_UNSET;
	}
	return m;
}

/* move forward within this line almost to next occurrence of key */
MARK	m_tch(MARK m, long cnt, int key)
{
	m = m_fch(m, cnt, key);
	if (m != MARK_UNSET)
	{
		m--;
	}
	prevfwdfn = m_tch;
	prevrevfn = m_Tch;
	return m;
}

/* move backward within this line almost to previous occurrence of key */
MARK	m_Tch(MARK m, long cnt, int key)
{
	m = m_Fch(m, cnt, key);
	if (m != MARK_UNSET)
	{
		m++;
	}
	prevfwdfn = m_Tch;
	prevrevfn = m_tch;

	return m;
}
#endif
