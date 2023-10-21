#include <stdio.h>
#include "argument.h"
/*
 * arg_process.c
 * arg_process, arg_each, arg_all, arg_conv, argisopt.
 *      fixed "-" no error message bug  6 November 1981 JS Quarterman
 *      Arg_REach, Arg_RProc values 3 December 1981 JSQ
 *      23 March 1982 JSQ  Support System III stuff:
 *              -l<value> and -l <value>  are equivalent
 *              --      means no more options 
 *              -       is an ordinary argument, (can't be an option)
 *      5 April 1982 jsq:  add fopen codes for conversion.
 */
/**/
/*
 * arg_process: 
 *
 * If there are no formal variable arguments, arg_process will process
 * all actuals on the first call, returning 2.  A call with no
 * actuals to process will return 0.
 *
 * If there are formal variable arguments, arg_process will process 
 * the fixed arguments (and leading options) on the first call,
 * and a set of variable arguments (and leading options) on each
 * call thereafter, until there are no more ordinary actuals.  Each call
 * will return 1, until there are no more ordinary actuals,
 * then a call processing trailing flags (if any) will return 2,
 * then all succeeding calls will return 0.
 *
 * Returns:  
 *      -1      error
 *      0       no processing done:  nothing to do
 *      1       a set of arguments completely processed
 *      2       last actual was converted
 *
 * Individual formal arguments may be processed using arg_each.
 */
int arg_process ()                    /* process command arguments */
{
  register int ret;

  if (argv_eof () && arg_all ())
    return (Arg_Control.Arg_RProc = 0);
  while ((ret = arg_each ()) == 1)
    ;
  if (ret == 2)                 /* an ordinary argument was done */
    ret = 1;
  else
  if (ret == 0)
    ret = 2;
  else
    ret = -1;
  return (Arg_Control.Arg_RProc = ret);
}

/*
 * arg_each: take the next given user argument and convert it. 
 * Converts both regular arguments and options.
 * Converts only one formal argument per call.
 * returns 1:   got an argument.
 *      2:      the last argument of a set was done.
 *      0:      no more arguments to process.
 *      -1:     too many arguments given, or conversion error. 
 * The last argument cracked may be found in Arg_Control.Arg_Last,
 * and its conversion type is in Arg_Control.Arg_LType.
 * If there is a function to call on conversion, call it,
 * and pass it its argument, plus processing code.
 * (If it wants to know what the argument is, it can look in Arg_Control.)
 * If error from func, return -1 instead of processing code.
 * Saving the returned value in Arg_REach allows recursion 
 * from within the AS_func routine.
 */
int arg_each ()                 /* crack an argument */
{
  register int ret;
  register ARGUMENT *arg;

  ret = _arg_option ();         /* try as an option */
  if (ret == 0)
    ret = _arg_okey ();         /* the first ordinary may be a key */
  if (Arg_Control.Arg_SOpt < 0) /* If true System III compatibility, */
    Arg_Control.Arg_SOpt = 1;   /* first ordinary turns off options. */
  if (ret == 0)
    ret = _arg_ordinary ();     /* try as ordinary */
  Arg_Control.Arg_REach = ret;
  if (Arg_Control.Arg_Code != 0)
    arg_decode (Arg_Control.Arg_Code);
  ret = Arg_Control.Arg_REach;
  if (ret <= 0)                 /* if error or EOF, */
    return (ret);               /* just return it */
  arg = Arg_Control.Arg_Last;   /* get arg just converted */
  if (arg -> AS_func != NULL)   /* and call func if necessary */
    if (-1 == ((* (arg -> AS_func)) (arg -> AS_farg, ret)))
      return (-1);              /* unless error from func */
  return (Arg_Control.Arg_REach);         /* return processing code */
}
/**/
/*
 * _arg_option:  crack an option
 * returns 1:   got one, and it was not the last option.
 *      0:      argument given is not an option.
 *      -1:     too many arguments given, or conversion error. 
 *
 * Both flag and switch arguments are done, 
 * and either type may also be a toggle.
 * Only one formal argument is done per call, 
 * though there may be many switches in an actual argument.
 */
