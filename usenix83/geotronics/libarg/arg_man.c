#include <stdio.h>
#include "argument.h"

/*
 * arg_man.c:  arg_man.
 */
/*
 * arg_man:  use arg_synopsis, arg_help, and arg_describe to print
 * a manual-like summary of the command.
 * 16 November 1981 JS Quarterman:  use noexit code.
 */
int arg_man (description)
char **description;
{
  register int code;

  code = Arg_Control.Arg_Code & ARGNEHELP; /* save it */
  Arg_Control.Arg_Code |= ARGNEHELP;       /* turn it on */
  arg_synopsis ();                      /* thus turning exits off */
  arg_help ();
  arg_describe (description);
  if (code)                             /* if it was on, */
    return (1);                         /* leave it on and return */
  Arg_Control.Arg_Code &= ~ARGNEHELP;      /* turn it off */
  exit (0);                             /* exit */
}

