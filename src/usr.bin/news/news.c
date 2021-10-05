/*
 * SPDX-License-Identifier: NCSA
 * 
 * news(1) - print news items
 *           (as according to System V Release 4 documentation)
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

#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <limits.h>
#include <pwd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <utime.h>

#define VAR_NEWS "/var/news"

char *argv0;
time_t lastctrlbrk;
int cut;

extern const char *user_from_uid(uid_t, int); /* libunatix */

struct stat statbuf;

void timedbreak (int ignored)
{
 if ((time(0)-lastctrlbrk)<2) exit(0);
 cut=1;
 lastctrlbrk=time(0);
}

void do_entry (char *filename)
{
 char buf[1024];
 char datestr[80];
 FILE *file;
 char *ptr;

 cut=0;

 ptr=strrchr(filename,'/');
 if (ptr) ptr++; else ptr=filename;

 file=fopen(filename,"rt");
 if (!file)
 {
  if (errno==ENOENT)
   fprintf (stderr, "%s: Article '%s' does not exist\n", argv0, ptr);
  else
   fprintf (stderr, "%s: %s: fopen(): %s\n", argv0, filename, strerror(errno));
  return;
 }

 if (fstat(fileno(file), &statbuf))
 {
  fprintf (stderr, "%s: %s: fstat(): %s\n", argv0, filename, strerror(errno));
  fclose(file);
  return;
 }

 strftime(datestr, 79, "%a %b %d %T %Y", localtime(&(statbuf.st_mtim.tv_sec)));
 printf ("\n%s (%s): %s\n", ptr, user_from_uid(statbuf.st_uid,0), datestr);
 while (1)
 {
  fgets(buf, 1023, file);
  if (cut||feof(file)) break;

  printf ("   %s", buf);
 }

 fclose(file);
}

void reap (void)
{
 fprintf (stderr, "%s: out of memory\n", argv0);
 exit(1);
}

void usage(void)
{
 fprintf (stderr, "%s: usage: %s [-ans | article ...]\n", argv0, argv0);
 exit(0);
}

