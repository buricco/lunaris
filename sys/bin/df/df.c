/*
 * SPDX-License-Identifier: NCSA
 * 
 * df(1) - report disk free space
 * (implemented according to IEEE 1003.1-2017)
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/statfs.h>
#include <mntent.h>

#ifndef _PATH_MOUNTED
#define _PATH_MOUNTED "/etc/mtab"
#endif

char *prog;

int h,k,P;

int df (char *what, char *where)
{
 struct statfs sfs;
 
 int e;
 int bsz;
 double multiplier;
 long long s,u,a;
 int p;
 int pint;
 
 e=statfs(what,&sfs);
 s=sfs.f_blocks;
 u=s-sfs.f_bfree;
 a=sfs.f_bavail;
 if (s) p=((s-u)*1000/s); else return 0;
 
 bsz=sfs.f_bsize;

 if (bsz!=512)
 {
  multiplier=(double)bsz/512;
  s*=multiplier;
  u*=multiplier;
  a*=multiplier;
 }
 
 if (k) {s>>=1; u>>=1; a>>=1;}
 
 pint=100-((p+5)/10);
 
 if (e)
  perror(what);
 else
 {
  if (!h)
  {
   h=1;
   if (P) /* Posix me harder */
    printf ("Filesystem %d-blocks Used Available Capacity Mounted on\n",
            k?1024:512);
   else
    printf ("Filesystem            %s         Used    Available   Capacity  "
            "Mounted on\n",k?"1024-blocks":" 512-blocks");
  }
  if (P) /* Posix me harder */
   printf ("%s %llu %llu %llu %d%% %s\n",where,s,u,a,pint,what);
  else
   printf ("%-20s %12llu %12llu %12llu       %3d%%  %s\n",
           where,s,u,a,pint,what);
 }
 
 return e;
}

void usage (void)
{
 fprintf (stderr, "%s: usage: %s [-kP] path ...\n", prog, prog);
 exit(1);
}

int main (int argc, char **argv)
{
 FILE *file;
 struct mntent *m;
 int e,t;
 e=k=h=P=0;
 
 prog=argv[0];
 
 while (-1!=(e=getopt(argc, argv, "kP")))
 {
  switch (e)
  {
   case 'k':
    k=1;
    break;
   case 'P':
    P=1;
    break;
   default:
    usage();
  }
 }
  
 e=0;
 
 if (argc==optind)
 {
  file=setmntent(_PATH_MOUNTED,"r");
  if (!file) {perror(_PATH_MOUNTED); return 1;}
  while (0!=(m=getmntent(file)))
   e+=df(m->mnt_dir,m->mnt_fsname);
  endmntent(file);
 }
 else
 {
  for (t=optind; t<argc; t++)
  {
   struct stat s1, s2;
   file=setmntent(_PATH_MOUNTED,"r");
   if (!file) {perror(_PATH_MOUNTED); return 1;}
   if ((lstat(argv[t],&s1)<0)||(S_ISCHR(s1.st_mode)))
   {
    fprintf (stderr, "%s: %s not block device, directory or mountpoint\n", prog, argv[t]);
    e++;
   }
   else
   {
    while (0!=(m=getmntent(file)))
     if (lstat(m->mnt_fsname,&s2)>=0)
      if (!memcmp(&(s1.st_dev), &(s2.st_rdev), sizeof(s1.st_dev)))
       e+=df(m->mnt_dir,m->mnt_fsname);

    endmntent(file);
   }
  }
 }
 
 return e;
}