static int _arg_option ()
{
  register char *flag;
  register int ret;
  
  if (Arg_Control.Arg_Switch != NULL)
    return (_arg_oswitch ());   /* switch or key argument in progress */
  if (Arg_Control.Arg_SOpt > 0)
    return (0);                 /* options are not to be recognized */
  flag = argv_get ();           /* get the current argument */
  if ((flag == NULL) || (!argisopt (flag)))
  {     /* if no more actuals or not an option */
    argv_unget (flag);          /* put it back */
    return (0);                 /* say it was not an option */
  }
  Arg_Control.Arg_Actual = flag;
  Arg_Control.Arg_Flag = flag;
  ret = _arg_oget (flag, ARG_FLAG);      /* try as a flag */
  if (ret != 0)
    return (ret);
  return (_arg_oswitch ());     /* try as a switch */
}

/**/
/*
 * _arg_okey:  set up to process a key;  let _arg_oswitch do the work.
 */
static int _arg_okey () 
{
  static int first = 1; /* do it only once:  speed is not important */
  register char *key;

  if (!first)
    return (0);
  first = 0;
  if (Arg_Control.Arg_SOpt > 0)
    return (0);                 /* options are not to be recognized */
  key = argv_get ();            /* get the current actual */
  if ((key == NULL) || (*key == '\0') || (NULL == arg_first (ARG_KEY)))
  {     /* if no actuals or no formal keys */
    argv_unget (key);
    return (0);
  }
  Arg_Control.Arg_Actual = key;
  Arg_Control.Arg_Flag = key;   /* save for later use */
  Arg_Control.Arg_Switch = key; /* say we are processing something */
  return (_arg_oswitch ());     /* do the first key */
}                               /* _arg_option will do the rest */

/*
 * _arg_oswitch:  do switch processing for _arg_option. 
 * Returns:  1 (got a switch); -1 (error); 0 (not a switch)
 * In fact, 0 is never returned, and arg_option depends on it.
 */
static int _arg_oswitch ()
{
  register char *option;        /* scratch option flag variable */
  register char *aswitch;       /* switch is a reserved word in C */
  register int type; 
  int ret;

  option = Arg_Control.Arg_Flag;  /* _arg_option put it here */
  if (!argisopt (option))
    type = ARG_KEY;
  else
    type = ARG_SWITCH;
  if (Arg_Control.Arg_Switch == NULL)
  {                             /* first switch call, so initialize */
    aswitch = option;
    if (*++aswitch == '\0')     /* skip '-' to get first switch char */
      --aswitch;                /* would match "-" to "--" switch */
  }
  else
    aswitch = Arg_Control.Arg_Switch;
  if (type == ARG_KEY)
  {
    option = "?";
    option[0] = *aswitch;        /* use the character directly as a key */
  }
  else
  {
    option = "-?";
    option[1] = *aswitch;        /* option a switch from the character */
  }
  if (*++aswitch == '\0')               /* this was the last one */
    aswitch = NULL;
  Arg_Control.Arg_Switch = aswitch;     /* set up for the next one */
  ret = _arg_oget (option, type);
  if (ret != 0)
    return (ret);
  return (_argsyntax (NULL, type
    , ((type == ARG_SWITCH) ? "unknown switch" : "unknown key")
    , option));
}

/**/
/*
 * _arg_oget:  do the stuff common to flag, switch and key options.
 * given a flag to test, find an option that matches, find its
 * conversion type, and call arg_conv to convert it.
 * Returns:  1 (found a match); 0 (found no match); -1 (error).
 */
