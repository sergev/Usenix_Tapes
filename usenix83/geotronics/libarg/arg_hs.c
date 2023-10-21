#include <stdio.h>
#include "argument.h"
#define ARGNLEN 128     /* argument flag and name max length */
/*
 * _arg_hs.c:  _arg_hs.
 * history:     created 21 September    John S. Quarterman.
 *              changed 2 October JS Quarterman to use <non-literals>.
 *              divided into arg_help.c, _arg_hs.c, arg_bhelp.c
 *                      5 Oct. JS Quarterman.
 *              6 December 1981 JSQ if NULL arg and 0 mask, do nothing:
 *                      assume called to echo no arguments.
 *                      Add quotes around shown value.
 *              1 January 1982 JSQ decrease width of help and show lines.
 *              27 March 1982 jsq:  System III compatibility.
 *              1 April 1982 jsq:  let option flag hit left margin.
 *              4 May 1982 jsq:  use AS_printf in "-show".
 */
/**/
/*
 * _arg_hs:  common code for arg_help and arg_show.
 * arg_help prints descriptions but not values. 
 * arg_show prints values but not descriptions.
 * First argument is printing function, and indicates arg_show if present.
 * Second argument is type mask for types of arguments to print.
 * Third argument is a specific argument to print.  If non-NULL,
 * Fourth argument is the type in which to print the argument.
 */
int _arg_hs (func, mask, arg, atype)
int (* func) ();
register int mask;
register ARGUMENT *arg;
int atype;
{
  register int *type;

  if (arg != NULL)
    return (_argahelp (func, arg, atype));

  if (mask == 0)/* called to echo with no arguments */
    return (1);

  printf ("%s:\n", progname);
  for (type = Arg_Control.Arg_Order; *type != 0; type++)
    if (*type & mask)
      _argthelp (func, *type);
  printf ("\n");
  fflush (stdout);
  return (1);
}
/*
 * _argthelp:  does the work for arg_help and arg_show. 
 * First argument is a printing function (arg_fprint) or NULL,
 * Second is the type of argument to deal with.
 * The argument array is taken from Arg_Control.Argument, as always.
 */
static int _argthelp (func, type)
int (* func) ();        /* NULL or arg_fprint */
int type;
{
  extern char *arg_what ();
  char *what;
  register ARGUMENT *arg;

  arg = arg_first (type);
  if (arg == NULL)
    return (1);                 /* none to print */
  if (func == NULL)             /* arg_help */
  {
    printf ("\n");
    what = arg_what (type);
    printf ("%20s%10s  %s\n", what, "format", "description");
  }
  for (; arg != NULL; arg = arg_next (arg, type))
    _argahelp (func, arg, type);
  return (1);
}

/**/
/*
 * _argahelp:  print help or show for a single argument.
 */
static int _argahelp (func, arg, type)
int (* func) ();
register ARGUMENT *arg;
register int type;
{
  char flagstring[ARGNLEN];
  char namestring[ARGNLEN];
  register char *format;

  if (arg == NULL)
  {
    printf ("NULL\n");
    return (0);
  }
  if (type == 0)
    type = arg -> AS_type;

  _arg_bhelp (arg, type, flagstring, namestring);
  if (type & ARG_VARIABLE)
    printf ("  [%8s%-14s]", flagstring, namestring);
  else if (type & ARG_POSSIBLE)
    printf ("  %9s%-14s]", flagstring, namestring);
  else
    printf ("%11s%-15s", flagstring, namestring);

  if (func == NULL)
    format = arg -> AS_scanf;   /* "-help" uses scanf format */
  else
    format = arg -> AS_printf;  /* "-show" uses printf format */
  if (*format == '\0')          /* ARG_DSCAN */
    format = ARG_DPRINT;        /* direct assignment special case */
  if ((func == NULL) && (type & ARG_OPTION) && (arg -> AS_type & ARG_TOGGLE)) 
    format = "";                /* for help, don't show toggle format */
  printf ("%4s  ", format);
  if (func == NULL)             /* arg_help */
    printf ("%s\n", arg -> AS_desc);
  else
  {                             /* arg_show */
    printf ("\"");
    (* func) (stdout, arg);
    printf ("\"");
    printf ("\n");
  }
  return (1);
}
