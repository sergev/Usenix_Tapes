/* pagemap - functions to manipulate a page of character data */
/* Written by Jim McBeath (jimmc) at SCI */
/* last edit 15-Sep-84 22:24:02 by jimmc (Jim McBeath) */
/* last edit 31-Aug-85 13:00:00 by tlr (Terry L. Ridder) */

#include "pagemap.h"
#include <stdio.h>

Pagemp
pageInit(r, c)		/* make a new page of data */
int r, c;		/* rows an columns desired in the page */
{
Pagemp pp;
int i, j;
char *ss;

    pp = XALLOC(Pagem, 1, "pageInit");	/* get the top level structure */
    pp->rows = r;
    pp->cols = c;		/* put in his numbers */
    pp->data = XALLOC(char *, r, "pageInit rowpointers");
    for (i = 0; i < r; i++)
    {
	pp->data[i] = ss = XALLOC(char, c+2, "pageInit data");
	for (j = 0; j <= c; j++)
	{
	    ss[j]=' ';	/* fill with spaces */
	}
	ss[c+1] = 0;		/* null terminated */
    }
    return pp;			/* return the pointer to him */
}

/*..........*/

pagePuts(pp, r, c, ss)
Pagemp pp;
int r,c;
char *ss;
{
char *dd;
int n;

    dd = &pp->data[r][c];		/* where to put it */
    n = strlen(ss);			/* number of chars to move */
    if (pp->cols - c < n)
    {
	 n = pp->cols - c;	/* don't run off page */
    }
    for ( ; n > 0; n--)
    {
	 *(dd++) = *(ss++);	/* transfer string */
    }
}

/*..........*/

pagePrint(pp, f)		/* output the page */
Pagemp pp;		/* pointer to the page to output */
FILE *f;		/* stream to output to */
{
int r, c;
int lastline;
char *ss;

    lastline = -1;
    for (r = 0; r < pp->rows; r++)	/* for each row */
    {
	ss = pp->data[r];		/* faster access */
	for (c = pp->cols + 1; c >= 0; c--)
	{
	    if (ss[c] > ' ')
	    {
		 break;	/* strip trailing spaces and nulls */
	    }
	}
	ss[++c] = 0;			/* make it null terminated */
	if (c > 0)
	{
	    lastline = r;		/* remember where the last line is */
	}
    }
    for (r = 0; r <= lastline; r++)		/* now output the lines */
    {
	fprintf(f, "%s\n", pp->data[r]);
    }
}

/* end */
