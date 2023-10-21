#include <stdio.h>
/*
 * 9 May 1982 jsq:  Version of vcmderr using %r instead of varargs.
 */

int vcmderr (eno, args)
int eno;
char ***args;
{
  errhead (eno);
  fprintf (stderr, "%r", *printargs);
  return (-1);
}

int _cmderr (name, eno, printargs)
char *name;
int eno;
char **printargs;
{
  extern char *progname;
  register char *psave;
  register int ret;

  psave = progname;
  progname = name;
  ret = vcmderr (eno, &printargs);
  progname = psave;
  return (ret);
}
