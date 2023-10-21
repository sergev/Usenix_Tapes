/*	cmdpf1.c	COMND module; COMND parsing routines, set # 1

	Copyright (C) 1985 Mark E. Mallett

	Permission is hereby granted to distribute this file indiscriminately.

	This file contains parsing routines for various individual
function codes, as well as the function-code decoder (?).

Edit history

When	Who	What
------	---	--------------------------------
84xxxx	MEM	Create file.


	Routines included are:

		CMDpf		Function parsing controller.
		CFPini		Initialize parse
		CFPnoi		Noise word (guide word) parsing
		CFPcfm		Confirm (carriage return)
		CFPkey		Parse keyword from table of keywords
		CFPgky		General keyword parser
		CFPktf		Keyword table fetch routine
*/


#include "stdio.h"			/* Standard system defs */
#include "comnd.h"			/* COMND interface definitions */
#include "comndi.h"			/* COMND internal definitions */


/* External/forward routines */

extern	int	CFPcfm();		/* Confirm */
extern	int	CFPdat();		/* Date and Time */
extern	int	CFPgsk();		/* General Storage Keyword */
extern	int	CFPini();		/* Initialize parse */
extern	int	CFPkey();		/* Keyword parse */
extern	char	**CFPktf();		/* Keyword fetch routine */
extern	int	CFPnoi();		/* Noise words (guide string) */
extern	int	CFPnum();		/* Number */
extern	int	CFPswi();		/* Switch */
extern	int	CFPtok();		/* Token */
extern	int	CFPtxt();		/* Text to end of line */
extern	int	CFPuqs();		/* Unquoted string */

/* External data */

extern	int	CMDbel;			/* Beep flag */

/* Internal (public) routines */



/* Internal (public) data */


/* Local (static) data */



static	WORD	CCkey[] = {		/* CC for keywords */
			0x0000,		/* ^@ ^A ^B ^C ^D ^E ^F ^G */
			0x3000,		/* ^H ^I ^J ^K ^L ^M ^N ^O */
			0x0000,		/* ^P ^Q ^R ^S ^T ^U ^V ^W */
			0x0000,		/* ^X ^Y ^Z ^[ ^\ ^] ^^ ^_ */
			0xC000,		/* sp  !  "  #  $  %  &  ' */
			0x0023,		/*  (  )  *  +  ,  -  .  / */
			0x5555,		/*  0  1  2  3  4  5  6  7 */
			0x5000,		/*  8  9  :  ;  <  =  >  ? */
			0x2AAA,		/*  @  A  B  C  D  E  F  G */
			0xAAAA,		/*  H  I  J  K  L  M  N  O */
			0xAAAA,		/*  P  Q  R  S  T  U  V  W */
			0xA802,		/*  X  Y  Z  [  \  ]  ^  _ */
			0x2AAA,		/*  `  a  b  c  d  e  f  g */
			0xAAAA,		/*  h  i  j  k  l  m  n  o */
			0xAAAA,		/*  p  q  r  s  t  u  v  w */
			0xA800		/*  x  y  z  {  |  }  ~ dl */
			  };

static	WORD	CCuqs[] = {		/* CC for unquoted string */
			0xFFFF,		/* ^@ ^A ^B ^C ^D ^E ^F ^G */
			0xFFFF,		/* ^H ^I ^J ^K ^L ^M ^N ^O */
			0xFFFF,		/* ^P ^Q ^R ^S ^T ^U ^V ^W */
			0xFCFF,		/* ^X ^Y ^Z ^[ ^\ ^] ^^ ^_ */
			0xEAAA,		/* sp  !  "  #  $  %  &  ' */
			0xAAEB,		/*  (  )  *  +  ,  -  .  / */
			0xAAAA,		/*  0  1  2  3  4  5  6  7 */
			0xAAAA,		/*  8  9  :  ;  <  =  >  ? */
			0xAAAA,		/*  @  A  B  C  D  E  F  G */
			0xAAAA,		/*  H  I  J  K  L  M  N  O */
			0xAAAA,		/*  P  Q  R  S  T  U  V  W */
			0xAAAA,		/*  X  Y  Z  [  \  ]  ^  _ */
			0xAAAA,		/*  `  a  b  c  d  e  f  g */
			0xAAAA,		/*  h  i  j  k  l  m  n  o */
			0xAAAA,		/*  p  q  r  s  t  u  v  w */
			0xAAAB		/*  x  y  z  {  |  }  ~ dl */
			  };


