/*	cmdpfd.c	COMND module; Date/time function parse.

	Copyright (C) 1985 Mark E. Mallett

	Permission is hereby granted to distribute this file indiscriminately.

	This file contains the code for the date/time parsing function.

Edit history

When	Who	What
------	---	--------------------------------
84xxxx	MEM	Create file.

	Routines included:

		CFPdat		Parse date and/or time

*/


#include "stdio.h"			/* Standard system defs */
#include "comnd.h"			/* COMND interface definitions */
#include "comndi.h"			/* COMND internal definitions */


typedef					/* Structure for "special" dates */
  struct {
    int		_mo;			/* Month (1 - 12) */
    int		_da;			/* Day of month (1 - 31) */
  } SMD;


/* External routines */


/* External data */

extern	int	CMDbel;			/* Beep request flag */

/* Internal (public) routines */



/* Internal (public) data */


/* Local (static) data */

	/* Tables for date and time parsing.  */

static	char	*Mnmtbl[] = {		/* Month names */
			"January",	"February",	"March",
			"April",	"May",		"June",
			"July",		"August",	"September",
			"October",	"November",	"December",
			NULL};

static	char	*Snmtbl[] = {		/* Table of special names */
			"April-Fools-Day",
			"Christmas",
			"Erics-Birthday",
			"Fourth-of-July",
			"Halloween",
			"New-Years-Day",
			"Valentines-day",
			NULL
			    };

static	SMD	Sndtbl[] = {		/* month/day for special names */
			4, 1,		/* April fools */
			12, 25,		/* Christmas */
			8, 19,		/* Eric's birthday */
			7, 4,		/* Fourth of July */
			10, 31,		/* Halloween */
			1, 1,		/* New Year's Day */
			2, 14,		/* Valentine's day */
			0, 0};		/*  end */


static	char	*Dowtbl[] = {		/* Day-of-week table */
			"Sunday", "Monday", "Tuesday", "Wednesday",
			"Thursday", "Friday", "Saturday", NULL
			    };


static	char	*Todtbl[] = {		/* Time-of-day table */
			"noon",
			NULL
			    };


static	CFB	Timcfb = {_CMNUM, _CFHPP|_CFSDH, 0, 10,
			"Time in the form hh:mm:ss", 0};
static	CFB	Todcfb = {_CMKEY, _CFHPP, &Timcfb, &Todtbl,
			"Time of day, ", 0};
static	CFB	Plscfb = {_CMTOK, _CFHPP|_CFSDH, 0, "+",
			  "+ for future offset from now", 0};
static	CFB	Mincfb = {_CMTOK, _CFHPP|_CFSDH, &Plscfb, "-",
			  "- for past offset from now", 0};
static	CFB	Adncfb = {_CMNUM, _CFHPP|_CFSDH, &Mincfb, 10,
			"Decimal month number", 0};
static	CFB	Dowcfb = {_CMKEY, _CFHPP, &Adncfb, &Dowtbl,
			"Day of week, ", 0};
static	CFB	Snmcfb = {_CMKEY, _CFHPP, &Dowcfb, &Snmtbl,
			"Special date, ", 0};
static	CFB	Mnmcfb = {_CMKEY, _CFHPP, &Snmcfb, &Mnmtbl,
			"Month name, ", 0};

static	CFB	Clncfb = {_CMTOK, 0, 0, ":", 0, 0};
static	CFB	Cmycfb = {_CMTOK, _CFHPP|_CFSDH, 0, ",",
			"Comma and year", 0};
static	CFB	Domcfb = {_CMNUM, _CFHPP|_CFSDH, 0, 10, "Day of month", 0};
static	CFB	Yrcfb =  {_CMNUM, _CFHPP|_CFSDH, 0, 10, "Year", 0};

/*

*//* CFPdat (CSBptr, CFBptr, ccptr)

	Function parse for type=_CMDAT, date and/or time

This function parses a date and/or time.  It does this by calling
COMND recursively, with special date/time parsing function blocks.


Accepts :

	CSBptr		Address of command state block
	CFBptr		Address of command function block
	ccptr		Address of CC table (N/A here)


Returns :

	<value>		Parse status, _CPxxx as defined in comndi.h.

*/

CFPdat (CSBptr, CFBptr, ccptr)

CSB		*CSBptr;		/* Addr of command state block */
CFB		*CFBptr;		/* Addr of command function block */
WORD		*ccptr;			/* Addr of CC table */

