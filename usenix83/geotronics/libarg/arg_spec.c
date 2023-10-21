#include <stdio.h>
#include "argument.h"
/*
 * arg_spec.c: _argspec (called by the macro argument.h/arg_spec)
 * arg_struct,
 * Arg_Control, arg_what,
 * arg_change, arg_next
 * This file contains all routines which deal directly with lists.
 *
 * history:     created 21 September 1981       John S. Quarterman.
 * converted for lists 23 October 1981 JS Quarterman.
 * 30 March 1982 jsq:  System III compatibility.
 * 5 April 1982 jsq:  AS_stdio.
 * 2 May 1982 jsq:  force non-null AS_desc (System III compatibility).
 */
/**/
/*
 * Arg_Control:  the control structure.
 */
static ARGUMENT dudarg = /* dummy first argument for Arg_Control.Arguments */
{
  NULL,                         /* empty initial list */
  0,                    
  "flag",                       /* these names are used by arg_acheck */
  "name",                       /* in making error messages */
  "sformat",
  "value pointer",
  "pformat",
  "description",
  NULL,
  NULL,
  NULL,
};
/*
 * order:  the order in which to print arguments for Arg_Order.
 * This is used by arg_help, arg_show, arg_synopsis, arg_nprompt, arg_tprompt,
 * and arg_what.
 */
static order [] =
{ 
  ARG_KEY, ARG_SWITCH, ARG_FLAG,        /* options in reverse order */
  ARG_FIXED, ARG_POSSIBLE, ARG_VARIABLE,/* ordinaries in order */
  0     /* end of array marker */
};

struct Arg_Control Arg_Control =        /* the control structure */
{
  &dudarg,      /* the formal argument list in use */
  &order[0],    /* printing order of argument types */
  0,            /* arg_code value */
  0,            /* don't suppress option recognition */
  NULL,         /* the next ordinary actual expected */
  0,            /* the type of the ordinary expected */
  NULL,         /* the last flag actual converted */
  NULL,         /* the next switch or key to be converted */
  NULL,         /* the last actual argument converted */
  NULL,         /* the last formal argument converted */
  0,            /* the type of the last formal converted */
};

/*
 * arg_what:  return a string for an argument type.
 */
char *arg_what (it)             /* return a string for an argument type */
register int it;
{
  static char *arg_types[sizeof (order) / sizeof (order[0])] =
  {                             /* names of the argument types */
    "key",                      /* options */
    "switch option",
    "flag option",
    "fixed argument",           /* ordinary arguments */
    "possible argument",
    "variable argument",
    NULL
  };
  register int *type;

  for (type = Arg_Control.Arg_Order; *type != 0; type++)
    if (*type & it)
      return (arg_types[type - Arg_Control.Arg_Order]); 
  return ("argument");
}
/**/
/*
 * _argspec:  general argument specification routine. 
 */
int _argspec (type, flag, name, sformat, value, pformat, desc, cfunc, cfarg)
int type;
char *flag, *name, *sformat, *value, *pformat, *desc;
int (* cfunc) (); 
char *cfarg;
{
  ARGUMENT atest;
  register ARGUMENT *arg = &atest;

  arg -> AS_type = type;
  arg -> AS_flag = flag;
  arg -> AS_name = name;
  arg -> AS_scanf = sformat;
  arg -> AS_value = value;   
  arg -> AS_printf = pformat;
  arg -> AS_desc = desc;
  arg -> AS_func = cfunc;
  arg -> AS_farg = cfarg;
  return (arg_struct (arg));
}
/**/
/*
 * arg_struct:  specify and check an argument given as an ARGUMENT.
 */
int arg_struct (from)
register ARGUMENT *from;
{
  extern char *malloc ();
  register ARGUMENT *to;
  register ARGUMENT *list;

  if (arg_acheck (from) != 1)           /* valid arg? */
    return (-1);
  if ((to = (ARGUMENT *) malloc (sizeof (*to))) == NULL)
    ent_error (from, 0, "no space", "");/* space for it? */
  *to = *from;                          /* fill arg from the one passed us */
  to -> AS_next = NULL;                 /* make it point nowhere */
  for                                   /* link it into the list */
  (     /* find last list element */
    list = Arg_Control.Arguments        /* point to dummy first element */
    ; list -> AS_next != NULL           /* stop before end of list */
    ; list = list -> AS_next            /* next list element */
  )
    ;
  list -> AS_next = to;                 /* put arg on end of list */
  return (1);
}

/**/
/* 
 * arg_acheck:  check the specification of a single formal argument.
 */
