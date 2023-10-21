#include <stdio.h>
#include "argument.h"
/*
 * arg_wrong.c:  arg_wrong.
 * 30 March 1982 jsq:  don't use cmderr.
 * 2 May 1982 jsq:  use arg_null for NULL pointer (System III compatibility).
 */
/*
 * arg_wrong:  uniform error reporting routine.
 */
int arg_wrong (arg, type, kind, message, value)
register ARGUMENT *arg;
register int type;
char *kind;
char *message;
char *value;
{
  extern char *arg_null ();
  extern char *arg_what (); 
  extern int errno;
  register int twoline = ((errno != 0) && (arg != NULL));

  errhead (errno);              /* program name and sys error message */
  fprintf (stderr, "%s%s%s", kind, message, (twoline ? "\n" : ""));
  if (arg != NULL)
  {
    if (twoline)
      errhead (0);              /* don't need sys error again */
    if (type == 0)
      type = arg -> AS_type; 
    fprintf (stderr, " %s %s", arg_what (type), arg_null (arg -> AS_flag));
    if (arg -> AS_name != NULL)
      fprintf (stderr, " <%s>", arg -> AS_name);
  }
  fprintf (stderr, " \"%s\"\n", arg_null (value));
  return (-1);
}