static	int	(*Cfptbl[])() = {	/* Command function routine table */
			CFPini,		/* Initialize parse */
			CFPkey,		/* Keyword parse */
			CFPnum,		/* Number */
			CFPnoi,		/* Noise words (guide string) */
			CFPcfm,		/* Confirm */
			CFPgsk,		/* General Storage Keyword */
			CFPswi,		/* Switch */
			CFPtxt,		/* Text to end of line */
			CFPtok,		/* Token */
			CFPuqs,		/* unquoted string */
			CFPdat		/* Date and time */
			      };

static	WORD	**Ccctbl[] = {		/* Default CC tables for each fc */
			NULL,		/* _CMINI */
			CCkey,		/* _CMKEY */
			NULL,		/* _CMNUM */
			NULL,		/* _CMNOI */
			NULL,		/* _CMCFM */
			CCkey,		/* _CMGSK */
			CCkey,		/* _CMSWI */
			NULL,		/* _CMTXT */
			NULL,		/* _CMTOK */
			CCuqs,		/* _CMUQS */
			NULL		/* _CMDTM */
				};



/*

*//* CMDpf (CSBptr, CFBptr)

	Process parse for a particular function block

This routine attempts to parse the remaining input according to
the command function block (CFBptr).  It is responsible for calling
the appropriate parse routine, and passing the result code back to
the main COMND executor.


Accepts :

	CSBptr		Address of command state block
	CFBptr		Address of command function block

Returns :

	<value>		parse result, of form _CPxxx as defined in the
			include file "comndi.h".

*/

CMDpf (CSBptr, CFBptr)

CSB		*CSBptr;		/* Addr of command state block */
CFB		*CFBptr;		/* Addr of command function block */

{
IND	int		i;		/* Index */
IND	WORD		*ccptr;		/* Addr of CC table, if any */

i = CFBptr -> CFB_FNC;			/* Get function code */
if ((i < 0) || (i > _CMMAX))		/* If out of legit range */
    {
    CSBptr -> CSB_RCD = _CRIFC;		/* Set invalid function code status */
    return (_CPABT);			/* Abort, right away. */
    }

ccptr = Ccctbl[i];			/* Get default CC table */
if (ccptr)				/* If any (if meaningful here) */
    if (CFBptr->CFB_FLG & _CFCC)	/* If user-supplied bit set */
	ccptr = CFBptr -> CFB_CC;	/*   then use his/hers! */
return ((*Cfptbl[i])(CSBptr, CFBptr, ccptr));	/* Process it */
}
/*

*//* CFBini (CSBptr, CFBptr, ccptr)

	Function parse for type=_CMINI, initialize the parse


This routine is called to initialize a parse for a line.

Accepts :

	CSBptr		Address of command state block
	CFBptr		Address of command function block
	ccptr		Address of CC table (where appropriate)


Returns :

	<value>		Parse status, _CPxxx as defined in comndi.h.

*/

CFPini (CSBptr, CFBptr, ccptr)

CSB		*CSBptr;		/* Addr of command state block */
CFB		*CFBptr;		/* Addr of command function block */
WORD		*ccptr;			/* Addr of CC table */

{
CSBptr -> CSB_FLN = -1;			/* No text filled, yet */
					/* -1 enables ^H recovery */
return (_CPSUCC);			/* This parse MATCHED, by jove */
}
/*

*//* CFPnoi (CSBptr, CFBptr, ccptr)

	Function parse for type=_CMNOI, noise words (guide string)

This routine attempts a parse of a particular guide string.  The
guide words are given in the CFB_DEF pointer area, and are matched if
enclosed in parentheses (the parens are NOT included in the supplied
string).

Accepts :

	CSBptr		Address of command state block
	CFBptr		Address of command function block
	ccptr		Address of CC table (where appropriate)


Returns :

	<value>		Parse status, _CPxxx as defined in comndi.h.

*/

CFPnoi (CSBptr, CFBptr, ccptr)

CSB		*CSBptr;		/* Addr of command state block */
CFB		*CFBptr;		/* Addr of command function block */
WORD		*ccptr;			/* Addr of CC table */

