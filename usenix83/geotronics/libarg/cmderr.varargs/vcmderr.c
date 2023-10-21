#include <stdio.h>
#include <varargs.h>

/*
 * vcmderr (errno, args)
 * An error header is printed on stderr using errhead.
 * This is followed by the equivalent of fprintf (stderr, format, pargs).
 * Varargs.h is V7, but was once accidentally distributed on a V6 tape....
 *
 * 9 May 1982 jsq:  System III compatibility;  created from _cmderr.
 */
/* #define COMPATIBLE      1       /* This produces a _cmderr kludge. */

int vcmderr (eno, args)
int eno;
va_list *args;
{
  extern int _print();
  extern FILE *_pfile;
  char *format;

  errhead (eno);                /* the error header */
  format = va_arg (*args, char *);
  _pfile = stderr;
  _print (format, args);        /* printf general routine */
  return (-1);
}

#ifdef COMPATIBLE
/*
 * _cmderr is the old equivalent of _vcmderr, and is obsolete.
 * This version is a kludge that will only work on backwards-stack
 * machines like the PDP-11.  It should go away completely shortly.
 */
int _cmderr (name, eno, pargs)
char *name;
int eno;
char **pargs;
{
  extern char *progname;
  extern int vcmderr ();
  va_list args = (va_list) pargs;       /* kludge! */
  register char *psave;

  psave = progname;
  progname = name;
  vcmderr (eno, &args);
  progname = psave;
  return (-1);
}
#endif COMPATIBLE
