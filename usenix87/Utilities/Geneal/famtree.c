/* famtree - make a family tree */
/* Written by Jim McBeath (jimmc) at SCI */
/* last edit 19-Jan-85 08:49:00 by jimmc (Jim McBeath) */
/* last edit 10-Sept-85 07:58:00 by tlr (Terry L. Ridder) */

/* A family tree is composed of a number of family blocks
 * connected by lines.  Each family block is a rectangle, with
 * lines coming out the top and bottom for the father and mother
 * families, respectively, and one line coming out the right
 * for the child families.  The family blocks are then placed
 * in the appropriate positions and connected by extending the
 * lines.
 */

#include "geneal.h"
#include "famtree.h"
#include "pagemap.h"

#define TRUE 1
#define FALSE 0

Fsqp famgen();

extern struct dpoint *gendp;		/* the datafile index number */
extern int dataStatus;
extern char *dataErrStrs[];

/*..........*/

int				/* 0 if OK */
famtree(n)
int n;				/* index number of the person or family
				 to do the tree for */
{
Fsqp fm;

    fm = famgen(n);	/* generate this family block and all ancestors */
/*    famdump(fm);	/*** dump the info we created */
    famgtmp(fm);	/*** output a portion of the tree */
    return 0;		/* assume it all went OK! */
}

/*..........*/

Fsqp
famgen(n)		/* generate a family block for person or family n */
int n;
{
Fsqp fm;
char *rtype;
char *cstr, *cstr0;
int i;
int individual;		/* set if it is an individual; not set if family */
int nochildren;
int childfound;
int tt, dd;

    if (n <= 0) 
    {
	return 0;
    }
    fm = XALLOC(Fsq, 1, "famgen");
    rtype = fgstr(n, "T");		/* get record type */
    if (rtype == 0 || *rtype == 0) 
    {
	return 0;	/* must have this record! */
    }
    if (rtype[0] == 'I')		/* if an individual's record */
    {
	individual = 1;
	fm->cnum = n;
	fm->pnum = fgnum(n, "P");
    }
    else
    {
	individual = 0;		/* not an individual */
	fm->cnum = -1;
	fm->pnum = n;
    }
    fm->pname = fgstr(fm->pnum, "N");
    fm->fnum = fgnum(fm->pnum, "H");
    fm->mnum = fgnum(fm->pnum, "W");
    fm->ffamily = famgen(fm->fnum);
    fm->mfamily = famgen(fm->mnum);
    if (fm->pnum > 0)
    {
	cstr = getData(gendp, fm->pnum, "C");	/* get child info */
	if (cstr == 0 || *cstr == 0 )
	{		/*** check for individual here? */
	    if (dataStatus == 7)
	    {
		warning("inconsistent data: person %d claims family %d,\n\
		which does not exist!", n, fm->pnum);
	    }
	    else if (dataStatus == 8)
	    {
		warning("inconsistent data: person %d claims family %d,\n\
		which has no child list!", n, fm->pnum);
	    }
	    else if (dataStatus == 0)	/* must be strlen(cstr)==0 */
	    {
		warning("inconsistent data: person %d claims family %d,\n\
		which has an empty child list!", n, fm->pnum);
	    }
	    else
	    {
		warning("error in getData for family %d: %s",
			fm->pnum, dataErrStrs[dataStatus]);
	    }
	    nochildren = TRUE;
	}
	else	/* we have a cstr */
	{
	    for (i = 0, cstr0 = cstr; (cstr0 && *cstr0); i++)
	    {
		cstr0 = index(cstr0 + 1, ',');
		    /* count separators to determine number in list */
	    }
	    fm->ccount = i;
	    fm->clist = XALLOC(int, i, "famgen clist");
	    childfound = FALSE;
	    for(cstr0 = cstr, i = 0; (cstr0 && *cstr0); i++)
	    {
		if (i >= fm->ccount) 
		{
		    fatalerr("loop too far on child list");
		}
		tt = sscanf(cstr0, "%d", &dd);	/* read child number */
		if (tt == 1) 
		{
		    fm->clist[i] = dd;
		}
		else
		{
		    warning("bad child list format in family %d", fm->pnum);
		    fm->clist[i] = dd = -1;
		}
		if (dd == n)
		{
		    childfound = TRUE;
		    fm->chloc = i;	/* remember his position in list */
		}
		cstr0 = index(cstr0, ',');
		if (cstr0 != 0) 
		{
		    cstr0++;
		}
	    }
	    if (individual && !childfound)
	    {
		warning("inconsistent data: person %d claims family %d,\n\
		but family does not claim child!", n, fm->pnum);
	    }
	    nochildren = FALSE;
	}
    }
    else nochildren = TRUE;
    if (nochildren)
    {
	fm->ccount = individual? 1 : 0;
	fm->clist = XALLOC(int, 1, "famgen clist");
	fm->clist[0] = individual? n : 0;
	fm->chloc = 0;
    }
    fm->cnlist = XALLOC(char *, fm->ccount, "famtree cnlist");
    fm->cblist = XALLOC(char *, fm->ccount, "famtree cblist");
    fm->cols = 0;
    for (i = 0; i < fm->ccount; i++)	/* fill in child info */
    {
	fm->cnlist[i] = tname(fm->clist[i]); /* get names */
	fm->cblist[i] = fgbegend(fm->clist[i]); /* get date info */
	if ((tt = strlen(fm->cnlist[i])) > fm->cols) 
	{
	    fm->cols = tt;
	}
	if ((tt = strlen(fm->cblist[i])) > fm->cols) 
	{
	    fm->cols = tt;
	}
		/* keep track of longest name */
    }
    if (strlen(fm->pname) > 2*fm->ccount) 
    {
	fm->lines = strlen(fm->pname);
    }
    else 
    {
	fm->lines = 2*fm->ccount;
	/* there must be enough lines for both the vertical family name
		and the list of children (2 lines per child) */
    }
    fm->acolmax = fm->cols;
    fm->alines = 0;
    fm->agens = 0;
    if (fm->ffamily)
    {
	if (fm->ffamily->acolmax > fm->acolmax)
	{
	    fm->acolmax = fm->ffamily->acolmax;
	}
	if (fm->ffamily->agens+1 > fm->agens)
	{
	    fm->agens = fm->ffamily->agens + 1;
	}
	fm->alines += fm->ffamily->alines;
    }
    if (fm->mfamily)
    {
	if (fm->mfamily->acolmax > fm->acolmax)
	{
	    fm->acolmax = fm->mfamily->acolmax;
 	}
	if (fm->mfamily->agens + 1 > fm->agens)
	{
	    fm->agens = fm->mfamily->agens + 1;
	}
	fm->alines += fm->mfamily->alines;
    }
    if (fm->lines > fm->alines) 
    {
	fm->alines = fm->lines;
    }
/*** still need to calculate chline */
    fm->chline = 0;	/***/
    return fm;
}

