#include <stdio.h>
#include <varargs.h>
/*
 * cmderr.c
 *
 * This file contains the routines cmderr and ecmderr, which provide
 * a decent error-reporting scheme.
 * history:     created 21 September 1981       John S. Quarterman.
 * 11 April 1982 jsq:  split into cmderr.c, ecmderr.c, and _cmderr.c.
 * 9 May 1982 jsq:  System III compatibility (varargs, vcmderr).
 */

/*
 * cmderr (errno, format, args)
 * uses vcmderr to print an error message.
 */
int cmderr (va_alist)
va_dcl
{
  extern int vcmderr ();
  va_list ap;
  register int eno;
  register int ret;

  va_start (ap);
  eno = va_arg (ap, int);
  ret = vcmderr (eno, &ap);
  va_end (ap);
  return (ret);
}
