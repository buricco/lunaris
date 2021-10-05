/*
 * Copyright 2020 S. V. Nickolas.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a 
 * copy of this software and associated documentation files (the "Software"), 
 * to deal with the Software without restriction, including without limitation 
 * the rights to use, copy, modify, merge, publish, distribute, sublicense, 
 * and/or sell copies of the Software, and to permit persons to whom the 
 * Software is furnished to do so, subject to the following conditions:
 * 
 *   1. Redistributions of source code must retain the above copyright 
 *      notices, this list of conditions and the following disclaimer.
 *   2. Redistributions in binary form must reproduce the above copyright 
 *      notices, this list of conditions and the following disclaimer in the 
 *      documentation and/or other materials provided with the distribution.
 *   3. Neither the names of the copyright holders, authors, or contributors 
 *      may be used to endorse or promote products derived from this software 
 *      without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED "AS IS", AND ANY EXPRESS OR IMPLIED WARRANTIES 
 * ARE DISCLAIMED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF 
 * MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL THE 
 * COPYRIGHT HOLDERS, AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY OR CONSEQUENTIAL DAMAGES 
 * (INCLUDING BUT NOT LIMITED TO PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND 
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF 
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <linux/vt.h>

#define DEV_CONSOLE "/dev/console"

char *argv0;

void die (char *x)
{
 fprintf (stderr, "%s: %s: %s\n", argv0, x, strerror(errno));
 exit(1);
}

int main (int argc, char **argv)
{
 int e, x, h;
 
 argv0=argv[0];
 
 if (argc!=2)
 {
  fprintf (stderr, "%s: usage: %s vt\n", argv0, argv0);
  return 1;
 }
 
 h=open(DEV_CONSOLE, O_RDWR);
 if (h==-1) die(DEV_CONSOLE);
 x=atoi(argv[1]);
 e=ioctl(h, VT_ACTIVATE, x) || ioctl(h, VT_WAITACTIVE, x);
 if (e) die(DEV_CONSOLE);
 return 0;
}
