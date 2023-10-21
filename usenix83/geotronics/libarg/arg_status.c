#include <stdio.h>
#include "argument.h"

/*
 * arg_status.c:  arg_status
 * 27 March 1982 jsq:  System III compatibility.
 * 2 May 1982 jsq:  use arg_null for NULL pointer (System III compatibility).
 */
/*
 * arg_status:  print Arg_Control on stdout.
 */
int arg_status ()
{
  extern char *arg_null ();
  extern char *arg_what ();

  printf ("Arg_Control\n");
  printf ("Arguments:  %07o\n", Arg_Control.Arguments -> AS_next);
  printf ("SOpt:  %d\n", Arg_Control.Arg_SOpt);
  arg_ashow (Arg_Control.Arg_Next, "Next");
  printf ("NType:  %s\n", arg_what (Arg_Control.Arg_NType));
  printf ("Flag:  %s\n", arg_null (Arg_Control.Arg_Flag));
  printf ("Switch:  %s\n", arg_null (Arg_Control.Arg_Switch));
  printf ("Actual:  %s\n", arg_null (Arg_Control.Arg_Actual));
  arg_ashow (Arg_Control.Arg_Last, "Last");
  printf ("LType:  %s\n", arg_what (Arg_Control.Arg_LType));
  printf ("REach:  %d\n", Arg_Control.Arg_REach);
  printf ("RProc:  %d\n", Arg_Control.Arg_RProc);
  printf ("\n");
  fflush (stdout);
  return (1);
} 

static int arg_ashow (arg, first)
register ARGUMENT *arg;
char *first;
{
  extern int _arg_hs ();
  extern int arg_fprint ();

  printf ("%s:  ", first);
  if (arg != NULL)
    _arg_hs (arg_fprint, 0, arg, 0);
  else
    printf ("(none)\n");
  return (1);
}
