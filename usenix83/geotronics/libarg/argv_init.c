#include <stdio.h>
#include "argument.h"

/*
 * argv_init.c: argv_init
 * Set up to read actual arguments from argv.
 * Argv_eof, argv_get, and argv_unget in argv_get.c should be used to 
 * handle the actuals.  Argv_unget is done completely in argv_get.c.
 *
 * history:     created 21 September 1981       John S. Quarterman.
 */
int argv_init (pargc, pargv)
int *pargc;
char ***pargv;
{
  extern int _argv_eof ();
  extern char *_argv_get ();
  extern int *argcp;
  extern char ***argvp;

  argv_switch.argv_eof = _argv_eof; 
  argv_switch.argv_get = _argv_get;
  argcp = pargc;
  argvp = pargv;
  return (1);
}

/*
 * check for eof.
 */
static int _argv_eof () 
{
  extern int *argcp;
  extern char ***argvp;

  if ((argcp == NULL) || (argvp == NULL))
    return (1);
  return (*argcp <= 1);
}

/*
 * _argv_get:  return next actual;  eof has already been checked.
 */
static char *_argv_get ()
{
  extern int *argcp;
  extern char ***argvp;

  --(*argcp);
  return (*++(*argvp));
}

static int *argcp = NULL;
static char ***argvp = NULL;
