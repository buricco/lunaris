/*
 * SPDX-License-Identifier: NCSA
 * 
 * mkdir(1) - make directories
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

mode_t getmode(const void *bbox, mode_t omode);
void *setmode(const char *p);

void usage (char *argv0)
{
 fprintf (stderr, "%s: usage: %s [-m mode] [-p] dirname ...\n", argv0, argv0);
 exit(1);
}

int mkdir_with_parents (char *path, mode_t mode)
{
 if (strrchr(path,'/'))
 {
  int e;
  char *p;
  
  p=malloc(strlen(path)+1);
  if (!p) return -1;
  strcpy(p,path);
  *(strrchr(p,'/'))=0;
  
  /* Ignore errors */  
  mkdir_with_parents(p, mode);
  free(p);
 }
 
 return mkdir(path, mode);
}

int main (int argc, char **argv)
{
 void *m;
 int r,s;
 int e, mode, pflag;
 
 mode=0755;
 
 while (-1!=(e=getopt(argc, argv, "m:p")))
 {
  switch (e)
  {
   case 'm':
    m=setmode(optarg);
    if (!m)
    {
     fprintf (stderr, "%s: invalid mode '%s'\n", argv[0], optarg);
     return 1;
    }
    mode=getmode(m,0755);
    free(m);
   case 'p':
    pflag=1;
    break;
   default:
    usage(argv[0]);
  }
 }
 
 if (optind==argc) usage(argv[0]);
 
 r=0;
 
 for (e=optind; e<argc; e++)
 {
  if (pflag)
  {
   if (0!=(s=mkdir_with_parents(argv[optind], mode))) perror(argv[optind]);
  }
  else
  {
   if (0!=(s=mkdir(argv[optind], mode))) perror(argv[optind]);
  }
  r+=s;
 }
 
 return r;
}
