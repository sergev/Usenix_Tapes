#include <stdio.h>
#include "argument.h"
/*
 * arg_init.c:  arg_init.
 */
/*
 * arg_init:  do the usual argument package initialization.
 * arg_init (&argc, &argv, help_main)
 * Set progname, use argv for actual arguments.
 * Specify -syn, -help, -show:  -help is arg_man (desc).
 * Define globals syn, help, show, for above options.
 * 27 March 1982 jsq:  use arg_spec, add "--".

 */
static int arg_fsyn, arg_fhelp, arg_fshow = 0;

int arg_init (pargc, pargv, desc)
int *pargc;
char ***pargv;
char **desc;
{
/* specify program name for error reporting */
  progname = *(*pargv);

/* specify actual arguments */
  argv_init (pargc, pargv);

/* specify formal arguments */ 
  arg_spec (0, "-syn", NULL, ARG_TFORMAT, &arg_fsyn 
    , ARG_TFORMAT, "give synopsis", arg_synopsis, NULL);
  arg_spec (0, "-help", NULL, ARG_TFORMAT, &arg_fhelp 
    , ARG_TFORMAT, "give help message", arg_man, desc);
  arg_spec (0, "-show", NULL, ARG_TFORMAT, &arg_fshow 
    , ARG_TFORMAT, "show argument values", arg_show, NULL);
  arg_spec (0, ARG_EOPT, NULL, ARG_TFORMAT, &(Arg_Control.Arg_SOpt)
    , ARG_TFORMAT, "end of options", NULL, NULL);

  return (1);
}

