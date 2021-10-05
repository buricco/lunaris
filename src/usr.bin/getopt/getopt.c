/*
 * Written by S. V. Nickolas.
 * Copyright disclaimed where possible; CC0 elsewhere.
 * While this code is roughly based on Henry Spencer's version, he dedicated
 * that one into the public domain.  I made a few tweaks to how it worked.
 * Some of the code is based on the SysV manual.
 */

#include <getopt.h>
#include <stdio.h>

int main (int argc, char **argv)
{
 int e;
 int r;
 char *argv0;
 char *optsel;
 int s;
 
 argv0=argv[0];
 
 if (argc==1)
 {
  fprintf (stderr, "%s: usage: %s optlist $*\n", argv0, argv0);
  return 1;
 }
 
 optsel=argv[1];
 argv+=2;
 argc-=2;
 r=s=0;
 
 while (-1!=(e=getopt(argc, argv, optsel)))
 {
  switch (e)
  {
   case '?':
    r++;
    break;
   default:
    if (s)
     printf (" ");
    else
     s++;

    if (optarg)
     printf ("-%c %s", e, optarg);
    else
     printf ("-%c", e);
  }
 }
 
 printf ("%s", s?" --":"--");
 
 for (e=optind; e<argc; e++) printf (" %s", argv[e]);
 printf ("\n");
 return r;
}
