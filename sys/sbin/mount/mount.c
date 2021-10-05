/*
 * SPDX-License-Identifier: NCSA
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

#include <getopt.h>
#include <limits.h>
#include <mntent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mount.h>

#define PROC_MOUNTS "/proc/mounts"
#define ETC_FSTAB   "/etc/fstab"

char *prog;

void list (void)
{
 FILE *file;
 int c, state;
 
 file=fopen(PROC_MOUNTS, "rt");
 if (!file) 
 {
  perror(PROC_MOUNTS); 
  exit(1);
 }
 state=0;
 while (1)
 {
  c=fgetc(file);
  if (c<0) 
  {
   fclose(file);
   exit(0);
  }
  if (c=='\n') {state=0; printf ("\n"); continue;}
  if (c==' ')
  {
   state++;
   switch (state)
   {
    case 1: 
     printf (" on ");
     break;
    case 2:
     printf (" type ");
     break;
    case 3:
     printf (" (");
     break;
    case 4:
     printf (")");
   }
   continue;
  }
  else
   if (state<4) printf ("%c", c);
 }
}

void dotheneedful (int simulate)
{
 FILE *file;
 struct mntent *mntent;
 
 file=setmntent(ETC_FSTAB,"rt");
 if (!file)
 {
  perror(ETC_FSTAB);
  exit(1);
 }
 
 while (1)
 {
  int e;
  
  mntent=getmntent(file);
  if (!mntent) break;
  
  if (simulate) continue;
  
  e=mount(mntent->mnt_fsname,
          mntent->mnt_dir,
          mntent->mnt_type,
          0,
          mntent->mnt_opts);
  
  if (e)
  {
   fprintf(stderr, "warning: ");
   perror(mntent->mnt_fsname);
  }
 }
 endmntent(file);
 exit(0);
}

void horpusdorpus (void)
{
 fprintf (stderr, "%s: usage: %s [-BfMr] [-o option] [-t type] device path\n"
                  "%s: usage: %s -a\n", prog, prog, prog, prog);
 exit(1);
}

int main (int argc, char **argv)
{
 int e;
 int aflag, Bflag, fflag, Mflag, rflag;
 char *opts, *type;
 unsigned long flags;
 
 prog=argv[0];
 
 aflag=Bflag=fflag=Mflag=rflag=0;
 opts=type=0;
 flags=0;
 
 if (argc==1) list();
 while (-1!=(e=getopt(argc, argv, "aBfMo:rt:")))
 {
  switch (e)
  {
   case 'a':
    aflag=1;
    break;
   case 'B':
    Bflag=1;
    break;
   case 'f':
    fflag=1;
    break;
   case 'M':
    Mflag=1;
    break;
   case 'o':
    opts=optarg;
    break;
   case 'r':
    rflag=1;
    break;
   case 't':
    type=optarg;
    break;
   default:
    horpusdorpus();
  }
 }
 
 if (aflag) if (argc==optind) dotheneedful(fflag);

 if ((argc-optind)!=2) horpusdorpus();
 
 flags |= (Bflag?MS_BIND:0)|
          (Mflag?MS_MOVE:0)|
          (rflag?MS_RDONLY:0);
          
 if (fflag) return 0;
 e=mount(argv[optind], argv[optind+1], type?type:"", flags, opts?opts:"");
 if (e)
  perror(argv[optind]);
}
