/*
 * SPDX-License-Identifier: NCSA
 * 
 * dirname(1) - return directory portion of a pathname
 * (implemented according to IEEE 1003.1-2017)
 *
 * Copyright 2020 Steve Nickolas
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main (int argc, char **argv)
{
 char *tank;
 
 if (argc!=2)
 {
  fprintf (stderr, "%s: usage: %s path\n", argv[0], argv[0]);
  return -1;
 }
 
 tank=malloc(strlen(argv[1]+1));
 if (!tank)
 {
  fprintf (stderr, "%s: out of memory\n", argv[0]);
  return -1;
 }
 strcpy(tank,argv[1]);
 
 /* "1. If string is //, skip steps 2 to 5." */
 if (strcmp(tank,"//"))
 {
  /* "2. If string consists entirely of <slash> characters, string shall be
   *     set to a single <slash> character. In this case, skip steps 3 to 8."
   */
  int t,sl;
  sl=1;
  for (t=0;t<strlen(tank);t++) if (tank[t]!='/') sl=0;
  if (sl)
  {
   printf ("/\n");
   free(tank);
   return 0;
  }

  /* "3. If there are any trailing <slash> characters in string, they shall be
   *     removed."
   */
  while (tank[strlen(tank)-1]=='/') tank[strlen(tank)-1]=0;

  /* "4. If there are no <slash> characters remaining in string, string shall
   *     be set to a single <period> character. In this case, skip steps 5 to
   *     8."
   */
  if (!strchr(tank,'/'))
  {
   printf (".\n");
   free(tank);
   return 0;
  }
  
  /* "5. If there are any trailing non-<slash> characters in string, they
   *     shall be removed."
   * 
   * There WILL be at least one slash in the string by this point.
   * See also step 7.
   */
  
  *(strrchr(tank,'/'))=0;
 }
 
 /* "6. If the remaining string is //, it is implementation-defined whether
  *     steps 7 and 8 are skipped or processed."
  * 
  * "7. If there are any trailing <slash> characters in string, they shall be
  *     removed."
  */
 while (tank[strlen(tank)-1]=='/') tank[strlen(tank)-1]=0;

 /* "8. If the remaining string is empty, string shall be set to a single
  *     slash character."
  */
 if (!*tank)
 {
  printf ("/\n");
  free(tank);
  return 0;
 }
 
 /* "The resulting string shall be written to standard output."
  * 
  * See above, where three exceptions are manually written to stdout.
  */
 printf ("%s\n", tank);
 free(tank);
 return 0;
}