static int _arg_oget (flag, type)
register char *flag;
register int type;
{
  register ARGUMENT *opt;

  for                           /* check for an option */
  (
    opt = arg_first (type); 
    opt != NULL; 
    opt = arg_next (opt, type)
  )
    if (strcmp (opt -> AS_flag, flag) == 0)
      break;                    /* found a match */
  if (opt == NULL)
    return (0);               /* no match */
	/* given option type plus special types from ARGUMENT */
  type = (type & ARG_OPTION) | (opt -> AS_type & ARG_SPECIAL);
  return (arg_conv (opt, type));
}


/**/
/*
 * _arg_ordinary:  crack an ordinary argument. 
 * This routine should not be called until _arg_option returns zero.
 * returns 1:   got an argument.
 *      2:      the last formal of a set was done.
 *      0:      the actual was an option, or there are no more.
 *      -1:     too many actuals given, or conversion error. 
 */
static int _arg_ordinary ()
{
  extern ARGUMENT *_arg_aset ();
  register char *value;
  register int type;
  register ARGUMENT *next;

  value = argv_get ();
  Arg_Control.Arg_Actual = value;
  _arg_aset (0);
  type = Arg_Control.Arg_NType; /* this has just been updated */
  if (value == NULL)            /* no more arguments */
  {                             
    if (arg_all ())             /* we are at the end of a set of arguments */
      return (0);
    return (_argsyntax (NULL, type, "too few arguments", value));
  }
  _arg_aset (1);
  next = Arg_Control.Arg_Next;
  if (next == NULL)   /* no more formals */
    return (_argsyntax (NULL, type, "too many arguments", value));
  if (-1 == arg_conv (next, type))      /* convert it */
    return (-1);
   /* set up the next expected */
  Arg_Control.Arg_Next = next = arg_next (next, type); 
  if (next == NULL)
  {                             /* last formal was done of this set */
    if 
    (
      (type == ARG_VARIABLE)    /* just did vars */
      ||
      (
	(type == ARG_FIXED)     /* just did fixeds */
	&&                      /* and there are vars */
	(arg_first (ARG_VARIABLE) != NULL)
      ) /* if no vars, want all actuals converted, not just fixeds */
    )
    return (2);                 /* end of a set of arguments */
  }
  return (1);                   /* just another argument */
}

/**/
/*
 * _arg_aset: set up for _arg_ordinary. 
 * returns 1 for Arg_Control.Arg_NType set to non-NULL, else 0.
 * The argument set, if one, causes first ARG_VARIABLE arg to be set up,
 * if necessary.
 * Empty formal ordinary sets are ignored by recursion.
 */
static ARGUMENT *_arg_aset (set)
register int set;
{
  if (Arg_Control.Arg_Next != NULL)
    return (Arg_Control.Arg_Next);
  switch (Arg_Control.Arg_NType)
  {
    default:    /* can never occur */
      Error ("arg_process state");
      exit (-1);
      break;
    case 0:     /* initial state */
      Arg_Control.Arg_NType = ARG_FIXED;
      break;
    case ARG_FIXED:
      Arg_Control.Arg_NType = ARG_POSSIBLE;
      break;
    case ARG_POSSIBLE:
      Arg_Control.Arg_NType = ARG_VARIABLE;
      /* fall through to ARG_VARIABLE */
    case ARG_VARIABLE:
      if (!set)
	return (NULL);
      break;
  }
  Arg_Control.Arg_Next = arg_first (Arg_Control.Arg_NType);
  if (Arg_Control.Arg_Next != NULL)
    return (Arg_Control.Arg_Next);      /* first of a set */
  if (Arg_Control.Arg_NType == ARG_VARIABLE)
    return (NULL);              /* last set to check:  end of recursion */
  return (_arg_aset (set));     /* recurse */
}
/* 
 * arg_all:  if we have reached a point in the formal arguments
 * where we can stop if there are no more actual arguments,
 * return 1, else 0.
 */
