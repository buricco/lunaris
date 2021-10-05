/*
 * SPDX-License-Identifier: NCSA
 * 
 * dmesg(8) - print or control the system message buffer
 * (implemented according to LSB 4.1 Core)
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

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/klog.h>

char *progname;

void usage (void)
{
 fprintf (stderr, "%s: usage: %s [-c] [-n level] [-s bufsize]\n",
          progname, progname);
 exit(1);
}

/*
 * switches:
 * 
 *   -c       - clear after displaying
 *   -n #     - set log level
 *   -s ##### - set buffer size (16392)
 */

int main (int argc, char **argv)
{
 int e;
 int n,s;
 int c;
 char *p;

 c=0;
 n=-1; s=-1;
 
 progname=argv[0];
 
 while (-1!=(e=getopt(argc, argv, "cn:s:")))
 {
  switch (e)
  {
   case 'c':
    c=1;
    break;
   case 'n':
    n=atoi(optarg);
    break;
   case 's':
    s=atoi(optarg);
    break;
  }
 }
 
 if (n!=-1)
 {
  e=klogctl(8, 0, n);
  if (e<0) perror(progname);
  return e;
 }
 
 e=klogctl(10,0,0);
 if (e<0)
 {
  perror(progname);
  return e;
 }
 p=malloc(e);
 if (!p)
 {
  fprintf (stderr, "%s: could not allocate %u bytes for read\n", progname, e);
  return 1;
 }
 e=klogctl(3,p,(s==-1)?16392:s);
 fwrite(p,(s==-1)?16392:s,1,stdout);
 free(p);
 
 if (c)
 {
  e=klogctl(5, 0, 0);
  if (e<0) perror(progname);
  return e;
 }
 
 return 0;
}