int main (int argc, char **argv)
{
 struct sigaction sa;
 char buf[PATH_MAX], newstime[PATH_MAX];
 char *home;
 time_t epoch;
 int e;
 int a, n, s;
 int entries;
 long pilespace;
 int *pileseq;
 char *pile;
 char **pilearr;
 char *pileptr;
 time_t *stamps;
 FILE *file;
 DIR *dir;
 struct dirent *dirent;

 argv0=argv[0];
 lastctrlbrk=0;

 sa.sa_handler=timedbreak;
 sigemptyset(&(sa.sa_mask));
 sa.sa_flags=0;

 sigaction(SIGINT, &sa, 0);

 home=getenv("HOME");
 if (!home)
 {
  fprintf (stderr, "%s: warning: $HOME not set, using current directory\n",
           argv[0]);
  sprintf(newstime, ".news_time");
 }
 else sprintf (newstime, "%s/.news_time", home);

 if (stat(newstime, &statbuf))
  epoch=0;
 else
  epoch=statbuf.st_mtim.tv_sec;

 a=n=s=0;

 while (-1!=(e=getopt(argc, argv, "ans")))
 {
  switch (e)
  {
   case 'a':
    a=1;
    break;
   case 'n':
    n=1;
    break;
   case 's':
    s=1;
    break;
   default:
    usage();
  }
 }

 if ((a+n+s)>1)
 {
  fprintf (stderr,
           "%s: -a, -n, -s switches are mutually exclusive\n", argv[0]);
  return 1;
 }

 if ((a+n+s)&&(optind<argc))
 {
  fprintf (stderr, "%s: switches and article names are mutually exclusive\n",
           argv[0]);
  return 1;
 }

 if (optind<argc)
 {
  for (e=optind; e<argc; e++)
  {
   sprintf (buf, VAR_NEWS "/%s", argv[e]);
   do_entry(buf);
  }
  printf ("\n");
  return 0;
 }

 if (a) epoch=0;

 dir=opendir(VAR_NEWS);
 if (!dir)
 {
  fprintf (stderr, "%s: " VAR_NEWS ": %s\n", argv[0], strerror(errno));
  return 1;
 }

 /*
  * First run: count entries and see how much storage we need for the list.
  * If -s, just show this number and quit.
  */
 entries=0;
 pilespace=0;
 while (1)
 {
  dirent=readdir(dir);
  if (!dirent) break;

  if (*(dirent->d_name)=='.') continue;

  if (!strcmp(dirent->d_name, "core")) continue;

  if (a)
   entries++;
  else
  {
   sprintf (buf, VAR_NEWS "/%s", dirent->d_name);
   pilespace+=strlen(dirent->d_name)+1;
   if (stat (buf, &statbuf))
   {
    fprintf (stderr, "%s: %s: %s (1)\n", argv[0], buf, strerror(errno));
    closedir(dir);
    return 1;
   }
   if ((statbuf.st_mtim.tv_sec)>epoch) entries++;
  }
 }
 closedir(dir);
 if (s)
 {
  if (!entries)
   printf ("No news items.\n");
  else if (entries==1)
   printf ("1 news item.\n");
  else
   printf ("%d news items.\n", entries);

  return 0;
 }

 /*
  * Now prepare to create a sorted file list.
  */
 pileseq=calloc(entries, sizeof(int));
 if (!pileseq)
 {
  reap();
 }

 stamps=calloc(entries, sizeof(time_t));
 if (!stamps)
 {
  free(pileseq);
  reap();
 }

 pilearr=calloc(entries, sizeof(char*));
 if (!pilearr)
 {
  free(stamps);
  free(pileseq);
  reap();
 }

 pile=calloc(pilespace, sizeof(char));
 if (!pile)
 {
  free(pilearr);
  free(pileseq);
  reap();
 }

 for (e=0; e<entries; e++) pileseq[e]=e;
 dir=opendir(VAR_NEWS);
 if (!dir)
 {
  fprintf (stderr, "%s: " VAR_NEWS ": %s\n", argv[0], strerror(errno));
  return 1;
 }

 pileptr=pile;

 e=0;

 while (1)
 {
  dirent=readdir(dir);
  if (!dirent) break;
  if (*(dirent->d_name)=='.') continue;
  if (!strcmp(dirent->d_name, "core")) continue;
  sprintf (buf, VAR_NEWS "/%s", dirent->d_name);
  if (stat (buf, &statbuf))
  {
   fprintf (stderr, "%s: %s: %s (2)\n", argv[0], buf, strerror(errno));
   closedir(dir);
   return 1;
  }
  if ((a)||((statbuf.st_mtim.tv_sec)>epoch))
  {
   pilearr[e]=pileptr;
   strcpy(pileptr, dirent->d_name);
   stamps[e]=statbuf.st_mtim.tv_sec;
   e++;
   pileptr+=strlen(dirent->d_name)+1;
  }
 }
 closedir(dir);

 /*
  * Iterate through the list
  */
 e=1;
 while (e)
 {
  int i, j, t;

  e=0;
  for (i=0; i<entries; i++)
  {
   for (j=i+1; j<entries; j++)
   {
    if (stamps[pileseq[i]]<stamps[pileseq[j]])
    {
     e=1;

     t=pileseq[i];
     pileseq[i]=pileseq[j];
     pileseq[j]=t;
    }
   }
  }
 }

 if (n)
 {
  printf ("news:");
  for (e=0; e<entries; e++) printf (" %s", pilearr[pileseq[e]]);
  printf ("\n");
 }
 else
 {
  for (e=0; e<entries; e++)
  {
   sprintf (buf, VAR_NEWS "/%s", pilearr[pileseq[e]]);
   do_entry(buf);
  }
  if (entries) printf ("\n");

  /*
   * Update stamp of ~/.news_time, if appropriate.
   */
  if (!a)
  {
   if (utime(newstime, 0))
    if (errno==ENOENT)
    {
     creat(newstime,0644);
    }
  }
 }

 free(pile);
 free(stamps);
 free(pilearr);
 free(pileseq);
 return 0;
}
