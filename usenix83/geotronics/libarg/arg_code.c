#include <stdio.h>
#include "argument.h"
/*
 * arg_code.c:  the real arg_code and arg_decode.
 * There is a dummy arg_decode in arg_decode.c,
 * which gets picked up if arg_code is not called.
 *
 * 6 December 1981 JSQ:  add actual arg echo and change codes. 
 *      Use if instead of switch for multiple codes.
 * 2 May 1982 jsq:  use arg_null for NULL pointer (System III compatibility).
 */
/*
 * arg_code:  just set Arg_Code by the argument and return same.
 */
int arg_code (code)
int code;
{
  return (Arg_Control.Arg_Code = code);
}

/*
 * arg_decode:  do something with the Arg_Code.
 * This routine is called by arg_each on any conversion.
 */
int arg_decode (code)
int code;
{
  extern int arg_fprint ();
  extern int argeactual ();

  if (code & ARGEACTUAL)
  {             /* echo last actual argument */
      _arg_hs (argeactual, 0, Arg_Control.Arg_Last, Arg_Control.Arg_LType);
  }
  if (code & ARGEFORMAL)
  {             /* echo last formal argument */
      _arg_hs (arg_fprint, 0, Arg_Control.Arg_Last, Arg_Control.Arg_LType);
  }
  fflush (stdout);
  return (1);
}

/*
 * argeactual:  print the last actual argument on the indicated file.
 */
static int argeactual (file, arg)
FILE *file;
ARGUMENT *arg;
{
  extern char *arg_null ();

  fprintf (file, "%s", arg_null (Arg_Control.Arg_Actual));
  return (1);
}
