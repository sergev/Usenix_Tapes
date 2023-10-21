#include <stdio.h>
/*
 * cmderr.c
 *
 * This file contains the routines cmderr and ecmderr, which provide
 * a decent error-reporting scheme.
 * history:     created 21 September 1981       John S. Quarterman.
 * 11 April 1982 jsq:  split into cmderr.c, ecmderr.c, and _cmderr.c.
 */

/*
 * cmderr (errno, format, printargs)
 * uses _cmderr to print an error message.
 */
int cmderr (eno, pfargs)
int eno;
char *pfargs;
{
  extern char *progname;
  return (_cmderr (progname, eno, &pfargs));
}