int arg_all ()
{
  extern ARGUMENT *_arg_aset ();

  _arg_aset (0);                /* make sure argument stuff is set up */
  if (Arg_Control.Arg_Switch != NULL)   /* switch or key being processed */
    return (0);
  if (Arg_Control.Arg_Next == NULL)      /* end of a set of arguments */
    return (1);
  if (Arg_Control.Arg_NType == ARG_POSSIBLE)
    return (1);                 /* can stop any time in possibles */
  return (0);
}
/**/
/*
 * arg_conv:  given an ARGUMENT and a conversion type,
 * find the value to convert, and convert it.
 * Returns 1 for success and -1 for error.
 */
int arg_conv (arg, type)
register ARGUMENT *arg;
register int type;
{
  extern char *arg_stdio ();
  register char *value;
  char *format;
  FILE *file;

  Arg_Control.Arg_Last = arg;   
  Arg_Control.Arg_LType = type;                 
  if (type & ARG_ORDINARY)
    value = Arg_Control.Arg_Actual;     /* ordinaries use one actual */
  else
  if (type & ARG_TOGGLE)
    value = NULL;               /* just set toggle options */
  else
  if ((value = Arg_Control.Arg_Switch) != NULL)
  {                             /* value is in same arg with switch */
    Arg_Control.Arg_Switch = NULL;  /* don't try value as switch */
  }
  else  
  if ((value = argv_get ()) != NULL)
  {                             /* value is in next argument */
    if (!(type & ARG_NOCHECK) && argisopt (value))
    {
      argv_unget (value);
      value = NULL;
    }
  }
  Arg_Control.Arg_Actual = value;       /* we have a value now */
  if (value == NULL)                    /* it's NULL */
  {                                     /* maybe default value */
    if (!(type & (ARG_DEFAULT | ARG_TOGGLE)))
      return (_argsyntax (arg, type, "no value", value));
    * ((ARG_TTYPE *) (arg -> AS_value)) = ARG_TON;
    return (1);
  }                             /* convert value */
  format = arg -> AS_scanf;
  if (*format == '\0')          /* ARG_DSCAN */
  {                             /* direct assignment */
    * ((ARG_DTYPE *) (arg -> AS_value)) = (ARG_DTYPE) value;
    return (1);
  }                             /* use scanf */
  if (*format == ARG_SCHAR)
  {                             /* stdio type */
    if ((format = arg_stdio (arg, value)) != NULL)
      return (_argsyntax (arg, type, format, value));
    return (1);
  }
  if (1 != sscanf (value, format, arg -> AS_value)) 
    return (_argsyntax (arg, type, "conversion failure", value));
  return (1);
}

/**/
/*
 * argisopt:  given a string, see if it is an option flag.
 * This is used by arg_spec, and so can't be static.
 */
int argisopt (value) 
register char *value;
{
  register int this;

  if (value == NULL) 
    return (0);
  if ((*value != '-') && (*value != '+') && (*value != '='))
    return (0);
  this = value[1];
  if (this == '\0')
    return (0);                 /* "-" is never an option */
  if (
   ((this >= '0') && (this <= '9'))
   ||
   (this == '.')
  )
    return (0);
  return (1);
}

/**/
/*
 * _argsyntax:  call arg_wrong to report a syntax error. 
 * If NULL arg and wildcard present, call wild card function first,
 * and then arg_wrong only if wild card function returns 0.
 * _argsyntax never returns 0, and usually returns -1.
 */
static int _argsyntax (arg, type, message, value)
register ARGUMENT *arg;
register int type;
char *message;
char *value;
{
  register ARGUMENT *wild;
  int ret = 0;

  Arg_Control.Arg_LType = type;
  if ((arg == NULL) && ((wild = arg_first (type | ARG_WILDCARD)) != NULL))
  {     /* if no match found, and wild card specified, call function */
    Arg_Control.Arg_Last = wild;
    ret = ((* (wild -> AS_func)) (wild -> AS_farg, -1, value));
  }
  if (ret != 0)
    return (ret);       /* action was taken by AS_func */
  Arg_Control.Arg_Next = arg;
  return (arg_wrong (arg, type, "syntax error:  ", message, value));
}
