/*
 * SPDX-License-Identifier: NCSA
 * 
 * echo - write arguments to standard output
 * (implemented according to IEEE 1003.1-2017, BSD historical precedent and 
 *  System V Release 2 documentation; pick your poison)
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

/*
 * For "XSI" compliance, set ECHO_BSD to 0 and ECHO_SYSV to 1.
 */

/* Set to 1 to enable BSD behavior (-n switch) */
#define ECHO_BSD 1

/* Set to 1 to enable System V/XSI behavior (backslash escapes) */
#define ECHO_SYSV 1

#include <stdio.h>

int main (int argc, char **argv)
{
 int nflag;
 int t;
 int started;
 
 t=1;
 nflag=started=0;
#if ECHO_BSD
 if (argc>1)
 {
  if (argv[1][0]=='-')
   if (argv[1][1]=='n')
   {
    nflag=1;
    t++;
   }
 }
#endif
 
 for (; t<argc; t++)
 {
  int c,s,o;
  
  if (started) fputc(' ',stdout); else started=1;
  
  s=0;
  o=0;
  for (c=0; argv[t][c]; c++)
  {
#if ECHO_SYSV
   if ((argv[t][c]=='\\')&&(!s))
   {
    s=1;
    continue;
   }
   else if (s==1)
   {
    switch (argv[t][c])
    {
     case 'a':
      o=7; s=2;
      break;
     case 'b':
      o=8; s=2;
      break;
     case 'c':
      nflag=1; s=3;
      break;
     case 'f':
      o=12; s=2;
      break;
     case 'n':
      o='\n'; s=2;
      break;
     case 't':
      o='\t'; s=2;
      break;
     case '\\':
      o='\\'; s=2;
      break;
     case '0':
      o=0; s=4;
      break;
    }
    if (s==2)
    {
     fputc (o,stdout);
     s=0;
     continue;
    }
   }
   if (s==4)
   {
    if ((argv[t][c]>='0')&(argv[t][c]<'8'))
    {
     o<<=3;
     o|=(argv[t][c]-'0');
    }
    else
    {
     s=0;
     fputc (o, stdout);
    }
   }
   if ((s)&&(s!=4))
   {
    s=0;
    continue;
   }
   if (!s)
   {
    fputc (argv[t][c], stdout);
   }
  }
  if (s==4) fputc(o, stdout);
 }
#else
   fputc(argv[t][c], stdout);
  }
 }
#endif
 
 if (!nflag) fputc('\n', stdout);
 return 0;
}
