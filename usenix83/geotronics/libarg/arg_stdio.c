#include <stdio.h>
#include "argument.h"
/*
 * arg_stdio.c:  arg_stdio.
 * Handle fopen and freopen format specification and conversion.
 * Called from arg_spec.c/arg_acheck and arg_process.c/arg_conv.
 * Arg_fprint.c/arg_doprint handles printing stdio file names alone.
 * Use of fopen/freopen can be deleted by using, for instance:
 *      char *arg_stdio () { return ("no stdio formats"); }
 */
char *arg_stdio (arg, name)
ARGUMENT *arg;
register char *name;
{
  static char sin[] = "`standard input'";
  static char sout[] = "`standard output'";
  static char serr[] = "`standard error'";
  register FILE *file = * ((FILE **) (arg -> AS_value));
  register char *format = arg -> AS_scanf;
  char *oldname = arg -> AS_stdio;
  int reopen = 0;

  if (*format != ARG_SCHAR)     /* first & means stdio format */
    return ("not stdio format");
  if (*++format == ARG_SCHAR)   /* second & means freopen, not fopen */
  {
    reopen++;
    format++;                   /* rest after the &s is the real stdio type */
  }
  if (name == NULL)
  {                             /* formal argument specification */
    if (reopen)
    {
      if (file == NULL)
	return ("need (FILE *)");
      else
      if (file == stdin)
	name = sin;
      else
      if (file == stdout)
	name = sout;
      else
      if (file == stderr)
	name = serr;
    }
  }
  else
  {                             /* argument conversion */
    if (strcmp (name, ARG_STDIO) == 0)
    {                           /* "-" standard io argument */
      if (reopen)
      {                         /* just leave it as it was */
	if (oldname != NULL)
	  name = oldname;
      }
      else
      {                         /* substitute stdin or stdout */
	if (*format == 'r')
	{
	  file = stdin;
	  name = sin;
	}
	else
	{
	  file = stdout;
	  name = sout;
	}
      }
    }
    else                        /* some ordinary file name */
    {
      if (reopen)               /* usually stdin or stdout */
	file = freopen (name, format, file);
      else                      /* no file open already */
	file = fopen (name, format);
      if (file == NULL)         /* open failed */
	return ("can't open");
    }
  }
  * ((FILE **) (arg -> AS_value)) = file;
  if (name != NULL)
  {
    extern char *malloc ();

    arg -> AS_stdio = (char *) malloc (strlen (name) + 1);
    if (arg -> AS_stdio != NULL)
      strcpy (arg -> AS_stdio, name);
    if (oldname != NULL)
      free (oldname);
  }
  return (NULL);                /* success */
}
