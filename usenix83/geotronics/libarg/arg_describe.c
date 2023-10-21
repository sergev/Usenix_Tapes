#include <stdio.h>
#include "argument.h"
/*
 * arg_describe.c:  arg_describe,
 *       arg_replace, arg_bracket, arg_indirect, arg_escape.
 */
/*
 * Arg_describe takes an array of strings containing help
 * information.  The array is terminated with a NULL string pointer.
 * Most strings are printed exactly as found.
 * Any newlines or other formatting characters must be imbedded.
 * There are four strings with special meanings.
 * Arg_replace is replaced with progname.
 * Arg_bracket causes the next string to be surrounded by <>.
 * Arg_indirect dereferences the next array element to get a string.
 * Arg_escape takes the next array element as a char **function,
 * calls it on a pointer to the array element after that,
 * and continues with the result as the string array.
 *
 * created September JS Quarterman
 * added arg_bracket and arg_escape 6 October JSQ
 * added arg_indirect 7 October 1981 JSQ
 */
char arg_replace[] = NULL;
char arg_bracket[] = "<%s>";
char arg_indirect[] = NULL;
char arg_escape[] = NULL;

int arg_describe (array)
register char **array;
{
  register char *what;
  register int indirect = 0;
  int bracket = 0;
  typedef char **(* escfunc) ();
  escfunc func;

  while ((array != NULL) && ((what = *array++) != NULL))
  {
    if (what == arg_escape)
    {                           /* just do the function call now */
      func = *((escfunc *) (array++));  /* get the function */
      array = (* func) (array);         /* use it to replace the array */
      continue;                 /* use the new array */
    }
    if (what == arg_indirect)
    {                           /* another level of indirection */ 
      indirect++;
      continue;
    }
    if (what == arg_bracket)
    {                           /* bracket the next normal string */
      bracket++;
      continue;
    }
    if (what == arg_replace)
      what = progname;          /* replace it and use it */
    else
    while (indirect-- > 0)      
      what = *((char **) what); /* indirect the desired number of levels */
    printf ((bracket ? arg_bracket : "%s"), what);   /* print it */
    bracket = indirect = 0;     /* reset indirection and bracketting */
  }
  fflush (stdout);              /* make sure it gets printed */
  return (1);                   /* success */
}
