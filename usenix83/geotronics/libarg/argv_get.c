#include <stdio.h>
#include "argument.h"

/*
 * argv_get.c:  argv_eof, argv_get, argv_unget.
 * actual argument routines:
 * tell if last actual has been read;  read next actual;  unget last actual.
 * Argv_init.c/argv_init or argv_file.c/argv_file should be called to
 * determine where to get the actual arguments (from argv or from a file), 
 * then the routines in this file should be called to handle the actuals.
 * The work of argv_eof and argv_get is done by routines in either
 * argv_init.c or argv_file.c, depending on which of argv_init or argv_file
 * was called last.  Argv_switch tells where to find them.
 *
 * history:     created 21 September 1981       John S. Quarterman.
 * 30 March 1982 jsq:  use Error instead of cmderr.
 */
int argv_eof ()
{
  if (argv_switch.argv_save != NULL) 
    return (0);
  if (argv_switch.argv_eof == NULL)
    return (1);
  return ((* (argv_switch.argv_eof)) ());
}

char *argv_get ()
{
  register char *saved;

  if (argv_switch.argv_save != NULL)
  {
    saved = argv_switch.argv_save;
    argv_switch.argv_save = NULL;
    return (saved);
  }
  if (argv_eof ())
    return (NULL);
  return ((* (argv_switch.argv_get)) ());
}

char *argv_unget (value)
register char *value;
{
  if (argv_switch.argv_save != NULL)
  {
    Error ("double call on argv_unget");
    return (NULL);
  }
  argv_switch.argv_save = value;
  return (value);
}

struct argv_switch argv_switch =
{
  NULL,
  NULL,
  NULL,
};
