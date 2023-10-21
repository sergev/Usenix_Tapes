/* famgdat.c - routines to get pieces of info records */
/* Written by Jim McBeath (jimmc) at SCI */
/* last edit 19-Jan-85 08:45:48 by jimmc (Jim McBeath) */
/* last edit 10-Sept-85 00:04:00 by tlr (Terry L. Ridder) */

/* These routines all package up various pieces of information
 * from specified records.  They each take as arguments a record
 * ID number; some take a field name as well.  Most return a pointer
 * to a string allocated from dynamic memory, which can be freed
 * by passing the pointer to the free() function.  Exceptions are
 * noted (e.g. fgnum returns an int).
 * Functions are:
 * fgnum(n,s)		returns the integer value of field s
 * fgstr(n,s)		the basic string routine which returns field s
 * bstr(n,s,b)		like fgstr, but returns string in buffer b
 * tname(n)		all of name but last name
 * bname(n)		full born (maiden) name
 * fname(n)		full name (including maiden and married last names)
 * fgbirth(n)		birth date and place
 * fgdeath(n)		death date and place
 * fgbegend(n)		birth and death dates
 * fgmar(n)		marriage date and place
 * fgclist(n,av)	get child list; return value is ac, fills pointer av
 */

#include "famtree.h"
#include "geneal.h"

#define TRUE 1
#define FALSE 0

extern char *strcat();
extern char *strcpy();

/* for forward references */
char *fgdateplace();

/*..........*/

int
fgnum(n,s)		/* get an item number from the data file */
int n;			/* person to get data item for */
char *s;		/* name of data item */
{
int tt, dd;
char *str;

    if (gendp == 0) 
    {
	fatalerr("no data file opened");
    }
    if (n <= 0) 
    {
	return -1;
    }
    str = getData(gendp, n, s);		/* get pointer to data item */
    if (str == 0) 
    {
	return -1;			/* -1 if no such item */
    }
    tt = sscanf(str, "%d", &dd);	/* get the number */
    if (tt != 1) 
    {
	return -1;			/* if no succesful scan */
    }
    return dd;			/* return the number we found */
}

/*..........*/

char *
fgstr(n, s)		/* get an item string from the data file */
int n;			/* person to get data item for */
char *s;		/* name of data item */
{
char *str;

    if (gendp == 0) 
    {
	fatalerr("no data file opened");
    }
    if (n <= 0) 
    {
	return "";
    }
    str = getData(gendp, n, s);	/* get pointer to data item */
    if (str == 0) 		/* null string if no such item */
    {
	return "";	
    }
    return strsav(str);
}

/*..........*/

int			/* returns 1 if found, 0 if not */
bstr(n, s, b)		/* get an item string from the data file */
int n;			/* person to get data item for */
char *s;		/* name of data item */
char *b;		/* the buffer to put it into */
{
    char *str;

    if (gendp == 0) 
    {
	fatalerr("no data file opened");
    }
    if (n <= 0) 			/* make string null */
    { 
	*b = 0; 
	return 0; 
    }					
    str = getData(gendp, n, s);	        /* get pointer to data item */
    if (str == 0) 			/* null string if no such item */
    { 
	*b = 0; 
	return 0; 
    }					/* null string if no such item */
    strcpy(b, str);		        /* copy the string to his buffer */
    return 1;
}

/*..........*/

char *
tname(n)			/* get the full name (except last name) */
int n;				/* person to get data item for */
{
char xname[200];
char Fname[100], mname[100], nname[100];

    bstr(n, "FN", Fname);	/* get first name */
    bstr(n, "MN", mname);	/* get middle name */
    bstr(n, "NN", nname);	/* get nick-name */
    xname[0] = 0;		/* start with null */
    if (indexes) 
    {
	sprintf(xname, "[%d]", n);	/* put in the id number */
    }
    addcstr(xname, Fname);	/* add first name */
    addcstr(xname, mname);	/* add middle name */
    addpstr(xname, nname);	/* add nickname in parens */
    return strsav(xname);
}

/*..........*/

char *
bname(n)		/* get the full born name */
int n;			/* person to get data item for */
{
char xname[200];
char Fname[100], mname[100], nname[100], lname[100], oname[100];

    bstr(n, "FN", Fname);	/* get first name */
    bstr(n, "MN", mname);	/* get middle name */
    bstr(n, "NN", nname);	/* get nick-name */
    bstr(n, "LN", lname);	/* get last name */
    bstr(n, "LNM", oname);	/* original last name */
    xname[0] = 0;		/* start with null */
    if (indexes) 
    {
	sprintf(xname, "[%d]", n);	/* put in the id number */
    }
    addcstr(xname, Fname);
    addcstr(xname, mname);
    addpstr(xname, nname);
    addcstr(xname, oname);
    if (oname[0] == 0) 
    {
	addcstr(xname, lname); /* use last name of no maiden */
    }
    return strsav(xname);
}

