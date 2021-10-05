/*
 * SPDX-License-Identifier: NCSA
 * 
 * readlink(1) - display target of symbolic link on standard output
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

#include <sys/stat.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int tracelink (char *filename, int recurse, int nonl)
{
 struct stat st;
 off_t sz;
 char p[PATH_MAX+1], *q;
 char *s;
 
 s=nonl?"%s":"%s\n";

 memset(p,0,PATH_MAX+1);
 
 if (lstat(filename,&st))
 {
  perror(filename);
  return -1;
 }
 
 if (!(S_ISLNK(st.st_mode))) return 1;
 
 if (recurse)
 {
  q=realpath(filename, p);
  if (!q) return -1;
  
  printf (s, p);
  return 0;
 }
 else
 {
  sz=st.st_size;
  readlink(filename,p,PATH_MAX);
  printf (s, p);
  return 0;
 }
}

void usage(char *argv0)
{
 fprintf (stderr, "%s: usage: %s [-fn] filename \n", argv0, argv0);
 exit(1);
}

int main (int argc, char **argv)
{
 int e, f, n;
 
 f=n=0;
 
 while (-1!=(e=getopt(argc, argv, "fn")))
 {
  switch (e)
  {
   case 'f':
    f++;
    break;
   case 'n':
    n++;
    break;
   default:
    usage(argv[0]);
  }
 }
 
 if ((argc-optind)!=1) usage(argv[0]);
 
 return tracelink (argv[optind], f, n);
}
