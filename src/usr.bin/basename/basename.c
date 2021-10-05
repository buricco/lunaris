/*
 * SPDX-License-Identifier: NCSA
 * 
 * basename(1) - return non-directory portion of a pathname
 * (implemented according to IEEE 1003.1-2017)
 *
 * Copyright 2019 Steve Nickolas
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void usage (char *pname)
{
 fprintf (stderr, "usage: %s string [suffix]\n", pname);
}

int main (int argc, char **argv)
{
 char *buf, *ptr;
 int t;
 int allslashes;
 
 if ((argc<2)||(argc>3))
 {
  usage(argv[0]);
  return 1;
 }
 
 /* 1. string is null: output is null */
 if (!*argv[1])
 {
  printf ("\n");
  return 0;
 }
 
 buf=malloc(strlen(argv[1])+1);
 if (!buf)
 {
  fprintf (stderr, "%s: insufficient memory\n");
  return 2;
 }
 strcpy(buf, argv[1]);
 
 /* 2. string is all path separators: output is a single slash */
 allslashes=1;
 for (t=0; t<strlen(buf); t++) 
  if (buf[t]!='/') {allslashes=0; break;}
 
 if (allslashes)
 {
  free(buf);
  printf ("/");
  return 0;
 }

 /* 3. remove all trailing path separators */
 while (buf[strlen(buf)-1]=='/')
  buf[strlen(buf)-1]=0;

 /* 4. discard everything before the final remaining path separator */
 ptr=buf;
 for (t=0; t<strlen(buf); t++)
  if ((buf[t]=='/')) ptr=buf+1;

 /* 5. if suffix is present, and is the end of the filename, strip it */
 if (argc==3)
 {
  if (strlen(argv[2])<strlen(ptr))
  {
   if (!strcmp(&(ptr[strlen(ptr)-strlen(argv[2])]),argv[2]))
    ptr[strlen(ptr)-strlen(argv[2])]=0;
  }
 }
 
 printf ("%s\n", ptr);
 free(buf);
 return 0;
}
