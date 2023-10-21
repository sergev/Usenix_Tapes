/*
 * errmsg.c
 * errmsg:  return an error message string for an error number.
 * Error number -1 is replaced with errno.
 * This version just looks at the C library error message array.
 */
char *errmsg (eno)
register int eno;
{
  extern char *sys_errlist[];
  extern int sys_nerr;
  extern int errno;

  if (eno == -1)
    eno = errno;
  if ((eno < 0) || (eno >= sys_nerr))
    return ("unknown error");
  return (sys_errlist[eno]);
}
