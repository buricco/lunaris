/*
 * SPDX-License-Identifier: NCSA
 * 
 * sum(1) - print checksum and block count of a file
 *          (as according to System V Release 2 documentation)
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

#include <stdio.h>

char *prog;

int rsum (FILE *file)
{
 unsigned short c;
 long s;
 unsigned char buf[512];
 
 c=0;
 s=0;
 while (1)
 {
  int e, t;
  
  e=fread(buf,1,512,file);
  if (ferror(file)) return 1;
  if (e<=0)
   break;
  
  s++;
  for (t=0; t<e; t++)
  {
   unsigned short xbit;
   
   xbit=(c&1)<<15;
   c>>=1;
   c|=xbit;
   c+=buf[t];
  }
  if (e<512) break;
 }
 
 printf("%.5u%6lu\n", c, s);
 return 0;
}

int sum (FILE *file, char *dispname)
{
 unsigned long c;
 long s;
 unsigned char buf[512];
 
 c=0;
 s=0;
 while (1)
 {
  int e, t;
  
  e=fread(buf,1,512,file);
  if (ferror(file)) return 1;
  if (e<=0)
   break;
  
  s++;
  for (t=0; t<e; t++) c+=buf[t];
  if (e<512) break;
 }
 
 c=(c&0xFFFF)+(c>>16);
 
 printf ("%u %lu %s\n", c, s, dispname);
 return 0;
}

int main (int argc, char **argv)
{
 int e, t, bsd;
 
 prog=argv[0];
 
 e=0;
 bsd=0;
 
 if (argc>1)
 {
  if (argv[1][0]=='-')
  {
   if (argv[1][1]=='r')
   {
    bsd=1;
    argc--;
    argv++;
   }
  }
 }
 
 if (argc==1)
 {
  if (bsd) rsum(stdin); else sum(stdin,"");
 }
 else for (t=1; t<argc; t++)
 {
  FILE *file;
  
  file=fopen(argv[t],"rb");
  if (!file)
  {
   perror(argv[t]);
   e++;
  }
  else
  {
   int f;
   
   f=bsd?rsum(file):sum(file,argv[t]);
   if (f) perror(argv[t]);
   e+=f;
   fclose(file);
  }
 }
 
 return e;
}
