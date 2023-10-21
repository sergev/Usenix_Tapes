#include <stdio.h>
#include "argument.h"
/*
 * arg_fprint.c:  arg_fprint, arg_sprint.
 */
/*
 * arg_fprint, arg_sprint:  produce a string for an argument value and print 
 * it on the file or just put it in a char array.
 * 29 March 1982 jsq:  remove "%[" handling.
 * 30 March 1982 jsq:  remove use of "%r".
 * 5 April 1982 jsq:  fopen types.
 * 2 May 1982 jsq:  handle %s of NULL (System III printf doesn't).
 */
arg_fprint (file, arg)
FILE *file;
ARGUMENT *arg;
{
  arg_doprint (file, NULL, arg);
}

/*
 * arg_sprint:  print an arg into ((where != NULL) ? where : file),
 * with file defaulting to stdout.
 */
arg_sprint (where, arg)
char *where;
ARGUMENT *arg;
{
  arg_doprint (NULL, where, arg);
}

static arg_doprint (file, whence, arg)
FILE *file;                     /* file for output */
char *whence;                   /* string for output */
ARGUMENT *arg;                  /* argument to output */
{
  extern char *arg_null ();
  union any
  {
    double a_double;
    float a_float;
    long a_long;
    int a_int;
    short a_short;
    char a_char;
    char *a_pchar;
    ARG_DTYPE a_dtype;
  };
  union any anything;
  char temporary[512];
  register char *where;
  register char *format;
  register char *value;
  int last;

  if (arg == NULL)
  {
    format = "%s";
    value = "(NULL ARGUMENT)";
  }
  format = arg -> AS_printf;
  value = arg -> AS_value;
  if (*format == '\0')          /* ARG_DSCAN */
  {                             /* special direct string assignment format */
    format = ARG_DPRINT;
    anything.a_dtype = * ((ARG_DTYPE *) value);
    last = 's';
  }
  else
  if (*format == ARG_SCHAR)
  {                             /* fopen types */
    format = ARG_DPRINT;
    anything.a_pchar = arg -> AS_stdio;
    last = 's';
  }
  else
  {                             /* %s and %c */
    last = format[strlen (format) - 1];
    if (last == 's')            /* printf and scanf both want (char *) */
      anything.a_pchar = (char *) value;
    else if (last == 'c')       /* printf wants (int), scanf (char *) */
      anything.a_int = * (char *) value; 
    else                        /* unknown type:  move it all */
      anything = * ((union any *) value);
  }
  if (last == 's')              /* System III printf doesn't handle this */
    anything.a_pchar = arg_null (anything.a_pchar);
  if ((where = whence) == NULL)
    where = &temporary[0];      /* arg_fprint */
  sprintf (where, format, anything);
  if (whence != NULL)
    return;                     /* arg_sprint */
  if (file == NULL)             /* arg_fprint */
    file = stdout;
  fprintf (file, "%s", where);
  return;                       /* arg_fprint */
}