/*..........*/

famdump(fm)		/* dump tree info */
Fsqp fm;
{
int i;

    if (fm == 0) 
    {
	return;
    }
    printf("Family at %X: \"%s\", ", fm, fm->pname);
    printf("P=%d, F=%d, M=%d, C=%d, ",
	fm->pnum, fm->fnum, fm->mnum, fm->cnum);
    printf("ccount=%d, clist=%X:\n", fm->ccount, fm->clist);
    for (i = 0; i < fm->ccount; i++) printf("%2d \"%s\" %s\n",
	fm->clist[i], fm->cnlist[i], fm->cblist[i]);
    {
    	printf("lines=%d, cols=%d, chloc=%d, agens=%d, \
	alines=%d, acolmax=%d, chline=%d;\n",
	fm->lines, fm->cols, fm->chloc, fm->agens,
	fm->alines, fm->acolmax, fm->chline);
    }
    printf("ffamily=%X, mfamily=%X\n\n", fm->ffamily, fm->mfamily);
    famdump(fm->ffamily);
    famdump(fm->mfamily);
}

/*..........*/

famgtmp(fm)		/* for debugging - output one family in tree form */
Fsqp fm;
{
Pagemp pp;
int lines, cols;
int i, j, x;
char *ss;

    lines = fm->lines; cols=fm->cols;
    pp = pageInit(lines, cols+5);
    for (i = 0; i < lines; i++)		/* put in the vertical line */
    {
	pagePutc(pp, i, 2, '|');
    }
    x = (lines-strlen(fm->pname))/2;	/* calculate starting position */
    for (i = x, ss = fm->pname; *ss; i++, ss++)
    {
	pagePutc(pp, i, 0, *ss);	/* put family name in vertically */
    }
    x = (lines - (2*fm->ccount))/2;	/* starting line for children */
    for (i = 0; i < fm->ccount; i++)
    {
	if (i == fm->chloc)		/* if this is the child of interest */
	{
	    for (j = 4; j < pp->cols; j++)
	    {
		pagePutc(pp, 2 * i + x, j, '-');
		    			/* put in a row of dashes first */
	    }
	}
	pagePuts(pp, 2 * i + x, 4, fm->cnlist[i]);
	pagePuts(pp, 2 * i + x + 1, 4, fm->cblist[i]);
    }
    pagePrint(pp, stdout);		/* output it */
}

/* end */
