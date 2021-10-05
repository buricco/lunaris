/*
 * SPDX-License-Identifier: NCSA
 * 
 * cat(1) - concatenate and print files
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

/*
 * As originally implemented, this cat(1) copied the System V Release 2
 * version.  With this version, I have replaced the System V "-s" (suppress
 * errors) switch with the BSD "-s" (squeeze newlines), and since I make
 * heavy use of my code on non-Unixlike operating systems, I added a new "-o"
 * switch to handle redirection internally instead of leaving it to the shell.
 * 
 * Additionally although I did not implement the line numbering switches -b
 * and -n (we have nl(1) for that) I implemented the switches which were taken
 * from BSD into System V Release 4 (-etv), with their documented behavior 
 * from that version.
 */

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

FILE *out;
int eflag, sflag, tflag, uflag, vflag;

static void usage (char *pname)
{
 fprintf (stderr, "%s: usage: %s [-su] [filename...]\n", pname, pname);
}

int cat (char *filename)
{
 FILE *in;
 int is_stdin;
 int ret;
 int last;
 
 ret=0;
 is_stdin=0;
 last=-1;
 if (!strcmp(filename,"-"))
 {
  is_stdin=1;
  in=stdin;
  rewind(in);
 }
 else
 {
  in=fopen(filename,"r");
 }
 
 if (!in)
 {
  perror(filename);
  return 1;
 }
 
 if (uflag)
 {
  setvbuf(in,0,_IONBF,0);
  setvbuf(out,0,_IONBF,0);
 }
 
 while (-1)
 {
  int c,m;
  
  c=fgetc(in);
  if (c<0)
  {
   if (ferror(in)) ret++;
   break;
  }
  if ((c=='\n')&&(last=='\n')&&sflag) continue;
  last=c;
  if (tflag)
  {
   if (c==9) {fprintf(out, "^I"); continue;}
   if (c==12) {fprintf(out, "^L"); continue;}
  }
  if (vflag)
  {
   m=0;
   if (c&0x80)
   {
    fputc('M',out);
    fputc('-',out);
    c&=0x7F;
    m=1;
   }
   if (c==0x7F) {fprintf(out,"^?"); continue;}
   if ((c<0x20)&&(c!='\n'))
   {
    if (((c!=9)&&(c!=12))||(m)) {fputc('^',out); c|='@';}
   }
  }
  if (eflag&&(c=='\n')) fputc('$',out);
  fputc(c,out);
 }
 if (!is_stdin) fclose(in);
 
 return ret;
}

int main (int argc, char **argv)
{
 int e, t;
 
 out=stdout;
 eflag=sflag=tflag=uflag=vflag=0;
 while (-1!=(e=getopt(argc, argv, "eo:stuv")))
 {
  switch (e)
  {
   case 'o':
    out=fopen(optarg, "wb");
    if (!out)
    {
     perror(optarg);
     return -1;
    }
    break;
   case 'e':
    eflag=1;
    break;
   case 's':
    sflag=1;
    break;
   case 't':
    tflag=1;
    break;
   case 'u':
    uflag=1;
    break;
   case 'v':
    vflag=1;
    break;
   default:
    usage(argv[0]);
    return 1;
  }
 }
 
 if (!vflag)
  eflag=tflag=0;
 
 e=0;
 
 if (argc==optind) 
  e=cat("-");
 else
  for (t=optind; t<argc; t++) e+=cat(argv[t]);
 
 if (out!=stdout) fclose(out);
 
 return e;
}
