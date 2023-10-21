/*	cmdpf2.c	COMND module; function parsing routines, set # 2

	Copyright (C) 1985 Mark E. Mallett

	Permission is hereby granted to distribute this file indiscriminately.

	This file contains parsing routines for various individual
function codes.

Edit history

When	Who	What
------	---	--------------------------------
84xxxx	MEM	Create file.

	Routines included:

		CFPnum		Number parsing
		CFPswi		Switch
		CFPtok		Parse an expected token
		CFPtxt		Parse text to end of line
		CFPuqs		Parse unquoted string

*/


#include "stdio.h"			/* Standard system defs */
#include "comnd.h"			/* COMND interface definitions */
#include "comndi.h"			/* COMND internal definitions */


/* External routines */


/* External data */

extern	int	CMDbel;			/* Beep request flag */

/* Internal (public) routines */



/* Internal (public) data */


/* Local (static) data */

static	char	*Numhlp[] = {		/* help texts for _CMNUM */
			"binary number",
			"number in base 3",
			"number in base 4",
			"number in base 5",
			"number in base 6",
			"number in base 7",
			"octal number",
			"number in base 9",
			"decimal number",
			"number in base 11",
			"number in base 12",
			"number in base 13",
			"number in base 14",
			"number in base 15",
			"hexadecimal number"
				};
/*

*//* CFPnum (CSBptr, CFBptr, ccptr)

	Function parse for type=_CMNUM, number.

This routine attempts a parse of a number thought to be next in the
command stream.  CFB_DAT of the command function block contains the
radix to use, if zero then 10 is assumed.

Accepts :

	CSBptr		Address of command state block
	CFBptr		Address of command function block
	ccptr		Address of CC table (where appropriate)


Returns :

	<value>		Parse status, _CPxxx as defined in comndi.h.

*/

CFPnum (CSBptr, CFBptr, ccptr)

CSB		*CSBptr;		/* Addr of command state block */
CFB		*CFBptr;		/* Addr of command function block */
WORD		*ccptr;			/* Addr of CC table */

{
IND	int	ec;			/* End char (action char, if any) */
IND	BYTE	c;			/* A character */
IND	int	cix;			/* Relative parse index */
IND	int	icix;			/* Initial relative parse index */
IND	int	base;			/* radix */
IND	int	val;			/* Value */
IND	int	sign;			/* Sign (-1, 0, 1) */

base = CFBptr -> CFB_DAT;		/* Get base */
if (base == 0)				/* If zero */
    base = 10;				/*  use decimal */
if ((base < 2) || (base > 16))		/* If invalid base */
    {
    CSBptr -> CSB_RCD = _CRBAS;		/* Invalid base */
    return (_CPABT);			/* Abort */
    }

/* Get the number. */

icix = cix = CMDspc (CSBptr);		/* Init command index */
val = 0;				/* Value is zero */
sign = 0;				/* No sign seen */

while (TRUE)				/* Collect... */
    {
    ec = CMDgcc (CSBptr, cix);		/* Get next character */
    if (sign == 0)			/* Check sign ? */
	{
	if (c == '-')
	    sign = -1;
	if (c == '+')
	    sign = 1;
	if (sign != 0)
	    {
	    cix++;
	    continue;
	    }
	}

    c = ec-'0';				/* Get binary equivalent */
    if (c > 9)				/* If it wasn't decimal */
	c = (toupper(ec)-'A')+10;	/*  use alpha set */
    if ((c < 0) || (c >= base))		/* If invalid digit */
	break;				/*  then be done */

    if (base == 2)			/* optimize for shifts */
	val <<= 1;
    else if (base == 4)
	val <<= 2;
    else if (base == 8)
	val <<= 3;
    else if (base == 16)
	val <<= 4;
    else
	val = val*base;
    val = val + c;			/* Add in new value */
    cix++;				/* Bump counter */
    }

/* Here on no more digits */

if (sign < 0)				/* If negative */
    val = -val;

CSBptr -> CSB_RVL._INT = val;		/* Stick it in there, in case */

switch (ec)				/* Dispatch on final char */
    {
    case _CCCMP:			/* Complete? */
	if (cix)			/* If anything */
	    {
	    CMDcpl (CSBptr, " ");	/*  add a space */
	    return (_CPCPE);
	    }
	CMDbel++;			/* Indicate our desire to beep */
	return (_CPAGN);		/* Might work.. */

    case _CCINC:			/* Incomplete? */
	return (_CPAGN);		/* Ok so far, try when you get more */

    case _CCHLP:			/* Help... */
	CMDhlp (CFBptr, Numhlp[base-2]);
	return (_CPGVH);		/* Gave help */

    default:				/* Anything else */
	if (cix > icix)			/* If parsed anything */
	    {
	    CMDcpt (CSBptr, cix);	/* Parse is ok */
	    return (_CPSUCC);		/* Return it. */
	    }
	return (_CPNOP);		/* Otherwise not */
    }
}
/*

*//* CFPswi (CSBptr, CFBptr, ccptr)

	Function parse for type=_CMSWI, switch.

This routine performs a parse for a switch next in the command
line.  This is like KWD, except that a slash must come first.

Accepts :

	CSBptr		Address of command state block
	CFBptr		Address of command function block
	ccptr		Address of CC table (where appropriate)


Returns :

	<value>		Parse status, _CPxxx as defined in comndi.h.

*/

CFPswi (CSBptr, CFBptr, ccptr)

CSB		*CSBptr;		/* Addr of command state block */
CFB		*CFBptr;		/* Addr of command function block */
WORD		*ccptr;			/* Addr of CC table */

