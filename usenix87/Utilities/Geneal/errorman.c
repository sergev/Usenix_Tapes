/* errorman.c - general error handling routines */
/* Written by Jim McBeath (jimmc) at SCI */
/* last edit 19-Jan-85 09:01:20 by jimmc (Jim McBeath) */
/* last edit 11-Sept-85 22:07:00 by tlr (Terry L. Ridder) */

/* To use these routines, the user must have the variable Progname declared
 * elsewhere; also, these routines call sprintf. 
 */

#include <stdio.h>

#define ERROR_EXIT 1
#define BSIZE 200

extern char *Progname;		/* user must set up the progname to use */
extern char *sprintf();

/*..........*/

/* VARARGS1 */
warning(s, arg1, arg2)
char *s;
int  arg1, arg2;
{
char buf[BSIZE];

    sprintf(buf, s, arg1, arg2);
    fprintf(stderr, "%s: warning: %s\n", Progname, buf);
}

/*..........*/

/* VARARGS1 */
fatalerr(s, arg1, arg2)
char *s;
int  arg1, arg2;
{
char buf[BSIZE];

    sprintf(buf, s, arg1, arg2);
    fprintf(stderr, "%s: fatal error: %s\n", Progname, buf);
    exit(ERROR_EXIT);
}

/*..........*/

/* VARARGS1 */
errormsg(s, arg1, arg2)
char *s;
int  arg1, arg2;
{
char buf[BSIZE];

    sprintf(buf, s, arg1, arg2);
    fprintf(stderr, "%s: error: %s\n", Progname, buf);
}

/* end */
