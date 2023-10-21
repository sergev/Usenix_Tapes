/* tprintf - do a sprintf into a temp buffer, then make a copy of the
 * string and return a pointer to the copy */
/* Written by Jim McBeath (jimmc) at SCI */
/* last edit 19-Jan-85 08:59:43 by jimmc (Jim McBeath) */
/* last edit 11-Sept-85 22:17:00 by tlr (Terry L. Ridder) */

#define BUFSIZ 1000

extern char *strsav();
extern char *sprintf();

/* VARARGS1 */
char *
tprintf(fmt, arg1, arg2, arg3)
char *fmt;
int  arg1, arg2, arg3;
{
char buf[BUFSIZ];

    sprintf(buf, fmt, arg1, arg2, arg3);	/* printf the string */
    return strsav(buf);				/* return a copy */
}

/* end */
