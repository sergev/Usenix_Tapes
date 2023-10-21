#include <stdio.h>
#include "argument.h"
/*
 * _arg_bhelp.c:  _arg_bhelp.
 * Common code for arg_help, arg_show, arg_synopsis.
 * 5 October 1981 JS Quarterman.
 * 23 March 1982 remove ARG_SAME JSQ.
 */
/*
 * _arg_bhelp sprintfs strings for the flag and name of an arg.
 * This is used by both _arg_ahelp and _arg_asynopsis.
 */
_arg_bhelp (arg, type, flagstring, namestring)
register ARGUMENT *arg;
int type;
char *flagstring;
char *namestring;
{
  register char *flagformat = "%s";
  register char *nameformat = "%s";

  if (type & ARG_OPTION)
  {
    flagformat = " [%s";
    switch ((arg -> AS_type) & ARG_SPECIAL)
    {
      case (ARG_TOGGLE):
	nameformat = "]";
	break;
      case (ARG_DEFAULT):
	nameformat = " [<%s>]]";
	break;
      case 0:
      default:
	nameformat = " <%s>]";
	break;
    }
  }
  else
  {
    flagformat = "";
    switch (type & ARG_ORDINARY) 
    {
      case ARG_VARIABLE:
	nameformat = " <%s>";
	break;
      case ARG_POSSIBLE:
	nameformat = " [<%s>";   /* there's no good way to do this one */
	break;
      case ARG_FIXED:
      default:
	nameformat = " <%s>";
	break;
    }
  }
  sprintf (flagstring, flagformat, arg -> AS_flag);
  sprintf (namestring, nameformat, arg -> AS_name);
}