static int arg_acheck (arg)
ARGUMENT *arg;
{
  extern char *arg_stdio ();
  register int type = arg -> AS_type;
  register char *flag = arg -> AS_flag;
  register int length; 

  flag = arg -> AS_flag;
  if (flag == NULL)
  {                             /* ordinary argument, not option */
    type &= ~(ARG_OPTION | ARG_SPECIAL);
    if ((type & ARG_ORDINARY) == 0)
      type |= ARG_FIXED;        /* default to fixed ordinary */
  }
  else
  if (strcmp (flag, ARG_FFIX) == 0)
    type = (type & ~(ARG_OPTION | ARG_SPECIAL)) | ARG_FIXED;
  else
  if (strcmp (flag, ARG_FPOS) == 0)
    type = (type & ~(ARG_OPTION | ARG_SPECIAL)) | ARG_POSSIBLE;
  else
  if (strcmp (flag, ARG_FVAR) == 0)
    type = (type & ~(ARG_OPTION | ARG_SPECIAL)) | ARG_VARIABLE;
  else
  {                             /* option, and maybe ordinary argument, too */
    length = strlen (flag);
    if (argisopt (flag))
    {                           /* it's a switch or flag */
      type &= ~ARG_KEY;         /* but not a key */
      if (length != ARG_SLENGTH)/* if not exactly one char, it's a flag */
	type = (type & ~ARG_SWITCH) | ARG_FLAG;
      if ((type & ARG_OPTION) == 0)
	type |= ARG_SWITCH;     /* default to switch */
    }
    else                        /* it's not a flag or switch:  */
    {                           /* maybe it's a key */
      if (length == ARG_KLENGTH)
	type = (type & ~ARG_OPTION) | ARG_KEY;
      else
	ent_error (arg, ARG_KEY, "key length", flag);
    }
    if (strcmp (flag, (type & ARG_KEY) ? ARG_WKEY : ARG_WFLAG) == 0)
      type |= ARG_WILDCARD;     /* special wild card option */
    if ((arg -> AS_name != NULL) && (*(arg -> AS_name) != '\0'))
      type = (type & ~ARG_TOGGLE) | ARG_VALUE;
    else                        /* no value is expected for the option */
      type = (type & ~(ARG_VALUE | ARG_NOCHECK | ARG_DEFAULT)) | ARG_TOGGLE;
    if (type & (ARG_TOGGLE | ARG_DEFAULT))
      if (strcmp (ARG_TFORMAT, arg -> AS_scanf) != 0)
	ent_error (arg, (type & ARG_OPTION), "scanf format", arg -> AS_scanf);
  }
  arg -> AS_type = type;
/**/
#define pointcheck(element)\
  if (arg -> element == NULL)\
    ent_error (arg, type, Arg_Control.Arguments -> element, NULL)
  if (type & ARG_ORDINARY)
    pointcheck (AS_name);
  pointcheck (AS_scanf);
  pointcheck (AS_value);
  if (arg -> AS_printf == NULL) /* default printf format to scanf format */
    arg -> AS_printf = arg -> AS_scanf;
  pointcheck (AS_desc);
  if ((type & ARG_WILDCARD) && (arg -> AS_func == NULL))
    ent_error (arg, type, "wildcard needs cfunc", NULL);
  arg -> AS_stdio = NULL;
  if (*(arg -> AS_scanf) == ARG_SCHAR)
    if ((flag = arg_stdio (arg, NULL)) != NULL)
      ent_error (arg, type, flag, arg -> AS_scanf);
  return (1);
}
/**/
/*
 * ent_error:  report an error using arg_wrong, then exit.
 * Still used in arg_formal.c, can't be static.
 */
int ent_error (arg, type, message, value)
ARGUMENT *arg; 
int type;
char *message;
char *value;
{
  if (message == NULL)
    message = "type";
  arg_wrong (arg, type, "specification error:  ", message, value);
  exit (-1);
}

/**/
/*
 * arg_change:  reinitialize for using a new ARGUMENT list.
 * Returns the previous ARGUMENT list.
 */
ARGUMENT *arg_change (list)
ARGUMENT *list;                 /* ARGUMENT list */
{
  register ARGUMENT *save = Arg_Control.Arguments -> AS_next;

  Arg_Control.Arguments -> AS_next = list;
  Arg_Control.Arg_Next = NULL;
  Arg_Control.Arg_NType = 0;
  Arg_Control.Arg_Flag = NULL;
  Arg_Control.Arg_Switch = NULL;
  Arg_Control.Arg_Actual = NULL;
  Arg_Control.Arg_Last = NULL;
  Arg_Control.Arg_LType = 0;
  return (save);
}

/**/
/*
 * arg_first (type)
 * Return the first argument of the type.
 * Arg_Control.Arguments is the dummy first ARGUMENT on the list,
 * and it will be skipped by arg_next, so the first ARGUMENT
 * checked will be the first real ARGUMENT.
 */
ARGUMENT *arg_first (type)
int type;
{
  return (arg_next (Arg_Control.Arguments, type));
}

/*
 * arg_next (arg, type)
 * Return the next ARGUMENT that matches the type.
 * Type 0 matches anything.
 */
ARGUMENT *arg_next (arg, type)
register ARGUMENT *arg;
register int type;
{
  if (arg == NULL)      /* end of list */
    return (NULL);
  while ((arg = arg -> AS_next) != NULL) 
    if ((type == 0) || (((arg -> AS_type) & type) == type))
      break;
  return (arg);
}
