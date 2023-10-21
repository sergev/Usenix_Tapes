#include <stdio.h>
/*
 * ecmderr (errno, format, printargs)
 * uses _cmderr to print an error message, and then exits.
 * The exit argument is taken from errno.
 * 11 April 1982 jsq:  split into cmderr.c, ecmderr.c, and _cmderr.c.
 */
int ecmderr (eno, pfargs)
int eno;
char *pfargs;
{
  extern char *progname;
  _cmderr (progname, eno, &pfargs);
  if (eno == 0)
    exit (-1);
  else
    exit (eno);
}

