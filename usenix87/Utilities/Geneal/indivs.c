/* indivs.v - print out short form info about an individual */
/* Written by Jim McBeath (jimmc) at SCI */
/* last edit 19-Jan-85 08:05:47 by jimmc (Jim McBeath) */
/* last edit 31-Aug-85 13:00:00 by tlr (Terry L. Ridder) */

#include "geneal.h"

extern char *sprintf();

int			/* 0 if OK */
indivs(n)
int n;			/* the person's ID number */
{
char lbuf[1000];
char *ss;
int pnum;		/* parent family number */
int fnum;		/* father's number */
int mnum;		/* mother's number */
int mgnum;		/* marriage number */
int snum;		/* spouse's number */
int mhnum, mwnum;
char mgstr[20];		/* used for building key string */
int i;			/* loop counter */
int t;			/* random numbers and status */
char *sostr;

    bstr(n, "T", lbuf);
    if (lbuf[0] == 0 || strcmp(lbuf, "I") != 0)
    {
	warning("id %d is not an individual");
	return 1;
    }
    ss = fname(n);
    printf("Name: %s\n", ss);
    ss = fgbirth(n);
    if (ss && *ss) 
    {
	printf("%s\n", ss);
    }
    ss = fgdeath(n);
    if (ss && *ss) 
    {
	printf("%s\n", ss);
    }
    pnum = fgnum(n, "P");	        /* get number of parent family */
    if (pnum > 0)			/* if we got a family */
    {
	if (indexes) 
	{
	    printf("Parent family: %d\n", pnum);
	}
	fnum = fgnum(pnum, "H");
	if (fnum >= 0)
	{
	    ss = fname(fnum);
	    printf("  Father: %s\n", ss);
	    ss = fgbegend(fnum);
	    if (ss && *ss) 
	    {
		printf("  %s\n", ss);
	    }
	}
	mnum = fgnum(pnum, "W");
	if (mnum >= 0)
	{
	    ss = fname(mnum);
	    printf("  Mother: %s\n", ss);
	    ss = fgbegend(mnum);
	    if (ss && *ss) 
	    {
		printf("  %s\n", ss);
	    }
	}
	ss = fgmar(pnum);
	if (ss) 
	{
	    printf("Parent's marriage: %s\n", ss);
	}
	doiclist(pnum, "Sibling", "Siblings", n);	/* do the siblings */
    }
    else	/* no parent in this record */
    {
	if (indexes) 
	{
	    printf("No parent family specified\n");
	}
    }
    for (i = 0; ; i++)	/* look at marriages */
    {
	sprintf(mgstr, "S%d", i);	/* build indicator string */
	mgnum = fgnum(n, mgstr);	/* get number of marriage */
	if (mgnum <= 0) 
	{
	    break;		/* stop if no more */
	}
	if (indexes) 
	{
	    printf("Marriage S%d: %d\n", i, mgnum);
	}
	mhnum = fgnum(mgnum, "H");	/* are we the husband? */
	mwnum = fgnum(mgnum, "W");	/*  or the wife */
	if (n != mhnum && n != mwnum)
	    warning("person %d claims marriage %d, but not vice-versa",
		n, mgnum);
	if (n == mhnum)
	{
	    snum = mwnum;
	    sostr = "Wife";
	}
	else
	{
	    snum = mhnum;
	    sostr = "Husband";
	}
	if (snum > 0)
	{
	    ss = bname(snum);
	    printf(" %s: %s\n", sostr, ss);
	    ss = fgbegend(snum);
	    if (ss && *ss) 
	    {
		printf(" %s\n", ss);
	    }
	}
	ss = fgmar(mgnum);
	if (ss && *ss) 
	{
	    printf(" %s\n", ss);
	}
	doiclist(mgnum,"Child","Children",-1);
    }
    for (i = 0; ; i++)
    {
	sprintf(mgstr, "GEN%d", i);
	t = bstr(n, mgstr, lbuf);
	if (t == 0) 
	{
	    break;		/* no more strings */
	}
	if (i == 0) 
	{
	    printf("General:\n");
	}
	printf("%s\n", lbuf);		/* print each line of GEN */
    }
    return 0;				/* finished it all OK */
}

/*..........*/

doiclist(pnum, c1, cm, xn)	/* print out info about children */
int pnum;			/* the family to deal with */
char *c1;			/* string to use if one child */
char *cm;			/* string to use if more than one child */
int xn;				/* number of special sibling, or -1 if none */
{
int i;
int t;
char *ss;
int *clist;
int cflag = 0;

    t = fgclist(pnum, &clist);	/* get list of kids */
    if (t == 0)
    {
	if (indexes) 
	{
	    printf("No %s specified\n", cm);
	}
	if (xn > 0) 
	{
	    warning("person %d claims parents %d but not vice-versa",
		xn, pnum);
	}
    }
    else
    {
	if (t == 1) 
	{
	    printf("  1 %s:\n", c1);
	}
	else 
	{
	    printf("  %d %s:\n", t, cm);
	}
	for (i = 0; i < t; i++)	/* for each child */
	{
	    if (clist[i] == xn) 
	    {
		cflag++;	/* note found special sibling */
	    }
	    if (indexes) 
	    {
		printf("  %s %d:", c1, i);
	    }
	    ss = tname(clist[i]);
	    printf("  %s%s\n", (clist[i]==xn?"*":""), ss);
	    ss = fgbegend(clist[i]);
	    if (ss && *ss) 
	    {
	   	printf("  %s\n", ss);
	    }
	}
	if (xn > 0 && cflag == 0)
	{
	    warning("person %d claims parents %d but not vice-versa",
		xn, pnum);
	}
    }
}

/* end */
