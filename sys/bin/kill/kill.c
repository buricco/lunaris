/*
 * SPDX-License-Identifier: NCSA
 * 
 * kill(1) - terminate or signal processes
 * (implemented according to IEEE 1003.1-2017 
 *  with -n extension from util-linux)
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

#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "signames.h"

int globargc;
static char *argv0;

void usage (void)
{
 fprintf (stderr, "usage: %s [-SIGNAL] process ...\n"
                  "       %s [-s SIGNAME] process ...\n"
                  "       %s [-n SIGNUM] process ...\n"
                  "       %s -l [SIGNUM ...]\n", argv0, argv0, argv0, argv0);
 exit(1);
}

void nope (void)
{
 fprintf (stderr, "%s: not a valid signal\n", argv0);
 exit(2);
}

void doit (int sig, char **argv, int start)
{
 int t;
 
 for (t=start; t<globargc; t++)
 {
  int n;
  
  n=atoi(argv[t]);

  switch (kill(n,sig))
  {
   case EINVAL:
    nope();
   case EPERM:
    fprintf (stderr, "%s: access denied to pid %u\n", argv0, n);
    exit(3);
   case ESRCH:
    fprintf (stderr, "%s: no such process %u\n", argv0, n);
    exit(4);
  }
 }
 exit(0);
}

int main (int argc, char **argv)
{
 int n;
 
 globargc=argc;
 argv0=argv[0];
 
 if (argc==1)
 {
  usage();
 }
 
 if (!strcmp(argv[1],"-l"))
 {
  if (argc==2)
  {
   const char *x;
   
   for (n=1; x=signum_to_signame(n); n++)
   {
    printf ("%u: %s\n",n,x);
   }
   
   return 0;
  }
  else
  {
   int t;
   
   for (t=2; t<argc; t++)
   {
    if (isdigit(argv[t][0]))
    {
     const char *x;
     n=atoi(argv[t]);
     x=signum_to_signame(n);
     if (!x) nope();
     printf ("%s\n", x);
    }
    else
    {
     int x;
     x=signame_to_signum(argv[t]);
     if (!x) nope();
     printf ("%u\n",x);
    }
   }
   
   return 0;
  }
 }
 
 if (argv[1][0]=='-')
 {
  if (argv[1][1]=='s')
  {
   if (argv[1][2])
   {
    if (argc<3) usage();
    n=signame_to_signum(&(argv[1][2]));
    if (!n) nope();
    doit(n,argv,2);
   }
   else
   {
    if (argc<4) usage();
    n=signame_to_signum(argv[2]);
    if (!n) nope();
    doit(n,argv,3);
   }
  }
  if (argv[1][1]=='n')
  {
   if (argv[1][2])
   {
    if (argc<3) usage();
    n=atoi(&(argv[1][2]));
    doit(n,argv,2);
   }
   else
   {
    if (argc<4) usage();
    n=atoi(argv[2]);
    doit(n,argv,3);
   }
  }
  else
  {
   int n;
   
   if (argc==2) usage();
   if (isdigit(argv[1][1]))
   {
    n=atoi(&(argv[1][1]));
    doit(n,argv,2);
   }
   else
   {
    n=signame_to_signum(&(argv[1][1]));
    if (!n) nope();
    doit(n,argv,2);
   }
  }
 }
 
 doit(SIGTERM,argv,1);
}
