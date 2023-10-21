#include <stdio.h>
#include "argument.h"

/* 
 * arg_show.c:  arg_show.
 */
/*
 * arg_show:  use arghs.c/_arg_hs and arg_fprint.c/arg_fprint
 * to show the values of all formal arguments.
 * 20 October 1981 use _arg_hs instead of _arg_hs JS Quarterman.
 */
int arg_show ()
{
  extern int arg_fprint ();

  return (_arg_hs (arg_fprint, ~0, NULL, 0));
}