{
IND	int	i,j,t;			/* Scratch */
IND	int	id,it;			/* Internal date and time */
IND	int	*rslptr;		/* Result pointer */

rslptr = CSBptr -> CSB_ABF;		/* Get pointer to result buffer */
curdtm (&id, &it);			/* Get today's date/time */
if (CFBptr -> CFB_FLG & _CFDTD)		/* If date wanted */
    {
    i = CFPdtd (CSBptr, CFBptr);	/* Try for date */
    if (!CFBptr -> CFB_FLG & _CFDTT)	/* If time not wanted */
	return (i);			/* Just return the result */

    switch (i)				/* Go by parse result of date */
	{
	case _CPABT:			/* If abort... */
	    return (_CPABT);		/*  return abort code */

	case _CPSUCC:			/* Success(!) */
	    id = rslptr[0];		/* Remember date */
	    j = COMND (CSBptr, &Clncfb); /* Try for colon for time */
	    if (j == _CRNOP)		/* If not parsed */
		{
		rslptr[0] = id;		/* Put date back */
		rslptr[1] = it;		/* Use current time */
		return (_CPSUCC);	/* Return ok */
		}
	    else if (j != _CROK)	/* If not OK */
		return (_CPABT);	/*  stop now */
	    break;			/* Continue with getting time! */

	case _CPAGN:			/* Try again later.. */
	case _CPGVH:			/* Gave help */
	case _CPNOP:			/* Could not parse.. */
	    rslptr[0] = id;		/* Store current date */
	    break;			/* Quit and continue */

	}
    }

/* Try for time, if wanted */

if (!(CFBptr -> CFB_FLG & _CFDTT))	/* If time not wanted */
    return (i);				/*  return result of date parse */

id = rslptr[0];				/* Remember date */
t = CFPdtt(CSBptr, CFBptr);		/* Try to parse time */
rslptr[0] = id;				/* Pass back date, if any */

if (!(CFBptr -> CFB_FLG & _CFDTD))	/* If we did not parse a date */
    return (t);				/*  return result of time attempt. */

if (i == _CPGVH)			/* If date gave help */
    return (_CPGVH);			/* then we HAVE to return that code */

return (t);				/* Otherwise return what time said. */
}
/*

*//* CFBdtd (CSBptr, CFBptr)

	Parse date portion of date/time

This function parses a date.  It does this by calling COMND
recursively.


Accepts :

	CSBptr		Address of command state block
	CFBptr		Address of command function block


Returns :

	<value>		Parse status, _CPxxx as defined in comndi.h.

*/

CFPdtd (CSBptr, CFBptr)

CSB		*CSBptr;		/* Addr of command state block */
CFB		*CFBptr;		/* Addr of command function block */

{
IND	CFB	*CFBmat;		/* Matching CFB address */
IND	int	m,d,y,id,it;		/* Date values */
IND	int	*rslptr;		/* Result pointer */
IND	char	**kptr;			/* Keyword ptr ptr */
IND	int	i;			/* Scratch */
IND	int	prem;			/* Remembers parse point */

prem = CSBptr -> CSB_PRS;		/* Remember the current parse point */
rslptr = CSBptr -> CSB_ABF;		/* Point to result area */
i = COMNDr (CSBptr, &Mnmcfb, 1);	/* Try parsing the date. */

if ((i != _CPSUCC) && (i != _CPCPE))	/* If not successful parse */
    return (i);				/*  then return whatever it was */

CFBmat = CSBptr -> CSB_CFB;		/* Get addr of matching CFB */

if (CFBmat == &Plscfb)			/* Plus date/time */
    {
    fprintf (stderr, "Relative date/time not supported\n");
    return (_CPABT);
    }

else if (CFBmat == &Mincfb)		/* Minus date/time */
    {
    fprintf (stderr, "Relative date/time not supported\n");
    return (_CPABT);
    }

else if ((CFBmat == &Mnmcfb)  || (CFBmat == &Snmcfb))
					/* Month or special date in year */
    {
    kptr = (char **) (CSBptr -> CSB_RVL._ADR);
					 /* Get the table entry address */
    if (CFBmat == &Mnmcfb)
	{
	m = (kptr - &Mnmtbl[0]) +1;	/* Get month number */

    /* Get day of month */

	i = COMNDr (CSBptr, &Domcfb, 2); /* Try for day of month */
	if ((i != _CPSUCC) && (i != _CPCPE)) /* Check not success */
	    {
	    CSBptr -> CSB_PRS = prem;	/* Reset parse pointer */
	    return (i);			/*  return */
	    }
	d = CSBptr -> CSB_RVL._INT;	/* Get day of month */
	}
    else				/* Special... */
	{
	i = (kptr - &Snmtbl[0]);	/* Get index */
	m = Sndtbl[i]._mo;		/* Get month */
	d = Sndtbl[i]._da;		/*  and day */
	}

    i = COMNDr (CSBptr, &Cmycfb, 2);	/* Try for comma and year */
    if ((i != _CPSUCC) && (i != _CPCPE)) /* Check not success */
	{
	CSBptr -> CSB_PRS = prem;	/* Reset parse pointer */
	return (i);
	}
    i = COMNDr (CSBptr, &Yrcfb, 2);	/* Now try for year */
    if ((i != _CPSUCC) && (i != _CPCPE)) /* Check not success */
	{
	CSBptr -> CSB_PRS = prem;	/* Reset parse pointer */
	return (i);
	}

    y = CSBptr -> CSB_RVL._INT;		/* Get year */
    i = cvedid (rslptr, &it, m, d, y, 0, 0);
    if (i != 0)
	{
	fprintf (stderr, "Bad component in date: %d\n", i);
	fprintf (stderr, "Inputs were %d/%d/%d\n", m, d, y);
	return (_CPNOP);		/* Bad parse */
	}
    return (_CPSUCC);			/* Got it. */
    }

else
    return (_CPNOP);			/* Nope. */
}
/*

*//* CFBdtt (CSBptr, CFBptr)

	Parse time portion of date/time

This function parses a time.  It does this by calling COMND
recursively.


Accepts :

	CSBptr		Address of command state block
	CFBptr		Address of command function block


Returns :

	<value>		Parse status, _CPxxx as defined in comndi.h.

*/

CFPdtt (CSBptr, CFBptr)

CSB		*CSBptr;		/* Addr of command state block */
CFB		*CFBptr;		/* Addr of command function block */

{
IND	int	i;			/* Scratch */

i = COMNDr (CSBptr, &Todcfb, 2);	/* Try parsing the time */
return (i);				/* Return now. */
}
