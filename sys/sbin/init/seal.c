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

#define _GNU_SOURCE
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/sendfile.h>

const int seals = F_SEAL_SEAL | F_SEAL_SHRINK | F_SEAL_GROW | F_SEAL_WRITE;

int main(int argc, char **argv, char **envp) {
  char *dir, *file, *path;
  int src, dst;
  ssize_t length;

  if (argc < 2) {
    fprintf(stderr, "Usage: %s PROG [ARG]...\n", argv[0]);
    return 64;
  }

  path = getenv("PATH");
  src = -1;

  if (strchr(argv[1], '/')) {
    if (access(file = argv[1], X_OK) < 0)
      err(EXIT_FAILURE, "%s", file);
    if (src = open(file, O_RDONLY), src < 0)
      err(EXIT_FAILURE, "open %s", file);
  }

  while (src < 0 && path) {
    dir = strsep(&path, ":");
    if (asprintf(&file, "%s%s%s", dir, *dir ? "/" : "", argv[1]) < 0)
      err(EXIT_FAILURE, "malloc");
    if (access(file, X_OK) < 0)
      free(file);
    else if (src = open(file, O_RDONLY), src < 0)
      err(EXIT_FAILURE, "open %s", file);
  }

  if (src < 0) {
    errno = ENOENT;
    err(EXIT_FAILURE, "%s", argv[1]);
  }

  dst = memfd_create(file, MFD_CLOEXEC | MFD_ALLOW_SEALING);
  if (dst < 0)
    err(EXIT_FAILURE, "memfd_create");

  while (length = sendfile(dst, src, NULL, BUFSIZ), length != 0)
    if (length < 0 && errno != EAGAIN && errno != EINTR)
      err(EXIT_FAILURE, "sendfile");
  close(src);
  free(file);

  if (fcntl(dst, F_ADD_SEALS, seals) < 0)
    err(1, "fcntl F_ADD_SEALS");
  fexecve(dst, argv + 1, envp);
  err(1, "fexecve");
}