{
IND	char	*dptr;			/* Default string pointer */
IND	int	cix;			/* Command index */
IND	int	c,c1;			/* Character */

cix = CMDspc (CSBptr);			/* Skip spaces */
dptr = CFBptr -> CFB_DEF;		/* Get addr of default string */
if (CSBptr -> CSB_RFL & _CFPFE)		/* If previous ended in escape */
    c = _CCCMP;				/* Flag completion wanted */
else					/* Otherwise */
    c = CMDgcc(CSBptr, cix++);		/*  get next char */

if (c == _CCCMP)			/* If completion wanted at start */
    {
    CMDcpl (CSBptr, "(");		/* Complete... opening paren */
    CMDcpl (CSBptr, dptr);		/* Complete... guide words */
    CMDcpl (CSBptr, ") ");		/* Complete... closing paren */
    return (_CPCPE);			/* Return successful parse */
    }

if (c == _CCINC)			/* If incomplete */
    return (_CPAGN);			/*  try again */

if (c != '(')				/* If not open paren */
    return (_CPSUCC);			/*  guide words are optional */

while (TRUE)				/* Loop on chars in input */
    {
    while (TRUE)			/* Get non-blank char from input */
	{
	c = CMDgcc (CSBptr, cix++);
	if ((c != ' ') && (c != '	'))
	    break;
	}

    while (TRUE)			/* Get non-blank char from default */
	{
	c1 = *dptr++;
	if ((c1 != ' ') && (c1 != '	'))
	    break;
	}

    if ((c == _CCCMP)			/* completion ? */
     || (c == _CCHLP)			/* Give help ? */
     || (c == _CCEND)			/* End? */
      )
	break;				/*  quit the loop */

    c = toupper(c);			/* Compare in same case */
    c1 = toupper(c1);			/*     .     */
    if (c1 != c)			/* If not the same */
	break;				/*  then leave this loop. */
    }

/* Found non-match, or special request */

if (c == _CCCMP)			/* If completion wanted, */
    {
    CMDcpl (CSBptr, --dptr);		/* Complete with rest of string */
    CMDcpl (CSBptr, ") ");		/* Complete with closing paren */
    return (_CPCPE);			/* Return success */
    }

if (c == _CCHLP)			/* Give help ? */
    {
    if (CMDhlp (CFBptr, "guide string: ("))
	{
	CMDpzs (CFBptr -> CFB_DEF);	/* Print string */
	CMDpzs (")");
	CMDfob();			/* Make sure it is seen */
	}
    return (_CPGVH);			/* Indicate we gave help */
    }

if (c == _CCEND)			/* If end of input */
    return (_CPNOP);			/*  no parse */

if (c == _CCINC)			/* If incomplete */
    return (_CPAGN);			/*  try again */

if ((c1 == NUL) && (c == ')'))		/* If end of guideword */
    {
    CMDcpt (CSBptr, cix);		/* Set parse checkpoint */
    return (_CPSUCC);			/* Return success! */
    }

return (_CPNOP);			/* Sorry, no parse */
}
/*

*//* CFPcfm (CSBptr, CFBptr, ccptr)

	Function parse for type=_CMCFM, confirm with carriage return


Accepts :

	CSBptr		Address of command state block
	CFBptr		Address of command function block
	ccptr		Address of CC table (where appropriate)


Returns :

	<value>		Parse status, _CPxxx as defined in comndi.h.

*/

CFPcfm (CSBptr, CFBptr, ccptr)

CSB		*CSBptr;		/* Addr of command state block */
CFB		*CFBptr;		/* Addr of command function block */
WORD		*ccptr;			/* Addr of CC table */

{
IND	int	c;			/* Character */

c = CMDgcc (CSBptr, CMDspc(CSBptr));	/* Get the character that's next */
switch (c)				/* Process it */
    {
    case _CCINC:			/* Incomplete */
	return (_CPAGN);		/*  just continue parsing */

    case _CCCMP:			/* Completion... */
	CMDbel++;			/* Indicate our desire to beep */
	return (_CPAGN);		/* Try again. */

    case _CCHLP:			/* Help */
	CMDhlp (CFBptr, "confirm with carriage return");
	return (_CPGVH);		/* Indicate help given */

    case _CCEND:			/* End? */
	return (_CPSUCC);		/*  we was successful */

    default:				/* Anything else */
	return (_CPNOP);		/*  no parse */
    }
}
/*
*//* CFPkey (CSBptr, CFBptr, ccptr)

	Function parse for type=_CMKEY, keyword parse.

This routine handles parsing for type _CMKEY, a table of keywords.
Basically, it hands off to CFPgky, the general keyword parser.

Accepts :

	CSBptr		Address of command state block
	CFBptr		Address of command function block
	ccptr		Address of CC table (where appropriate)

Returns :

	<value>		Parse result, one of the _CPxxx values, as defined
			in "comndi.h"

*/

CFPkey (CSBptr, CFBptr, ccptr)

CSB		*CSBptr;		/* Addr of command state block */
CFB		*CFBptr;		/* Addr of command function block */
WORD		*ccptr;			/* Addr of CC table */

