/*
 * SPDX-License-Identifier: NCSA
 * 
 * Copyright 2007, 2013, 2020 S. V. Nickolas
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal with the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the author nor the names of other contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

/*
 * uname
 *
 * switches:
 *   -s system name (default if nothing supplied)
 *   -n node name
 *   -r release name
 *   -v version
 *   -m machine type (default if invoked as "arch")
 *   -a all of the above
 *
 * POSIX says always use this order, no matter in which order the switches
 * were actually entered.
 *
 * When invoked as "arch", with no parameters, acts like the GNU version of
 * arch, which is equivalent to uname -m.
 *
 * author: usotsuki
 */

#include <sys/utsname.h>
#include <stdio.h>
#include <string.h>
#include "getopt.h"

int main (int argc, char **argv)
{
 extern int optind, optopt;
 extern char *optarg;
 int c;
 int synerr;

 struct utsname utsname;

 int s,n,r,v,m;

 s=n=r=v=m=0;

 synerr=0;

 if (strstr(argv[0], "arch"))
 {
  if (argc!=1)
  {
   fprintf (stderr,"%s: usage: %s\n",argv[0],argv[0]);
   return 1;
  }
  m=1;
 }
 else
  while (-1!=(c=getopt(argc, argv, "snrvma")))
  {
   switch (c)
   {
    case 's':
     s=1;
     break;
    case 'n':
     n=1;
     break;
    case 'r':
     r=1;
     break;
    case 'v':
     v=1;
     break;
    case 'm':
     m=1;
     break;
    case 'a':
     s=n=r=v=m=1;
     break;
    case '?':
     synerr++;
   }
  }
  
 if (synerr)
 {
  fprintf (stderr,"%s: usage: %s [-amnrsv]\n",argv[0],argv[0]);
  return 1;
 }

 if (!(s||n||r||v||m)) s=1;

 if (uname(&utsname))
 {
  perror(argv[0]);
  return 2;
 }

 if (s)
 {
  printf ("%s",utsname.sysname);
  if (n||r||v||m) printf (" ");
 }

 if (n)
 {
  printf ("%s",utsname.nodename);
  if (r||v||m) printf (" ");
 }

 if (r)
 {
  printf ("%s",utsname.release);
  if (v||m) printf (" ");
 }

 if (v)
 {
  printf ("%s",utsname.version);
  if (m) printf (" ");
 }

 if (m)
 {
  printf ("%s",utsname.machine);
 }

 printf ("\n");
 return 0;
}
