/*
 * SPDX-License-Identifier: NCSA
 * 
 * killall5(1) - terminate or signal processes
 * (implemented according to Solaris 10 manual page)
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
 * Cliff 'em all!
 */

#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main (int argc, char **argv)
{
 int e;
 
 if (geteuid())
 {
  fprintf (stderr, "%s: mollyguard: must be run by superuser\n", argv[0]);
  return 1;
 }
 
 if (argc==1)
 {
  e=kill(-1, SIGTERM);
  
  if (e==EPERM) fprintf (stderr, "%s: access denied\n");
  return e;
 }
 
 if (argc!=2)
 {
  fprintf (stderr, "%s: usage: %s [signal]\n", argv[0], argv[0]);
  return 2;
 }
 
 if (!isdigit(*argv[1]))
 {
  fprintf (stderr, "%s: mollyguard: only numeric signals taken\n", argv[0]);
  return 3;
 }
 
 e=kill(-1, atoi(argv[1]));
 if (e==EPERM) fprintf (stderr, "%s: access denied\n");
 return e;
}
