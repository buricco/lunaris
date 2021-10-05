/*
 * SPDX-License-Identifier: NCSA
 * 
 * realpath(1) - return resolved physical path
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

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main (int argc, char **argv)
{
 char buf[PATH_MAX];
 
 int e, q, r;
 
 q=0;
 
 while (-1!=(e=getopt(argc, argv, "q")))
 {
  switch (e)
  {
   case 'q':
    q=1;
    break;
   default:
    fprintf (stderr, "%s: usage: %s [-q] [path ...]\n", argv[0], argv[0]);
    return 1;
  }
 }
 
 if (optind==argc)
 {
  if (!realpath(".",buf))
  {
   perror(".");
   return 1;
  }
  printf ("%s\n", buf);
  return 0;
 }
 
 r=0;
 for (e=optind; e<argc; e++)
 {
  if (!realpath(argv[e], buf))
  {
   r++;
   if (!q) perror(argv[e]);
  }
  else
   printf ("%s\n", buf);
 }
 return r;
}
