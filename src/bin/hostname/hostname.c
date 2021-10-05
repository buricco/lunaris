/*
 * SPDX-License-Identifier: NCSA
 * 
 * hostname(1) - set or print name of current host system
 * domainname(1) - set or print YP domain of current host system
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

#include <sys/param.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static void usage_domainname (char *pname)
{
 fprintf (stderr, "%s: usage: %s [name]\n", pname, pname);
 exit (1);
}

static void usage_hostname (char *pname)
{
 fprintf (stderr, "%s: usage: %s [-s | name]\n", pname, pname);
 exit (1);
}

int main (int argc, char **argv)
{
 int d,e,s;
 char h[HOST_NAME_MAX+1];
 
 d=(strstr(argv[0],"domainname"))?1:0;
 
 s=0;
 
 if (d)
 {
  if (argc>2) usage_domainname(argv[0]);
  
  if (argc==1)
  {
   if (getdomainname(h,HOST_NAME_MAX))
   {
    perror ("getdomainname()");
    return 1;
   }
   printf ("%s\n", h);
   return 0;
  }
  
  if (setdomainname(argv[1],strlen(argv[1])))
  {
   perror(argv[1]);
   return 1;
  }
 }
 else
 {
  while (-1!=(e=getopt(argc, argv, "s")))
  {
   switch (e)
   {
    case 's':
     s=1;
     break;
    default:
     usage_hostname(argv[0]);
   }
  }
 
  if (s && (optind<argc))
  {
   fprintf (stderr, "%s: -s and name are mutually exclusive\n", argv[0]);
   return 1;
  }
 
  if (argc-optind>1) usage_hostname(argv[0]);
 
  if (argc==optind)
  {
   if (gethostname(h,HOST_NAME_MAX))
   {
    perror ("gethostname()");
    return 1;
   }
  
   if (s)
   {
    char *p;
   
    p=strchr(h,'.');
    if (p) *p=0;
   }
  
   printf ("%s\n", h);
   return 0;
  }
  else
  {
   if (sethostname(argv[optind],strlen(argv[optind])))
   {
    perror(argv[optind]);
    return 1;
   }
  }
 }
 return 0;
}
