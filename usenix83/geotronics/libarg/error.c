#include <stdio.h>
/*
 * ErrName and Error fake the GClib routines.
 */
ErrName (string)
char *string;
{
  extern char *progname;

  progname = string;
}

Error (string)
char *string;
{
  extern int errno;

  errhead (errno);
  fprintf (stderr, "%s\n", string);
  exit (-1);
}
