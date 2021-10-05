/*
 * SPDX-License-Identifier: NCSA
 * 
 * uptime(1) - show how long the system has been running
 *             (reimplementation of the procps-ng version)
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
 * A difference from the procps-ng version is how logged-in users are counted.
 * As a result, this version may report more logged-in users than there are.
 * I'm not yet sure how to work around this.  who(1) has the same issue.
 */

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <utmp.h>

#define PROC_UPTIME "/proc/uptime"
#define PROC_LOADAVG "/proc/loadavg"
#define VAR_RUN_UTMP "/var/run/utmp"

enum utmode { UT_DEFAULT, UT_PRETTY, UT_SINCE } utmode;

char *prog;

void mutual(void)
{
 fprintf (stderr, "%s: -p and -s switches are mutually exclusive\n", prog);
 exit(1);
}

void usage(void)
{
 fprintf (stderr, "%s: usage: %s [-p | -s]\n", prog, prog);
 exit(1);
}

void oopsie(void)
{
 fprintf (stderr, "%s: bad /proc/loadavg format\n", prog);
 exit(1);
}

int main (int argc, char **argv)
{
 int e;
 time_t curtime;
 long long upsecs;
 FILE *file;
 char buf[80];
 char *zptr;
 struct tm *tm;
 struct utmp utmp;
 unsigned w, d, dh, h, dhm, m, s, ucnt;
 char load[80], *load1end, *load2end, *load3end;
 int mx;
 
 prog=argv[0];
 utmode=UT_DEFAULT;
 
 while (-1!=(e=getopt(argc, argv, "ps")))
 {
  switch (e)
  {
   case 'p':
    if (utmode!=UT_DEFAULT) mutual();
    utmode=UT_PRETTY;
    break;
   case 's':
    if (utmode!=UT_DEFAULT) mutual();
    utmode=UT_SINCE;
    break;
   default:
    usage();
  }
 }
 
 file=fopen(PROC_UPTIME, "rt");
 if (!file)
 {
  perror(PROC_UPTIME);
  return 1;
 }
 fgets(buf,78,file);
 fclose(file);
 zptr=strchr(buf,'.');
 if (!zptr)
 {
  fprintf(stderr, "%s: Invalid " PROC_UPTIME " format\n", prog);
  return 1;
 }
 *zptr=0;
 upsecs=atoll(buf);
 s=upsecs%60;
 dhm=upsecs/60;
 m=dhm%60;
 dh=dhm/60;
 h=dh%24;
 d=dh/24;
 
 switch (utmode)
 {
  case UT_DEFAULT:
   file=fopen(VAR_RUN_UTMP, "rb");
   if (!file)
   {
    perror(VAR_RUN_UTMP);
    return 1;
   }
   ucnt=0;
   while (1==fread(&utmp, sizeof(struct utmp), 1, file))
   {
    if ((*(utmp.ut_name))&&(*(utmp.ut_line)))
     if ((*(utmp.ut_line)!='~')&&(utmp.ut_pid)) ucnt++;
   }
   fclose(file);
   file=fopen(PROC_LOADAVG, "rt");
   if (!file)
   {
    perror(PROC_LOADAVG);
    return 1;
   }
   fgets(load,78,file);
   fclose(file);
   load1end=strchr(load,' ');
   if (!load1end) oopsie();
   *load1end=0; load1end++;
   load2end=strchr(load1end,' ');
   if (!load2end) oopsie();
   *load2end=0; load2end++;
   load3end=strchr(load2end,' ');
   if (!load3end) oopsie();
   *load3end=0;
   curtime=time(0);
   tm=localtime(&curtime);
   if (!tm)
   {
    perror("localtime(3) failed");
    return 1;
   }
   strftime(buf, 78, "%T", tm);
   printf (" %s up ", buf);
   if (d) printf ("%u days, ", d);
   printf ("%2u:%02u,  %u users,  load average: %s, %s, %s\n", h, m, ucnt,
           load, load1end, load2end);
   break;
  case UT_PRETTY:
   w=d/7;
   d%=7;
   printf ("up ");
   if (upsecs<60) {printf ("%u seconds\n", s); return 0;}
   if (w) {printf ("%u weeks", w); if (d||h||m) printf (", ");}
   if (d) {printf ("%u days", d); if (h||m) printf (", ");}
   if (h) {printf ("%u hours", h); if (m) printf (", ");}
   if (m) printf ("%u minutes\n", m);
   break;
  case UT_SINCE:
   curtime=time(0)-upsecs;
   tm=localtime(&curtime);
   if (!tm)
   {
    perror("localtime(3) failed");
    return 1;
   }
   strftime(buf, 78, "%F %T", tm);
   printf ("%s\n", buf);
   break;
 }
 
 return 0;
}
