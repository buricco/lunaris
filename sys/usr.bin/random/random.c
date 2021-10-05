/*
 * SPDX-License-Identifier: NCSA
 * 
 * random(1) - generate a random number
 *             (as according to System V Release 4 documentation)
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

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

char *argv0;

void usage (void)
{
 fprintf (stderr, "%s: usage: %s [-s] [scale]\n", argv0, argv0);
 exit(0);
}

int main (int argc, char **argv)
{
 int e;
 unsigned scale, sflag;
 
 srand(time(0));
 
 scale=1;
 sflag=0;
 argv0=argv[0];
 
 while (-1!=(e=getopt(argc, argv, "s")))
 {
  switch (e)
  {
   case 's':
    sflag=1;
    break;
   default:
    usage();
  }
 }
 
 if (argc-optind>1)
  usage();
 
 if (argc-optind)
 {
  scale=atoi(argv[optind]);
  if ((!scale)||(scale>255))
  {
   fprintf (stderr, "%s: invalid scale '%s'\n", argv0, argv[optind]);
   return 0;
  }
 }

 e=rand()%(scale+1);
 if (!sflag) printf ("%u\n", e);
 return e;
}
