#include <stdio.h>
#include "argument.h"
/*
 * arg_synopsis.c:  arg_synopsis, arg_prompt
 */
/*
 * arg_synopsis:  print something like the SYNOPSIS section of man 1.
 * Toggle switches are grouped together in one set of double square brackets,
 * [[-switches]]:  the double brackets are to distinguish them from
 * a single flag toggle [-switches].  Better look strange than be
 * unintelligible;  standard UNIX doesn't really have flags, 
 * so this problem doesn't arise in ordinary manual pages.
 *
 * history:     created 1981    John S. Quarterman
 *              2 October 1981  JS Quarterman:  use _arg_bhelp.
 *              16 November 1981 JS Quarterman:  use ARGNEHELP.
 */
int arg_synopsis ()
{
  extern char *progname;

  printf ("%s", progname);
  arg_tprompt (ARG_OPTION | ARG_ORDINARY);
  printf ("\n");
  fflush (stdout);
  if (Arg_Control.Arg_Code & ARGNEHELP)
    return (1);
  exit (0);
}

/*
 * arg_nprompt:  print a prompt for the remaining ordinary arguments. 
 * To be called by argv_get after being passed through argv_file. 
 * Returns 1, in case it is called directly.
 * This implementation requires 2s complement integers.
 */
int arg_nprompt ()
{
  register int type;

  arg_all ();   /* force initialization of Arg_NType */
  type = Arg_Control.Arg_NType;
  if (_arg_osyn (Arg_Control.Arg_Next, type) > 0)   /* if partial type */
    type <<= 1; /* continue with next type */
  type = (-type) & ARG_ORDINARY;        /* all bits >= type in ARG_ORDINARY */
  return (arg_tprompt (type));
}

/*
 * arg_tprompt:  prompt for every type of argument that matches the mask.
 */
int arg_tprompt (mask)
register int mask;
{
  extern int _arg_tsyn ();
  register int *type;

  for (type = Arg_Control.Arg_Order; *type != 0; type++)
    if (*type & mask)
      _arg_tsyn (*type);
  printf ("\n");
  fflush (stdout);
  return (1);
}

static int _arg_tsyn (type)
register int type;
{
  register ARGUMENT *arg;
  register int count = 0;

  arg = arg_first (type);
  if (arg == NULL)
    return (count);             /* if there aren't any, don't print */ 
  if (type & ARG_KEY)
  {                             /* keys are done differently from all else */
    printf (" ");
    for (; arg != NULL; arg = arg_next (arg, type), count++)
      printf ("%c", *(arg -> AS_flag));
    return (count);
  }
  if (type & ARG_OPTION)
  {             /* print options */
    if (type & ARG_SWITCH)
    {           /* check for stoggle options */
      arg = arg_first (type | ARG_TOGGLE);
      if (NULL != arg)
      {           /* print stoggle switches all together */
	printf (" [[-");
	for (; arg != NULL; arg = arg_next (arg, type | ARG_TOGGLE), count++)
	  printf ("%c", arg -> AS_flag[1]);
	printf ("]]");
      }
      arg = arg_first (type);
    }           /* print all other options */
    for (; arg != NULL; arg = arg_next (arg, type), count++)
      if ((arg -> AS_type & (ARG_SWITCH | ARG_TOGGLE)) 
       != (ARG_SWITCH | ARG_TOGGLE))
	_arg_asyn (arg, type);
    return (count);
  }
/* ordinary arguments */
  return (count + _arg_osyn (arg, type));
}

static int _arg_osyn (arg, type)
register ARGUMENT *arg;
register int type;
{
  register int count = 0;
  register int back; 

  if (arg == NULL)
    return (0);
  if ((type & ARG_VARIABLE) && (arg == arg_first(type)))
    printf (" [");      /* if first variable argument, open bracket */
  for (; arg != NULL; arg = arg_next (arg, type), count++)
    _arg_asyn (arg, type);
  if (type & ARG_VARIABLE)
    printf (" ]");      /* close bracket after last variable argument */
  else
  if (type & ARG_POSSIBLE)
    for (back = 0; back < count; back++)
      printf ("]");                     /* close bracket possibles */
  return (count);
}

/**/
/*
 * _arg_asyn:  print synopsis of a single argument.
 */
static int _arg_asyn (arg, type)
register ARGUMENT *arg;
register int type;
{
  char flagstring[64];
  char namestring[64];

  if (arg == NULL)
  {
    printf (" NULL");
    return (1);
  }
  if (type == 0)
    type = arg -> AS_type;
  _arg_bhelp (arg, type, flagstring, namestring);
  printf ("%s%s", flagstring, namestring);
  return (1);
}
