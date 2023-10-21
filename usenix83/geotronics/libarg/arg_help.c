#include <stdio.h>
#include "argument.h"

/*
 * arg_help.c:  arg_help.
 * 5 October JS Quarterman.
 * 20 October use new _arg_hs JS Quarterman.
 * 16 November remove _arg_help JS Quarterman.
 */
int arg_help ()
{
  register int ret;

  ret = _arg_hs (NULL, ~0, NULL, 0);
  if (Arg_Control.Arg_Code & ARGNEHELP)
    return (ret);
  exit (0);
}

