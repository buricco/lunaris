/*
 * Copyright (C) 2006-2020 Chris Webb.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a 
 * copy of this software and associated documentation files (the "Software"), 
 * to deal in the Software without restriction, including without limitation 
 * the rights to use, copy, modify, merge, publish, distribute, sublicense, 
 * and/or sell copies of the Software, and to permit persons to whom the 
 * Software is furnished to do so, subject to the following conditions:
 * 
 *   * The above copyright notice and this permission notice shall be included 
 *     in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL 
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
 * DEALINGS IN THE SOFTWARE.
 */

#include <err.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mount.h>
#include <sys/syscall.h>

int main(int argc, char **argv) {
  int old, new;

  if (argc < 2) {
    fprintf(stderr, "Usage: %s NEW-ROOT [PUT-OLD]\n", argv[0]);
    return 64;
  }

  if (argc > 2 && strcmp(argv[1], argv[2]) != 0) {
    if (syscall(__NR_pivot_root, argv[1], argv[2]) < 0)
      err(EXIT_FAILURE, "cannot pivot to new root %s", argv[1]);
    return EXIT_SUCCESS;
  }

  if (old = open("/", O_DIRECTORY | O_RDONLY), old < 0)
    err(EXIT_FAILURE, "cannot open /");
  if (new = open(argv[1], O_DIRECTORY | O_RDONLY), new < 0)
    err(EXIT_FAILURE, "cannot open %s", argv[1]);
  if (fchdir(new) < 0 || syscall(__NR_pivot_root, ".", ".") < 0)
    err(EXIT_FAILURE, "cannot pivot to new root %s", argv[1]);

  if (fchdir(old) < 0)
    err(EXIT_FAILURE, "cannot re-enter old root");
  if (mount("", ".", "", MS_SLAVE | MS_REC, NULL) < 0)
    err(EXIT_FAILURE, "cannot disable old root mount propagation");
  if (umount2(".", MNT_DETACH) < 0)
    err(EXIT_FAILURE, "cannot detach old root");
  if (fchdir(new) < 0)
    err(EXIT_FAILURE, "cannot re-enter new root");

  return EXIT_SUCCESS;
}
