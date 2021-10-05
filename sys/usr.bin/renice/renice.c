/*
 * SPDX-License-Identifier: NCSA
 * 
 * renice(8) - set nice(2) values of running processes
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

#include <sys/resource.h>
#include <sys/time.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define RN_ISPID  0x00
#define RN_ISPGID 0x01
#define RN_ISUID  0x02

static char *argv0;

extern int uid_from_user (const char *, uid_t *);

int do_renice (int n, int v, int p)
{
 int c, r;
 
 r=setpriority(p, v, getpriority(p, v)+n);
 if (!r) return 0;
 fprintf (stderr, "%s: %d: %s\n", argv0, c, strerror(errno));
 return 1;
}

int renice_pid (int n, int optind, int argc, char **argv)
{
 int e,t;
 
 e=0;
 for (t=optind; t<argc; t++)
  e+=do_renice(n, atoi(argv[t]), PRIO_PROCESS);
 
 return e;
}

int renice_pgid (int n, int optind, int argc, char **argv)
{
 int e,t;
 
 e=0;
 for (t=optind; t<argc; t++)
  e+=do_renice(n, atoi(argv[t]), PRIO_PGRP);
 
 return e;
}

int renice_uid (int n, int optind, int argc, char **argv)
{
 int e,t,v;
 
 e=0;
 for (t=optind; t<argc; t++)
 {
  if (!(uid_from_user(argv[t], &v)))
  {
   fprintf (stderr, "%s: no such user '%s'\n", argv0, argv[t]);
   return -1;
  }
  
  e+=do_renice(n, v, PRIO_USER);
 }
 
 return e;
}

void usage (void)
{
 fprintf (stderr, "%s: usage: %s [-g | -p | -u] -n increment id ...\n",
          argv0, argv0);
 exit(1);
}

int main (int argc, char **argv)
{
 int e, n;
 int mode;
 
 argv0=argv[0];
 
 e=0;
 n=0;
 mode=RN_ISPID;
 while (-1!=(e=getopt(argc, argv, "gpun:")))
 {
  switch (e)
  {
   case 'g':
    mode=RN_ISPGID;
    break;
   case 'p':
    mode=RN_ISPID;
    break;
   case 'u':
    mode=RN_ISUID;
    break;
   case 'n':
    n=atoi(optarg);
    break;
   default:
    usage();
  }
 }
 
 if (optind==argc) usage();
 switch (mode)
 {
  case (RN_ISPID):
   return renice_pid(n, optind, argc, argv);
  case (RN_ISPGID):
   return renice_pgid(n, optind, argc, argv);
  case (RN_ISUID):
   return renice_uid(n, optind, argc, argv);
 }
 
 return 0;
}
