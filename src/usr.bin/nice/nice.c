/*
 * SPDX-License-Identifier: NCSA
 * 
 * nice(1) - invoke a utility with an altered nice(2) value
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

static char *argv0;

void bogus (char *what)
{
 fprintf (stderr, "%s: niceness '%s' is bogus\n", argv0, what);
 exit(1);
}

void usage (void)
{
 fprintf (stderr, "%s: usage: %s [-n increment] command\n", argv0, argv0);
 exit(1);
}

int main (int argc, char **argv)
{
 int index;
 int niceness;
 
 argv0=argv[0];
 
 niceness=10;

 /*
  * Do not use getopt(); instead, handle the parsing ourselves.
  * Using getopt() might mess with handling of parameters to commands,
  * especially if it permutes (like GNU's).
  */
 
 index=1;
 if (!strcmp(argv[1], "-n"))
 {
  index=3;
  if (argc<4) usage();
  niceness=atoi(argv[2]);
  if ((!niceness)&&(argv[2][0]!='0')) bogus(argv[2]);
 }
 else if (!strncmp(argv[1], "-n", 2))
 {
  index=2;
  if (argc<3) usage();
  niceness=atoi(&(argv[1][2]));
  if ((!niceness)&&(argv[1][2]!='0')) bogus(&(argv[1][2]));
 }
 
 errno=0;
 if (nice(niceness)==-1)
 {
  if (errno) /* It's OK, POSIX says to do this */
  {
   fprintf (stderr, "%s: failed to set niceness to %d: %s\n", 
            argv0, niceness, strerror(errno));
  }
  return 1;
 }
  
 execvp(argv[index], argv+index);
 perror(argv[index]);
 return 127-(errno!=ENOENT);
}
