/*
 * SPDX-License-Identifier: NCSA
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/swap.h>

static char *argv0;

void usage (void)
{
 fprintf (stderr, "%s: usage: %s -a\n"
                  "%s: usage: %s device\n"
                  "%s: usage: %s file\n", 
          argv0, argv0, argv0, argv0, argv0, argv0);
 exit(1);
}

int oneswapoff (char *which)
{
 int e;
 
 e=swapoff(which);
 if (e) perror(which);
 return e;
}

int allswapsoff (void)
{
 FILE *file;
 char *l, *p, *r;
 int e, o;
 ssize_t s;
 size_t n;
 
 file=fopen("/proc/swaps", "rt");
 if (!file)
 {
  perror("/proc/swaps");
  return -1;
 }

 l=0;
 e=0;
 
 /* Eat the first line */
 s=getline(&l, &n, file);
 r=strstr(l, "Type");
 if (!r)
 {
  fclose(file);
  fprintf (stderr, "%s: invalid /prog/swaps format (1)\n", argv0);
  return -1;
 }
 o=r-l;
 while (1)
 {
  int v;
  if (l) free(l);
  
  n=0;
  
  s=getline(&l, &n, file);
  if (feof(file))
   break;
  
  r=strstr(l, "partition");
  if (!r) r=strstr(l, "file");
  if (!r)
  {
   free(l);
   fclose(file);
   fprintf (stderr, "%s: invalid /prog/swaps format (2)\n", argv0);
   return -1;
  }

  r--;  
  while (*r==' ') r--;
  r[1]=0;
  if (0!=(v=swapoff(l))) perror(l);
  e+=v;
 }
 if (l) free(l);
 l=0;
 fclose(file);
 return e;
}

int main (int argc, char **argv)
{
 int e;
 int a;
 
 argv0=argv[0];
 
 a=0;
 
 while (-1!=(e=getopt(argc, argv, "a")))
 {
  switch (e)
  {
   case 'a':
    a=1;
    break;
   default:
    usage();
  }
 }
 
 if (a)
 {
  if (optind!=argc) usage();
 }
 else
 {
  if (argc==optind) usage();
 }
  
 if (a) 
  return allswapsoff(); 
 else
  return oneswapoff(argv[optind]);
}
