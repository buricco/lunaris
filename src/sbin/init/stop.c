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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <sys/reboot.h>
#include <unistd.h>

int main(int argc, char **argv) {
  if (argc == 2 && !strcmp(argv[1], "halt"))
    return reboot(RB_HALT_SYSTEM);
  else if (argc == 2 && !strcmp(argv[1], "kexec"))
    return reboot(RB_KEXEC);
  else if (argc == 2 && !strcmp(argv[1], "poweroff"))
    return fork() > 0 ? pause() : reboot(RB_POWER_OFF);
  else if (argc == 2 && !strcmp(argv[1], "reboot"))
    return reboot(RB_AUTOBOOT);
  else if (argc == 2 && !strcmp(argv[1], "suspend"))
    return reboot(RB_SW_SUSPEND);

  fprintf(stderr, "\
Usage: %s ACTION\n\
Actions:\n\
  halt      halt the machine\n\
  kexec     jump to a new kernel loaded for kexec\n\
  poweroff  switch off the machine\n\
  reboot    restart the machine\n\
  suspend   hibernate the machine to disk\n\
All actions are performed immediately without flushing buffers or a\n\
graceful shutdown. Data may be lost on unsynced mounted filesystems.\n\
", argv[0]);
  return EX_USAGE;
}
