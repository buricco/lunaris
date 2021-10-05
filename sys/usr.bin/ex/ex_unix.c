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

/* unix.c */

#include "config.h"
#include "ex_common.h"

/* For System-V, we use VMIN/VTIME to implement the timeout.  For no timeout,
 * VMIN should be 1 and VTIME should be 0; for timeout, VMIN should be 0 and
 * VTIME should be the timeout value.
 */

#include <termios.h>

int ttyread(char *buf, int len, int time)
{
	struct termios tio;
	int	bytes;	/* number of bytes actually read */

	/* arrange for timeout */
	tcgetattr(0, &tio);
	if (time)
	{
		tio.c_cc[VMIN] = 0;
		tio.c_cc[VTIME] = time;
	}
	else
	{
		tio.c_cc[VMIN] = 1;
		tio.c_cc[VTIME] = 0;
	}
	tcsetattr(0, TCSANOW, &tio);

	/* Perform the read.  Loop if EINTR error happens */
	while ((bytes = read(0, buf, (unsigned)len)) < 0)
	{
		/* probably EINTR error because a SIGWINCH was received */
		if (*o_lines != LINES || *o_columns != COLS)
		{
			*o_lines = LINES;
			*o_columns = COLS;
			if (mode != MODE_EX)
			{
				/* pretend the user hit ^L */
				*buf = ctrl('L');
				return 1;
			}
		}
	}

	/* return the number of bytes read */
	return bytes;

	/* NOTE: The terminal may be left in a timeout-mode after this function
	 * returns.  This shouldn't be a problem since Elvis *NEVER* tries to
	 * read from the keyboard except through this function.
	 */
}

