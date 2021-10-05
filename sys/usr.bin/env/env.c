/*
 * SPDX-License-Identifier: NCSA
 * 
 * env(1) - set the environment for command invocation
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

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

extern char **environ;

int cpysetenv (char *t)
{
 char *p,*q;
 int e;
 
 if (!strchr(t,'=')) return -1;
 p=malloc(strlen(t)+1);
 if (!p)
 {
  fprintf (stderr, "Out of memory\n");
  return -1;
 }
 strcpy(p,t);
 q=strchr(p,'=');
 *(q++)=0;
 e=setenv(p,q,1);
 free(p);
 if (e) perror("setenv()");
 return e;
}

static void usage (char *pname)
{
 fprintf (stderr, "%s: usage: %s [-i] [tag=content ...] [command args ...]\n",
          pname, pname);
 exit (1);
}

int main (int argc, char **argv)
{
 int e,i;
 
 i=0;
 while (-1!=(e=getopt(argc, argv, "i")))
 {
  switch (e)
  {
   case 'i':
    i=1;
    break;
   default:
    usage(argv[0]);
  }
 }
 
 if (i)
 {
  if (clearenv())
  {
   perror("clearenv()");
   return 1;
  }
 }
 
 for (e=optind; e<argc; e++)
 {
  if (!strchr(argv[e],'=')) break;
  if (cpysetenv(argv[e]))
   return 1;
 }
 
 if (e<argc)
 {
  i=execvp(argv[e], argv+e);
  perror(argv[e]);
  if (errno==ENOENT) return 127;
  return 126;
 }
 
 for (i=0; environ[i]; i++) printf ("%s\n", environ[i]);
 return 0;
}
