#include <stdio.h>
#include <varargs.h>
/*
 * ecmderr.c
 *
 * This file contains the routines cmderr and ecmderr, which provide
 * a decent error-reporting scheme.
 * history:     created 21 September 1981       John S. Quarterman.
 * 11 April 1982 jsq:  split into cmderr.c, ecmderr.c, and _cmderr.c.
 * 9 May 1982 jsq:  System III compatibility (varargs, vcmderr).
 */

/*
 * ecmderr (errno, format, args)
 * Uses vcmderr to print an error message, then exits.
 * The exit argument is taken from errno.
 */
int ecmderr (va_alist)
va_dcl
{
  extern int vcmderr ();
  register int eno;
  va_list ap;

  va_start (ap);
  eno = va_arg (ap, int);
  vcmderr (eno, &ap);
  va_end (ap);
  if (eno == 0)
    exit (-1);
  else
    exit (eno);
}

