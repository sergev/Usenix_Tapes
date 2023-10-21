#include <stdio.h>
#include "argument.h"
/*
 * arg_error.c
 * arg_of, _arg_error, arg_error
 * history:     created 21 September    John S. Quarterman.
 * 16 November 1981 JSQ use ARGNEERROR
 */
/**/
/*
 * ARGUMENT *arg; int type; char *message;
 * arg_of (thing, type)     
 *     return the ARGUMENT of the given type whose arg_value is the thing.
 *     arg_of (&local, 0) is the usual usage.
 * arg_error (thing, type, message) 
 *      _arg_error (arg_of (thing, type), type, message); exit (-1);
 *     arg_error (&local, 0, "bad value") is the usual usage.
 * _arg_error (arg, type, message)  
 *     Print an error message for the ARGUMENT as the given type.
 *     The arg defaults to Arg_Control.Arg_Last.
 *     The type defaults to the (first) type of arg.
 *     Argue.c/arg_wrong does the work.
 */
ARGUMENT *arg_of (thing, type)
register char *thing;
register int type;
{
  register ARGUMENT *arg;
  
  for (arg = arg_first (type); 
   (arg != NULL); arg = arg_next (arg, type))
    if (arg -> AS_value == thing)
      return (arg);
  return (NULL);
}

int _arg_error (arg, type, message)
register ARGUMENT *arg;
int type;
char *message;
{
  extern int arg_wrong ();
  char value[128];

  if (arg == NULL)
    arg = Arg_Control.Arg_Last;
  arg_sprint (value, arg);
  return (arg_wrong (arg, type, "", message, value));
}

int arg_error (thing, type, message)
char *thing;
register int type;
char *message;
{
  extern int _arg_error ();

  _arg_error (arg_of (thing, type), type, message);
  if (Arg_Control.Arg_Code & ARGNEERROR)
    return (-1);
  exit (-1);
}
