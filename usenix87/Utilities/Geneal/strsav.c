/* strsav - make a copy of a string and return a pointer to it */
/* Written by Jim McBeath (jimmc) at SCI */
/* last edit  6-Jan-85 22:21:20 by jimmc (Jim McBeath) */
/* last edit 31-Aug-85 13:00:00 by tlr (Terry L. Ridder) */

extern char *strcpy();
extern char *xalloc();

char *
strsav(ss)
char *ss;
{
char *dd;

    dd = xalloc(strlen(ss) + 1, "strsav");
    strcpy(dd, ss);		/* make a copy of the string */
    return dd;			/* return the copy */
}

/* end */