/*..........*/

char *
fname(n)		/* get the full name */
int n;			/* person to get data item for */
{
char xname[200];
char Fname[100], mname[100], nname[100], lname[100], oname[100];

    bstr(n, "FN", Fname);	/* get first name */
    bstr(n, "MN", mname);	/* get middle name */
    bstr(n, "NN", nname);	/* get nick-name */
    bstr(n, "LN", lname);	/* get last name */
    bstr(n, "LNM", oname);	/* original last name */
    xname[0] = 0;		/* start with null */
    if (indexes) 
    {
	sprintf(xname,"[%d]", n);	/* put in the id number */
    }
    addcstr(xname, Fname);
    addcstr(xname, mname);
    addpstr(xname, nname);
    addcstr(xname, oname);
    addcstr(xname, lname);
    return strsav(xname);
}

/*..........*/

char *
fgbirth(n)			/* get birth date info */
int n;				/* person to get data item for */
{
    return fgdateplace(n, "B", "BP", "b");
}

/*..........*/

char *
fgdeath(n)			/* get death date info */
int n;				/* person to get data item for */
{
    return fgdateplace(n, "D", "DP", "d");
}

/*..........*/

char *
fgbegend(n)			/* get birth/death date info */
int n;				/* person to get data item for */
{
char bdate[100], ddate[100], dates[200];

    bstr(n, "B", bdate);	/* get birth date */
    bstr(n, "D", ddate);	/* get death date */
    if (*bdate == 0 && *ddate == 0) 
    {
	return "";
    }
    if (*ddate == 0) 
    {
	sprintf(dates, "( b: %s )", bdate);
    }
    else if (*bdate == 0) 
    {
	sprintf(dates, "( d: %s )", ddate);
    }
    else 
    {
	sprintf(dates, "( b: %s, d: %s )", bdate, ddate);
    }
    return strsav(dates);
}

/*..........*/

char *
fgmar(n)			/* get marriage date info */
int n;				/* person to get data item for */
{
    return fgdateplace(n, "M", "MP", "m");
}

/*..........*/

char *
fgdateplace(n, dk, pk, cc)	/* get date/place info */
int n;				/* person to get data item for */
char *dk;			/* date keyword */
char *pk;			/* place keyword */
char *cc;			/* string to use as output key */
{
char date[100], place[100];

    bstr(n, dk, date);		/* get date */
    bstr(n, pk, place);		/* get place */
    if (*date == 0 && *place == 0) 
    {
	return "";
    }
    if (*date && *place) 
    {
	return tprintf("%s: %s, %s", cc, date, place);
    }
    if (*date) 
    {
	return tprintf("%s: %s", cc, date);
    }
    return tprintf("%s: %s", cc, place);
}

/*..........*/

int			/* returns count */
fgclist(n, av)
int n;			/* id of family */
int **av;		/* ADDRESS of where to put the av we return */
{
    char cstr[200];
    char *cstrp;
    int *clist;
    int i, ac, tt, dd;

    bstr(n, "C", cstr);	/* get list of kids */
    cstrp = cstr;

    for (ac = 0; (cstrp && *cstrp); ac++)
    {
	cstrp = index(cstrp + 1, ',');	/* count separators */
    }
    if (ac == 0) 
    {
	return 0;
    }
    clist = XALLOC(int, ac, "fgclist");
    for(cstrp = cstr, i = 0; (cstrp && *cstrp); i++)
    {
	if (i >= ac) 
	{
	    fatalerr("loop too far on child list in fgclist");
	}
	tt = sscanf(cstrp, "%d", &dd);	/* read child number */
	if (tt == 1) 
	{
	    clist[i] = dd;
	}
	else
	{
	    warning("bad child list format in family %d", n);
	    clist[i] = dd = -1;
	}
	cstrp = index(cstrp, ',');
	if (cstrp != 0) 
	{
	    cstrp++;
	}
    }
    if (i != ac)
    {
	warning("bad child list format in family %d", n);
	for (; i < ac; i++)
	{
	    clist[i] = -1;	/* fill with -1 */
	}
    }
    *av = clist;		/* fill in pointer */
    return ac;			/* return count */
}

/*..........*/

addcstr(dd, ss)			/* add a string to another */
char *dd;			/* the string being built */
char *ss;			/* the string to add if not null */
{
    if (ss && *ss)
    {
	if (dd[0]) 
	{
	    strcat(dd," ");
	}
	strcat(dd, ss);
    }
}

/*..........*/

addpstr(dd, ss)			/* add a string in parens */
char *dd;			/* the string to add to */
char *ss;			/* the string to add in parens if not null */
{
    if (ss[0])
    {
	if (dd[0]) 
	{
	    strcat(dd, " ");
	}
	strcat(dd, "(");
	strcat(dd, ss);		/* add the string in parens */
	strcat(dd, ")");
    }
}

/* end */
