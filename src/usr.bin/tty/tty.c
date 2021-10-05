/*
 * SPDX-License-Identifier: NCSA
 * 
 * tty - return user's terminal name
 * (implemented according to IEEE 1003.1-2017)
 * 
 * Copyright 2020 S. V. Nickolas.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal with the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimers.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimers in the 
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the names of the authors nor the names of contributors may be
 *    used to endorse or promote products derived from this Software without
 *    specific prior written permission.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE CONTRIBUTORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS WITH THE SOFTWARE.
 */

/*
 * Current standard removes the -s switch.  -l from sysv doesn't seem to be
 * appropriate for modern *x anyway.
 */
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main (int argc, char **argv)
{
 if (argc==1)
 {
  if (isatty(fileno(stdin)))
  {
   printf ("%s\n",ttyname(fileno(stdin)));
   return 0;
  }
  else
  {
   printf ("not a tty\n");
   return 1;
  }
 }

 fprintf (stderr, "%s: usage: %s\n", argv[0], argv[0]);
 return 2;
}
