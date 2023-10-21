#include <stdio.h>
/*
 * errhead.c:  errhead
 * Print an error message header containing the program name
 * and a system error message. 
 * If (eno == 0), no system message is printed,
 * if (eno == -1), eno is taken from errno,
 * and if (eno > 0), eno is used directly.
 * The error messages correspond to those of intro(2).
 * The routine errhead always returns -1.
 */
int errhead (eno)
int eno;
{
  extern char *errmsg ();
  extern char *progname;

  if (progname != NULL)
    fprintf (stderr, "* %s * ", progname);
  if (eno != 0)
    fprintf (stderr, "%s; ", errmsg (eno));
  return (-1);
}

char *progname;