{
return (CFPgky (CSBptr, CFBptr, CFBptr -> CFB_DAT, CFPktf, ccptr));
}
/*
*//* CFPgsk (CSBptr, CFBptr, ccptr)

	Function parse for type=_CMGSK, general keyword.

This routine handles parsing for type _CMGSK, where a method of fetching
each candidate keyword is provided.  CFB_DAT contains the address of a
CGK block, which in turns specifies a routine to call to get a pointer
to a keyword's pointer when given a specified base (also in CGK) and
the address of the previous pointer.

This routine calls CFBgky, the general keyword parser.


Accepts :

	CSBptr		Address of command state block
	CFBptr		Address of command function block
	ccptr		Address of CC table (where appropriate)

Returns :

	<value>		Parse result, one of the _CPxxx values, as defined
			in "comndi.h"

*/

CFPgsk (CSBptr, CFBptr, ccptr)

CSB		*CSBptr;		/* Addr of command state block */
CFB		*CFBptr;		/* Addr of command function block */
WORD		*ccptr;			/* Addr of CC table */

{
IND	CGK	*CGKptr;		/* Points to COMND Genl Kwd block */

CGKptr = (CGK *) CFBptr -> CFB_DAT;	/* Get addr of CGK block */
return (CFPgky (CSBptr, CFBptr, CGKptr->CGK_BAS, CGKptr->CGK_KFR, ccptr));
}
/*

*//* CFPgky (CSBptr, CFBptr, base, kfr, ccptr)

	General keyword parse.

This routine handles parsing for strings, using a general keyword fetch
mechanism.

Accepts :

	CSBptr		Address of command state block
	CFBptr		Address of command function block
	base		A base variable to pass to the keyword fetch
			  routine (we don't need to know what it is).
	kfr		Address of the routine to fetch strings.
	ccptr		Addr of CC table to use.


Returns :

	<value>		Parse status, _CPxxx as defined in comndi.h.

*/

CFPgky (CSBptr, CFBptr, base, kfr, ccptr)

CSB		*CSBptr;		/* Addr of command state block */
CFB		*CFBptr;		/* Addr of command function block */
char		*base;			/* Some base pointer */
char		**(*kfr)();		/* Addr of routine to fetch ptr
					   to next string */
WORD		*ccptr;			/* Addr of CC table */

