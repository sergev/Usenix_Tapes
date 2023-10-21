/* gbrowse.c - browse through the genealogy file */
/* Written by Jim McBeath (jimmc) at SCI */
/* last edit 19-Jan-85 09:08:30 by jimmc (Jim McBeath) */
/* last edit 31-Aug-85 13:00:00 by tlr (Terry L. Ridder) */

#include "geneal.h"

extern char *gets();

int		/* 0 if OK */
gbrowse()	/* browse the database */
{
char *ss;
char lbuf[1000];
char tbuf[1000];
int n;
int t;

    indexes++;		/* note we want to see all the index numbers */
    while (1)		/* until a break takes us out */
    {
	printf(">");		/* give the browse prompt */
	ss = gets(lbuf);		/* read in a line */
	if (ss == 0)
	{
	    break;		/* quit on eof */
	}
	if (lbuf[0] == 0)
	{
	    continue;	/* ignore blank lines */
	}
	t = sscanf(lbuf, "%d", &n);	/* read his number */
	if (t != 1)
	{
	    printf("numbers only, please\n");
	    continue;		/* read another line */
	}
	if (n <= 0)
	{
	    continue;
	}
	t = bstr(n, "T", tbuf);	/* see what the record type is */
	if (t == 0)		/* if not there */
	{
	    printf("bad record %d\n", n);
	    continue;
	}
	if (strcmp(tbuf, "I") == 0)
	{
	    indivs(n);
	}
	else if (strcmp(tbuf, "F") == 0)
	{
	    family(n);
	}
	else
	{
	    printf("unknown type code %s in record %d\n", tbuf, n);
	}
/* continue the loop */
    }
    return 0;
}

/* end */
