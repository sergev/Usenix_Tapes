#include <stdio.h>
#include "argument.h"
/*
 * arg_formal.c
 * arg_ordinary, arg_option, arg_formal: 
 * obsolete formal argument specification routines.
 */
int arg_ordinary (type, name, format, value, desc, func, farg)
register int type;
register char *name, *format, *value, *desc;
int (* func) ();
char *farg;
{
  if (!(type & ARG_ORDINARY) || (type & (ARG_OPTION | ARG_SPECIAL)))
    ent_error (NULL, type, NULL, name);
  return (arg_formal (type, NULL, name, format, value
    , desc, func, farg));
}
int arg_option (type, flag, name, format, value, desc, func, farg)
register int type;
register char *flag, *name, *format, *value, *desc;
int (* func) ();
char *farg;
{
  if (!(type & ARG_OPTION) || (type & ARG_ORDINARY))
    ent_error (NULL, type, NULL, flag);
  return (arg_formal (type, flag, name, format, value
    , desc, func, farg));
}
/*
 * arg_formal:  specify and check an argument given as attributes.
 */
int arg_formal (type, flag, name, format, value, desc, func, farg)
int type;
char *flag, *name, *format, *value, *desc;
int (* func) (); 
char *farg;
{
  if ((type & ARG_OPTION) && ((type & (ARG_VALUE | ARG_TOGGLE)) == 0))
    type |= ARG_VALUE;
  if ((flag != NULL) && (*flag == '\0'))
    flag = NULL;                /* null string means something to arg_spec */
  if ((type & (ARG_FUNCTION | ARG_WILDCARD)) == 0)
  {
    func = NULL;
    farg = NULL;
  }
  return (arg_spec (type, flag, name, format, value
    , format, desc, func, farg));
}