{
CSBptr -> CSB_RCD = _CRIFC;		/* Set invalid function (NYI) */
return (_CPABT);			/* Abort now. */
}
/*

*//* CFPtok (CSBptr, CFBptr, ccptr)

	Function parse for type=_CMTOK, token

This function performs an exact match on an expected token

Accepts :

	CSBptr		Address of command state block
	CFBptr		Address of command function block
	ccptr		Address of CC table (where appropriate)


Returns :

	<value>		Parse status, _CPxxx as defined in comndi.h.

*/

CFPtok (CSBptr, CFBptr, ccptr)

CSB		*CSBptr;		/* Addr of command state block */
CFB		*CFBptr;		/* Addr of command function block */
WORD		*ccptr;			/* Addr of CC table */

{
IND	int	cix;			/* Command index */
IND	int	c,c1;			/* Chars */
IND	char	*tptr;			/* Token pointer (ptr to token) */


tptr = CFBptr -> CFB_DAT;		/* Get address of the token */
cix = CMDspc(CSBptr);			/* Init parse index */
while (TRUE)				/* Look at chars */
    {
    if ((c = *tptr++) == NUL)		/* If end of token passed */
	{
	CMDcpt (CSBptr, cix);		/* Checkpoint to here! */
	return (_CPSUCC);		/* We matched */
	}

    c = toupper(c);			/* Ignore case */
    c1 = CMDgcc (CSBptr, cix);		/* Get next char from cmd */
    if (c != toupper(c1))		/* If not the same */
	break;				/*  exit the loop */
    cix++;				/* Fine, skip it */
    }

/* Here on mismatch... check for specials */

switch (c1)
    {
    case _CCCMP:			/* Complete.. */
    case _CCINC:			/* or incomplete */
	return (_CPAGN);		/*  try again */

    case _CCHLP:			/* Help ? */
	if (CMDhlp (CFBptr, "token: "))
	    CMDpzs (CFBptr -> CFB_DAT);
	CMDfob();
	return (_CPGVH);		/* Say we gave help */

    default:				/* Anything else */
	return (_CPNOP);		/* No parse */
    }
}
/*

*//* CFPtxt (CSBptr, CFBptr, ccptr)

	Function parse for type=_CMTXT, text

This function parses text to the end of the line.

Accepts :

	CSBptr		Address of command state block
	CFBptr		Address of command function block
	ccptr		Address of CC table (where appropriate)


Returns :

	<value>		Parse status, _CPxxx as defined in comndi.h.

*/

CFPtxt (CSBptr, CFBptr, ccptr)

CSB		*CSBptr;		/* Addr of command state block */
CFB		*CFBptr;		/* Addr of command function block */
WORD		*ccptr;			/* Addr of CC table */

{
IND	int	cix;			/* Command index */
IND	int	c;			/* Character */
IND	char	*aptr;			/* Atom buffer pointer */

cix = 0;				/* Init parse index */
aptr = CSBptr -> CSB_ABF;		/* Point to atom buffer */

while (TRUE)				/* Get chars */
    {
    c = CMDgcc (CSBptr, cix);		/* Get next char */
    switch (c)				/* Process specials */
	{
	case _CCCMP:			/* Escape ? */
	    CMDbel++;			/* We'd beep */

	case _CCINC:			/* Incomplete */
	    return (_CPAGN);		/*  try again */

	case _CCHLP:			/* Help ? */
	    CMDhlp (CFBptr, "text, end with carriage return");
	    return (_CPGVH);		/* Gave help */

	case _CCEND:			/* End of line */
	    *aptr = NUL;		/* End the string */
	    CMDcpt (CSBptr, cix);	/* Checkpoint to here */
	    return (_CPSUCC);		/* Return success */

	default:			/* Other */
	    break;			/*  loop */
	}

    *aptr++ = c;			/* Store the char */
    cix++;				/* Bump char index */
    }
}
/*

*//* CFPuqs (CSBptr, CFBptr, ccptr)

	Function parse for type=_CMUQS, unquoted string

This function parses an unquoted string; a string delimited by
spaces or commas only.


Accepts :

	CSBptr		Address of command state block
	CFBptr		Address of command function block
	ccptr		Address of CC table (where appropriate)


Returns :

	<value>		Parse status, _CPxxx as defined in comndi.h.

*/

CFPuqs (CSBptr, CFBptr, ccptr)

CSB		*CSBptr;		/* Addr of command state block */
CFB		*CFBptr;		/* Addr of command function block */
WORD		*ccptr;			/* Addr of CC table */

{
IND	int	cix;			/* Command index */
IND	int	ec;			/* Character */
IND	int	aptr;			/* Atom buffer pointer */

cix = CMDcab (CSBptr, 			/* Collect atom buffer */
		CSBptr -> CSB_ABF,	/*  where to put it */
		CSBptr -> CSB_ASZ,	/*  how big it is */
		ccptr,			/* Char table */
		&ec,			/* What the end char is */
		CMDspc(CSBptr));	/* Initial parse index */

switch (ec)				/* Process according to final */
    {
    case _CCINV:			/* Invalid delimiter */
	return (_CPNOP);		/* No Parse */

    case _CCCMP:			/* Escape ? */
	CMDcpl (CSBptr, " ");		/* Add a space */
	return (_CPCPE);		/* Flag completed with escape */

    case _CCINC:			/* Incomplete */
	return (_CPAGN);		/*  try again */

    case _CCHLP:			/* Help ? */
	CMDhlp (CFBptr, "unquoted string");
	return (_CPGVH);		/* Gave help */

    default:				/* Completed atom */
	CMDcpt (CSBptr, cix);		/* Checkpoint to here */
	return (_CPSUCC);		/* Return success */
	}
}
