#include <stdio.h>
#include "argument.h"

/*
 * argv_line.c: argv_line
 * Set up to read actual arguments from a file, one per line.
 * Argv_eof, argv_get, and argv_unget in argv_get.c should be used to 
 * handle the actuals.  Argv_unget is done completely in argv_get.c.
 *
 * history:     created 21 September 1981       John S. Quarterman.
 */
int argv_line (file)
FILE *file;
{
  extern int _argv_eof ();
  extern char *_argv_get ();
  extern FILE *argv_fp;

  argv_switch.argv_eof = _argv_eof; 
  argv_switch.argv_get = _argv_get;
  argv_fp = file;
  return (1);
}

/*
 * check for eof.
 */
static int _argv_eof () 
{
  extern FILE *argv_fp;

  if (argv_fp == NULL)
    return (1);
  return (feof (argv_fp));
}

/*
 * _argv_get:  return next actual;  eof has already been checked.
 */
static char *_argv_get ()
{
  extern FILE *argv_fp;
  static char gotten[128];
  register int length;

  gotten[0] = '\0';
  if (NULL == fgets (gotten, sizeof (gotten), argv_fp))
    return (NULL);
  length = strlen (gotten);
  if ((length > 0) && (gotten[--length] == '\n'))
    gotten [length] = '\0';     /* null any newline terminator */
  return (gotten);
}

static FILE *argv_fp = NULL;
