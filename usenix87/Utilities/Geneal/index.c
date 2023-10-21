/* index - index handler for large pseudo-arrays */
/* Written by Jim McBeath (jimmc) at SCI */
/* last edit 10-Jan-85 07:59:39 by jimmc (Jim McBeath) */
/* last edit 31-Aug-85 13:00:00 by tlr (Terry L. Ridder) */

/* This module implements a very large pseudo-array.  There are three
   entry points, to initialize a pseudo-array, to insert an item, and
   to read an item.  The array contains integers (which can of course
   be used for another purpose, such as pointers to something else).

   Structure of a table:
   A table is a recursive structure which contains 2 words of size
   information and n words of pointers.  The first word of size
   information tells how many entries are represented by this and
   all lower tables; the second word of size information tells how
   many entries are actually in this table.  If the two numbers are
   the same, then the table is an end node which actually has the
   data entries in it.
*/

/*..........*/

#include "geneal.h"

extern char *malloc();		/* added by tlr */

int indexDebug = 0;		/* a debugging flag */

/*..........*/

struct toplevel *		/* pointer to the structure for future calls */
initIndex()			/* init a new index table */
{
struct toplevel *tt;
struct tblblock *dd;
int i;

    tt = (struct toplevel *)malloc(sizeof(struct toplevel));
	/* get space for top level block */
    if (!tt)
    {
	 return 0;
    }
    dd = (struct tblblock *)malloc(sizeof(struct tblblock));
	/* get space for top level table */
    if (!dd) 
    {
        free((char *)tt);
        return 0;
    }
    tt->data = dd;			/* save pointer in our block */
    tt->numlevs = 1;		/* we always start with one level */
    dd->repnum = TBLSIZ;
    dd->count = TBLSIZ;
    for (i = 0; i < dd->count; i++)
    {
	dd->tdata[i] = 0;	/* clear all data */
    }
    return tt;			/* return pointer to top level block */
}

/*..........*/

setIndex(tt, ix, num)		/* put index value into table */
struct toplevel *tt;		/* table to use */
int ix;				/* the index where it goes */
int num;			/* the value to put there */
{
struct tblblock *dd, *dd0;
int i;

    if (indexDebug) 
    {
        printf("setIndex: index=%d, value=%d\n", ix, num);
        if (!tt)
	{
	    printf("setIndex: no table index\n");
	}
        else if (!(tt->numlevs))
	{
	    printf("setIndex: no numlevs\n");
	}
        else if (!(tt->data))
	{
	    printf("setIndex: no data array\n");
	}
    }
    if (!tt)
    {
	 return -1;		/* check for errors */
    }
    if (!(tt->numlevs))
    {
	 return -1;
    }
    if (!(tt->data))
    {
	 return -1;
    }
    dd = tt->data;			/* get data pointer */
    while (ix >= dd->repnum)
    {	/* create a higher level */
    	if (indexDebug)
    	{
	 	printf("setIndex: %d(ix) > %d(repnum)\n", ix, dd->repnum);
    	}
    	dd0 = (struct tblblock *)malloc(sizeof(struct tblblock));
		/* get space for a higher level */
    	if (!dd0)
    	{
	 	return -1;		/* error */
    	}
    	dd0->repnum = dd->repnum*TBLSIZ;
    	dd0->count = TBLSIZ;
    	for (i = 0; i < TBLSIZ; i++)
    	{
	 	dd0->tdata[i] = 0;	/* clear table */
    	}
    	dd0->tdata[0] = (int)dd;	/* put in pointer to next level down */
    	tt->data = dd0;		/* put in new top-level pointer */
    	tt->numlevs++;
    	if (indexDebug)
    	{
	 	printf("setIndex: numlevs=%d\n", tt->numlevs);
    	}
    	dd = dd0;
    }
    while (dd->repnum > dd->count)
    {	/* scan down to the last level */
        if (indexDebug)
        {
	    printf("setIndex: %d(repnum) > %d(count)\n",
	    dd->repnum, dd->count);
        }
        dd0 = (struct tblblock *)(dd->tdata[ix/dd->count]);
	/* get pointer to next table lower */
        if (!dd0)
        {			/* if no table there, have to make one */
	    dd0 = (struct tblblock *)malloc(sizeof(struct tblblock));
	    if (!dd0)
	    {
	       return -1;		/* error */
	    }
	    dd0->repnum = dd->repnum/dd->count;
	    dd0->count = TBLSIZ;
	    for (i = 0; i < TBLSIZ; i++)
	    {
	        dd0->tdata[i] = 0; /* clear the new table */
	    }
	    dd->tdata[ix/dd->count] = (int)dd0;	/* save pointer to it */
	}
        ix %= dd->count;		/* lower the index */
        dd = dd0;
    }
    dd->tdata[ix] = num;		/* put it in */
    if (indexDebug)
    {
	printf("setIndex: table %X, index %d, value %d\n",
	dd, ix, num);
    }
    return ix;
}

/*..........*/

int				/* return value out of table */
				/*  returns 0 if no entry */
getIndex(tt, ix)
struct toplevel *tt;
int ix;				/* the index to look for */
{
struct tblblock *dd, *dd0;

    if (indexDebug) 
    {
        printf("getIndex: index=%d\n", ix);
        if (!tt) 
	{
	    printf("getIndex: no table\n");
	}
        else if (!tt->data) 
	{
	    printf("getIndex: no data array\n");
	}
        else if (!tt->numlevs) 
	{
	    printf("genIndex: no numlevs\n");
	}
    }
    if (!tt) 
    {
	return 0;		/* check for errors */
    }
    if (!tt->data) 
    {
	return 0;
    }
    if (!tt->numlevs)
    {
 	 return 0;
    }
    dd = tt->data;
    if (ix >= dd->repnum) 
    {
        if (indexDebug) 
	{
	    printf("getIndex: index %d > repnum %d\n", ix,dd->repnum);
	}
        return 0;	/* we don't have them that high */
    }
    while (dd->repnum > dd->count) 
    {	/* scan down to bottom level */
        if (indexDebug) 
	{
	    printf("getIndex: %d(repnum) > %d(count)\n",
		dd->repnum, dd->count);
	}
        dd0 = (struct tblblock *)(dd->tdata[ix/dd->count]);
	/* get pointer to next level */
        if (!dd0) 
	{
	    if (indexDebug) 
	    {
		printf("getIndex: no table\n");
	    }
	    return 0;		/* nothing there */
	}
        ix %= dd->count;
        dd = dd0;
    }
    if (indexDebug)
    {
	printf("getIndex: table %X, index %d, value %d\n",
	    dd, ix, dd->tdata[ix]);
    }
    return dd->tdata[ix];		/* this is the data entry */
}
    
/* end of index */