{
IND	int	ec;			/* End char (action char, if any) */
IND	BYTE	c,c1;			/* A character */
IND	char	**entptr;		/* Entry pointer pointer */
IND	char	*entstr;		/* Entry string pointer */
IND	char	**matptr,**mat1;	/* Two previous matches */
IND	char	*matend;		/* End of matched string */
IND	BYTE	*wrdptr;		/* Pointer to the word */
IND	int	cix;			/* Command index */
IND	int	hcol;			/* Help display column # */
IND	char	*hptr;			/* start of help pointer */
IND	int	i;			/* Scratch */

hptr = NULL;				/* Init start-of-help pointer */
if (CFBptr -> CFB_FLG & _CFHPP)		/* If user-supplied help */
    hptr = "";				/* don't US say "keyword" */
if (hptr == NULL)			/* Now if no start-of-help */
    hptr = "keyword, ";			/*  use our own */


/* Collect the command string into the atom buffer, for starts. */

cix = CMDcab (CSBptr, 			/* Collect atom buffer */
		CSBptr -> CSB_ABF,	/*  where to put it */
		CSBptr -> CSB_ASZ,	/*  how big it is */
		ccptr,			/* Char table */
		&ec,			/* What the end char is */
		CMDspc(CSBptr));	/* Initial parse index */

if (ec == _CCINV)			/* If invalid delimiter */
    return (_CPNOP);			/*  return no-parse */

/* Now attempt to match the string in the atom buffer according to
   strings given. */

entptr = NULL;				/* Init entry pointer */
matptr = NULL;				/* No match yet */
mat1 = NULL;				/* No first match */

while ((entptr = (*kfr)(base, entptr)) != NULL)
					/* End of table is a null entry */
    {
    entstr = *entptr;			/* Get pointer to string */
    wrdptr = CSBptr -> CSB_ABF;		/* Get pointer to atom buffer */
    while (TRUE)			/* Scan to end of word */
	{
	c = *wrdptr++;
	c = toupper(c);			/* Get its uppercase */
	c1 = *entstr++;			/* Get next char from table ent */
	if (c == NUL)			/* If end of our string */
	    break;			/*  go check more */
	if (c != toupper(c1))		/* if mismatch */
	    break;			/*  be done */
	}

    if (c == NUL)			/* If end of word, we matched */
	{
	if (ec == _CCHLP)		/* Give help ? */
	    {
	    if (!matptr)		/* No other match... */
		matptr = entptr;	/* Remember it */
	    else			/* Must print as help */
		{
		if (!mat1)		/* If must print help message */
		    {
		    mat1 = matptr;
		    if (CMDhlp (CFBptr, hptr))
			{
			CMDpzs ("one of the following:\n");
		 	CMDpzs (*matptr);
			hcol = strlen (*matptr);
			}
		    }
		i = hcol;
		hcol = (hcol+16)&(-16);		/* Next tab stop */
		if (!(CFBptr -> CFB_FLG & _CFSDH)) /* If not suppressed */
		    {
		    if (hcol + strlen(*entptr) > 79)
			{
			hcol = 0;
			CMDpoc ('\n');
			}
		    else
			{
			CMDpoc ('\011');
			if (hcol-i > 8)
			    CMDpoc ('\011');
			}
		    CMDpzs (*entptr);
		    hcol += strlen(*entptr);
		    }
		}
	    continue;
	    }

	if (c1 == NUL)			/* If also end of entry , real match */
	    {
	    matptr = entptr;		/* Set match to this entry */
	    matend = entstr-1;		/* Remember completion ptr */
	    mat1 = NULL;		/* Disambiguate */
	    break;			/* Go return it. */
	    }

	else
	    {
	    if (matptr)			/* Any other ambig match ? */
		mat1 = matptr;		/*  flag it */
	    else			/* Otherwise */
		{
		matptr = entptr;	/*  remember this one */
		matend = entstr-1;	/* Point to rest-of-string */
		}
	    }
	}
    }

/* Here, scanned table. */

if (mat1 && matptr)			/* If multiply matched */
    {
    switch (ec)				/* Process according to final char */
	{
	case _CCHLP:			/* Gave help already? */
	    CMDfob();			/* Show what we did */
	    return (_CPGVH);		/* Return that status */

	case _CCCMP:			/* Complete */
	    CMDbel++;			/* Say we would beep */

	case _CCINC:			/* Incomplete input? */
	    return (_CPAGN);		/* Try again */

	case _CCEND:			/* End of line */
	    return (_CPNOP);		/* No match. */

	default:
	    return (_CPNOP);		/* Ambiguous with some other junk,
					   give failure.  */
	}
    }

if (matptr)				/* Any match at all ? */
    {
    CSBptr->CSB_RVL._ADR = matptr;	/* Pass back the result. */
    switch (ec)				/* Process according to action char */
	{
	case _CCHLP:			/* Give help */
	    if (CMDhlp (CFBptr, hptr))
		{
		CMDpzs ("only choice is: ");
		CMDpzs (*matptr);
		}
	    CMDfob();
	    return (_CPGVH);		/* Gave help */

	case _CCCMP:			/* Complete */
	    CMDcpl (CSBptr, matend);	/* Complete with end of match */
	    CMDcpl (CSBptr, " ");	/* Add space */
	    return (_CPCPE);		/* Completed with escape */

	case _CCINC:			/* Incomplete */
	    return (_CPAGN);		/* Try again with more */

	default:			/* Anything else */
	    if (*CSBptr -> CSB_ABF == NUL)
		return (_CPNOP);	/* If null entry, no parse */
	    CMDcpt (CSBptr, cix);	/* Checkpoint parse to here */
	    return (_CPSUCC);		/* Success */
	}
    }

else					/* No match at all */
    {
    switch (ec)				/* Depends on final char */
	{
	default:			/* Anything */
	    return (_CPNOP);		/*  no parse */
	}
    }
}
/*

*//* CFPktf (base, str)

	Returns address of next string in a table of pointers to strings

This is a general keyword address fetcher for use in the _CMKEY parse
function.  It accepts the address of a string ptr, and returns the address
of the next one.


Accepts :

	base		Address of the base of the table
	str		Address of a pointer to the previous string, or
			NULL to indicate fetching the first string.


Note: this routine deals in address of POINTERS because it points to
the table entry, in general, where the table is composed of pointers
to strings.  Get it?

Returns :

	<value>		Address of the next string, or
			NULL if no more.

*/

char **CFPktf (base, str)

char		*base[];		/* Table address */
char		**str;			/* Addr of string pointer */

{
if (str == NULL)			/* If pointer is NULL */
    {
    if (*base == NULL)			/* If empty table */
	return (NULL);			/*  return end */
    return (base);			/* Return base address */
    }

if (*++str == NULL)			/* If next entry is NULL */
    return (NULL);			/*  then return END */
else					/* Otherwise */
    return (str);			/*  return the pointer */
}
