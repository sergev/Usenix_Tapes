/*	comnd.c		COMND module; Main module.

	Copyright (C) 1985 Mark E. Mallett

	Permission is hereby granted to distribute this file indiscriminately.

	COMND.C, along with its associated files, contains routines which
	provide command parsing facilities of the TOPS-20 style, with
	command recognition, incremental help, and indirection.

Edit history

When	Who	What
------	---	--------------------------------
84xxxx	MEM	Create file.


	Routines included:

		COMND		The "user" call to parsing.
		COMNDr		General support COMND call
		CMDcab		Collect atom buffer
		CMDspc		Skip leading spaces
		CMDgcc		Get nth unparsed character
		CMDcpt		Checkpoint parsed area
		CMDcpl		Perform command completion
		CMDhlp		Perform common help
		CMDfill		Fill the command line.

*/


#include "stdio.h"			/* Standard system defs */
#include "comnd.h"			/* COMND interface definitions */
#include "comndi.h"			/* COMND internal definitions */


/* External routines */

extern		CMDptc();

/* External data */


/* Internal (public) routines */



/* Internal (public) data */

	int	CMDbel = {0};		/* Beep count */
	int	CMDgvh = {0};		/* Gave-help count */
	int	(*Cocrtc)() = {CMDptc};	/* Output char routine to call */


/* Local (static) data */


static	int	Echofl = {TRUE};	/* Echo on or off */
/*

*//* COMND (CSBptr, CFBptr)

	The general command call

This routine processess a request to parse an element of a command.

Accepts :

	CSBptr		Address of Command State Block
	CFBptr		Address of Command Function Block


Returns :

	<value>		One of result codes _CRxxx as defined in
			  the comnd.h include file

*/

int COMND (CSBptr, CFBptr)

CSB		*CSBptr;		/* Addr of command state block */
CFB		*CFBptr;		/* Addr of command function block */

{
return (COMNDr (CSBptr, CFBptr, 0));	/* Call comnd in non-support mode */
}
/*

*//* COMNDr (CSBptr, CFBptr, sflg)

	General COMND support call

This routine is the general COMND call, for use by extern or internal
(support) routines.

Accepts :

	CSBptr		Address of Command State Block
	CFBptr		Address of Command Function Block
	sflg		0 if general call
			1 if first through n-1st support call
			2 if final support call


Returns :

	<value>		One of result codes _CRxxx as defined in
			  the comnd.h include file



Notes:

	sflg may be set in order to make multiple calls to COMNDr appear
function the same way as a single call to COMND with chained CFBs.  That
is, each successive call to COMNDr in support mode passes a new CFB list
which is a logical extention of the current list.

	Please note that routines setting sflg will be returned internal
parse result codes (_CPxxx) rather than the usual external result codes
(_CRxxx), in order to allow full handling of all conditions.

*/

int COMNDr (CSBptr, CFBptr, sflg)

CSB		*CSBptr;		/* Addr of command state block */
CFB		*CFBptr;		/* Addr of command function block */
int		sflg;			/* Support-mode flag */

{
IND	int	agn;			/* Try-again flag */
IND	int	parse;			/* Parse result */
IND	int	i;			/* Scratch */
	CFB	*CFBcur;		/* Current CFB */
IND	int	c,c1;			/* Char */
IND	char	*sptr;			/* String pointer */
	int	cix;			/* Char index */

CSBptr -> CSB_RFL &= (~_CFPFE);		/* Clear prev-was-escape-completed */
if (CSBptr -> CSB_RFL & _CFESC)		/* If last WAS esc completed */
    CSBptr -> CSB_RFL |= _CFPFE;	/*  then set it */

while (TRUE)				/* Loop until we have something
					   to return to our gentle caller */
    {
    if (!sflg)				/* Reset things if not support mode */
	{
	CMDbel = 0;			/* Set beep counter to 0 */
	CMDgvh = 0;			/* Set gave-help to 0 */
	agn = FALSE;			/* Try-again to FALSE */
	}
    Echofl = !(CSBptr -> CSB_PFL & _CFNEC);
					/* Set echo on or off as desired */
    CFBcur = CFBptr;			/* Start at beginning */
    if (CSBptr -> CSB_FLN == -1)	/* If initial */
	{
	CMDfill (CSBptr, CFBptr);	/* Collect input */
	continue;			/*  and skip to end of while loop */
	}

    if (!sflg)				/* If not support mode */
	{
	if (CFBcur -> CFB_FLG & _CFDPP)	/* Check out default if any */
	    if (sptr = CFBcur -> CFB_DEF) /* (must be non-null ptr) */
		{
		cix = CMDspc (CSBptr);	/* Get inx after spaces */
		c = CMDgcc(CSBptr, cix);
					/* Get first non-space non-tab */
		cix = cix + CSBptr -> CSB_PRS; /* Get real index */
		c1 = CSBptr -> CSB_BUF[cix]; /* Get real character there */
		if ((c == _CCCMP) || (c == _CCEND))
		    {			/* If ok to default */
		    if (c == _CCEND)	/* If it was return */
			if ((cix) && (CSBptr->CSB_BUF[cix-1] != ' '))
			    CSBptr->CSB_BUF[cix++] = ' ';
					/* make sure space separation. */
		    strcpy (&CSBptr -> CSB_BUF[cix], sptr);
		    CSBptr -> CSB_FLN = cix = cix+strlen(sptr);
		    if (c == _CCCMP)	/* If completion */
			{
			CMDpzs (sptr);	/* Show it */
			CMDfob();
			}
		    CSBptr -> CSB_BUF[cix] = c1;
		    }
		}
	}


    CSBptr -> CSB_RFL &= (~_CFESC);	/* Clear completed-with-escape */
    while (CFBcur)			/* Go through... */
	{
	CSBptr -> CSB_CFB = CFBcur;	/* Remember which CFB we're at */
	parse = CMDpf(CSBptr, CFBcur);
					/* Try parse on this function */
	switch (parse)			/* Process the completion code */
	    {
	    case _CPCPE:		/* Completed with escape */
		CSBptr -> CSB_RFL = _CFESC; /* Indicate to caller */
		if (sflg)		/* If support... */
		    return (_CPCPE);	/*  return appropriate code */
		return (_CROK);		/* Return OK */


	    case _CPSUCC:		/* Successfull */
		if (sflg)		/* If support mode */
		    return (_CPSUCC);	/*  return success indication */
		return (_CROK);		/*  Return OK */

	    case _CPABT:		/* Abort */
		if (sflg)		/* If supportmode */
		    return (_CPABT);	/*  return the abort flag */
		return (CSBptr -> CSB_RCD); /* Pass back the return code */

	    case _CPGVH:		/* Gave help */
		CMDgvh++;		/* Count it and act like _CPAGN */

	    case _CPAGN:		/* Try again... */
		agn = TRUE;		/*  set to try again */
	    }

	CFBcur = CFBcur -> CFB_CFB;	/* Get next one */
	}

/* Gone through all the function blocks.. take appropriate result action */

    if (CSBptr -> CSB_PRS == 0)		/* If nothing parsed */
	{
	cix = CMDspc (CSBptr);		/* Skip spaces */
	if (CMDgcc (CSBptr, cix) == _CCEND) /* See if no input */
	    {
	    CSBptr -> CSB_FLN = -1;	/* Indicate reprompt */
	    continue;			/* Go to end of the while loop */
	    }
	}

    if (!agn)				/* If not to try again */
	{
	if (sflg)			/* If support mode */
	    return (_CPNOP);		/*  return internal code */
	return (_CRNOP);		/* Return no-parse */
	}

    if (sflg)				/* If supportmode */
	return (_CPAGN);		/*  return that we ought try agn */

    if (CMDgvh)				/* If gave help... */
	{
	CMDpoc ('\n');			/* give CR */
	CMDpzs (CSBptr -> CSB_PMT);	/* Output prompt */
	for (i = 0; i < CSBptr -> CSB_FLN; i++)
	    CMDpoc (CSBptr -> CSB_BUF[i]); /* Output text typed */
	CMDfob();
	CMDbel = 0;			/* Don't beep */
	}

    if (CMDbel)				/* If wanted to beep */
	{
	CMDpoc ('\007');			/* Make a beep */
	CMDfob();
	}

    CMDfill (CSBptr, CFBptr);		/* do user editing */
    if (CSBptr -> CSB_RFL & _CFRPT)	/* If reparse indicated */
	{
	CSBptr -> CSB_PRS = 0;		/* Reset the parse pointer */
	if (CSBptr -> CSB_RSB)		/* If any setjmp buffer */
	    longjmp (CSBptr -> CSB_RSB, 1); /* Execute the longjump */
	else				/* Otherwise */
	    return (_CRRPT);		/* Return re-parse */
	}
    }
}
/*

*//* CMDcab (CSBptr, buf, buflen, cctbl, ecptr, icix)

	Collect Atom Buffer

This routine collects characters from the input buffer into an atom
buffer as specified by the caller.  It makes use of a character
characteristic table, as described elsewhere.


Accepts :

	CSBptr		Address of command state block
	buf		Address of buffer to store data into
	buflen		Length of buffer
	cctbl		Address of character/characteristic table
	ecptr		Where to store the ending character value
	icix		Initial relative parse index

Returns :

	<value>		New character index relative to parse pointer, for
			use in checkpointing if the parse is successful
			at this point.
	*ecptr		Value of character which caused the end of atom
			collection.

*/

CMDcab (CSBptr, buf, buflen, cctbl, ecptr, icix)

CSB		*CSBptr;		/* Addr of command state block */
char		*buf;			/* Addr of buffer */
int		buflen;			/* Length of buffer */
WORD		*cctbl;			/* CC table */
int		*ecptr;			/* Where to put ending char */
int		icix;			/* Initial cix */

{
IND	int	c;			/* Character */
IND	int	cc;			/* CharChar value */
IND	int	cix;			/* Relative parse index */
IND	int	binx;			/* Buffer index */

cix = icix;				/* Init relative index */
binx = 0;				/* Init buffer index */
while (TRUE)				/* Get characters */
    {
    c = CMDgcc (CSBptr, cix);		/* Get char */
    if ((c & 0xff00) != 0)		/* If special */
	break;				/*  then we done. */

    cc = (cctbl[c/8]>>((7-(c%8))*2))&0x3;	/* Get cc value */
    if ((cc == 0)			/* Invalid char */
     || ((cc == 1) && (binx == 0)))	/*  or invalid first char */
	{
	c = _CCINV;			/* Return invalid char */
	break;				/* Go return */
	}

    if (cc == 3)			/* Break character */
	break;				/*  then break. */

    buf[binx++] = c;			/* Store the char */
    cix++;				/* Bump the parse index */
    }

buf[binx] = NUL;			/* NUL-terminate the buffer */
*ecptr = c;				/* Pass the ending char up */
return (cix);				/* Return the index. */
}
/*

*//* CMDspc (CSBptr)

	Parse past spaces

This routine skips past spaces which are after the current parse point,
and returns the relative parse index of the first non-space non-tab
character.


Accepts :

	CSBptr		Address of command state block


Returns :

	<value>		Relative parse index of first non-space non-tab char.

*/

CMDspc (CSBptr)

CSB		*CSBptr;		/* Addr of command state block */

{
IND	int	c;			/* Character */
IND	int	cix;			/* Command index */

cix = 0;				/* Set command index */
while (TRUE)				/* Skip spaces/tabs */
    {
    c = CMDgcc (CSBptr, cix);		/* Get char */
    if ((c != ' ') && (c != '\011'))
	break;				/* Quit when got something other */
    cix++;				/*  skip em */
    }

return (cix);				/* Return the value */
}
/*

*//* CMDgcc (CSBptr, cix)

	Return unparsed character.

This routine returns the "cix"th unparsed character from the command
line indicated by the command state block at CSBptr.


Accepts :

	CSBptr		Address of command state block
	cix		character index


Returns

	<value>		character @ cix, or special code _CCxxx, as
			 defined in "comndi.h"

*/

int CMDgcc (CSBptr, cix)

CSB		*CSBptr;		/* Command state block addr */
int		cix;			/* Character index */

{
IND	int	i;			/* Scratch */
IND	int	c;			/* character */

i = CSBptr -> CSB_PRS + cix;		/* Get buffer index */
if (i > CSBptr -> CSB_FLN)		/* If beyond filled space */
    return (_CCEND);			/*  return END indicator */
c = CSBptr -> CSB_BUF[i];		/* Get character there */
if (i < CSBptr -> CSB_FLN)		/* If within filled area */
    return (c);				/*  return it. */
switch (c)				/* Dispatch on char found at end */
    {
    case '\r':				/* Return */
	return (_CCEND);		/* Return end indication */

    case '?':				/* help ? */
	return (_CCHLP);		/*  give help */

    case '\033':			/* Escape */
	return (_CCCMP);		/* Completion */

    case '\006':			/* Control-F */
	return (_CCCMP);		/* Completion */

    default:				/* Other */
	return (_CCINC);		/* Just return incomplete */
    }
}
/*

*//* CMDcpt (CSBptr, cix)

	Checkpoint parse index


This routine checkpoints the parse by remembering that "cix" new
characters have been parsed.


Accepts :

	CSBptr		Address of command state block
	cix		Character index past old parse point


Returns


*/

int CMDcpt (CSBptr, cix)

CSB		*CSBptr;		/* Command state block addr */
int		cix;			/* Character index */

{
CSBptr -> CSB_PRS += cix;		/* Set checkpoint */
}
/*

*//* CMDcpl (CSBptr, str)

	Perform completion for a command string (element)

This routine attaches a string onto that already input, thus "completing"
a portion of the input string.


Accepts :

	CSBptr		Address of command state block
	str		string to add to input string


Returns :


Notes :

	As a side-effect, the parse-pointer is set to the end of the
string, since it is assumed that the completion was performed on the
say-so of a parsing routine.


*/

CMDcpl (CSBptr, str)

CSB		*CSBptr;		/* Addr of command state block */
char		*str;			/* Addr of string to add on */

{
IND	int	i;			/* Index */
IND	int	c;			/* Character */

i = CSBptr -> CSB_FLN;			/* Get filled length */
while ((c = *str++) != NUL)		/* Swap chars */
    {
    if (i >= (CSBptr -> CSB_BSZ-1))	/* Make sure it fits */
	{
	CMDpoc ('\007');
	break;
	}
    CSBptr -> CSB_BUF[i++] = c;		/*  store it. */
    CMDpoc (c);				/* Show it */
    }
CSBptr -> CSB_BUF[i] = NUL;		/* Put marker under end of string */
CSBptr -> CSB_FLN = i;			/* Update filled pointer */
CSBptr -> CSB_PRS = i;			/* Update parse pointer */
CMDfob();				/* Make it be seen */
}
/*

*//* CMDhlp (CFBptr, str)

	Assist in beginning to give help

Accepts :

	CFBptr		Address of command function block
	str		String to print as "default".

Returns :

	<value>		TRUE if default help processing should continue
			FALSE otherwise.

*/

int CMDhlp (CFBptr, str)

CFB		*CFBptr;		/* Addr of command function block */
char		*str;			/* String to print by default */

{
IND	int	flg;			/* Flags from CFB */

flg = CFBptr -> CFB_FLG;		/* Get flags */
if ((flg & _CFHPP) || !(flg & _CFSDH))	/* If help to be given */
    {
    if (CMDgvh)				/* See if we must give "or" */
	CMDpzs ("\n or ");		/*  give it. */
    else				/* Otherwise */
	CMDpoc (' ');			/*  make sure we give a space */
    }

if (flg & _CFHPP)			/* If any user help text */
    if (CFBptr -> CFB_HLP)		/*  If any... */
	CMDpzs (CFBptr -> CFB_HLP);	/*   print it */

if (flg & _CFSDH)			/* Suppress default ? */
    {
    CMDfob ();
    return (FALSE);			/* Yup, indicate that by return */
    }
CMDpzs (str);				/* Print his string */
CMDfob();
return (TRUE);				/* Tell him to continue */
}
/*

*//* CMDfill (CSBptr, CFBptr)

	Fill the command line into the text buffer.

This routine is called to fill or to continue filling text into the command
buffer which is indicated by the command state block.  It is responsible
for rubout and other command line editing function.

Accepts :

	CSBptr		Address of the command state block

	CFBptr		Address of the first CFB in the CFB chain

Returns :



*/

CMDfill (CSBptr, CFBptr)

CSB		*CSBptr;		/* Addr of CSB */
CFB		*CFBptr;		/* Addr of first CFB */

{
IND	int		c;		/* Character */
IND	int		i;		/* Scratch */
IND	int		col;		/* Column number */
IND	int		col1;		/* Another column number */
IND	int		recena;		/* Recovery enabled */
IND	int		hlpena;		/* If help enabled */

recena = FALSE;				/* Set recovery not enabled */
if (CSBptr -> CSB_FLN == -1)		/* but if it is */
    {
    CMDpzs (CSBptr -> CSB_PMT);		/* Output the prompt */
    CMDfob();				/* Make sure it is seen */
    CSBptr -> CSB_FLN = 0;		/* Reset filled length */
    recena = TRUE;			/* Set recovery enable */
    }

hlpena = TRUE;				/* Presume help is enabled */
i = CFBptr -> CFB_FLG;			/* Get flags from CFB */
if ((i & _CFSDH) && (!(i & _CFHPP)))	/* If no help provided for */
    hlpena = FALSE;			/*  then set help not enabled */

CSBptr -> CSB_RFL = CSBptr -> CSB_RFL & (~_CFRPT);
					/* Clear bits we want cleared */
col = CMDccol (CSBptr, CSBptr -> CSB_FLN); /* See what column we're at */

while (TRUE)
    {
    c = CMDgtc();			/* Get character */

/* Check specials */

    if (recena)				/* Recovery enabled ? */
	{
	recena = FALSE;			/* Not true after this */
	if (c == '\010')		/* control-h */
	    {
	    CSBptr -> CSB_FLN = CSBptr -> CSB_PRS;
					/* Set filled length to parse inx */
	    for (i = 0; i < CSBptr -> CSB_FLN; i++)
		CMDpoc (CSBptr -> CSB_BUF[i]);
	    CMDfob();			/* Show it */
	    col = CMDccol (CSBptr, CSBptr -> CSB_FLN);
	    CSBptr -> CSB_PRS = 0;	/* Parse from zero */
	    continue;
	    }
	CSBptr -> CSB_PRS = 0;		/* Parse from zero */
	}

    if ((c == '\010')			/* Backspace */
       || (c == '\177')			/*  or rubout */
       || (c == '\027')			/* or control-W */
       || (c == '\030'))		/* or control-X */
	{
	if ((i = CSBptr -> CSB_FLN-1) >= 0)	/* If anything accumulated */
	    {
	    if (c == '\027')		/* If control-W */
		{
		for (; i > 0; i--)	/* Find beginning of "field" */
		    {
		    c = CSBptr -> CSB_BUF[i-1];
		    if ((!isalpha(c)) && (!isdigit(c)))
			break;		/* Found it. */
		    }
		}
	    else if (c == '\030')	/* If control-X */
		i = 0;			/*  erase to beginning */
	    CSBptr -> CSB_FLN = i;	/* Set new index */
	    if (i <= CSBptr -> CSB_PRS)	/* If backed to parsed area */
		CSBptr -> CSB_RFL |= _CFRPT; /* Set reparse flag */
	    if (Echofl)			/* If echoing... */
		{
		col1 = CMDccol (CSBptr, i);
					/* Find out what column to go to */
		while (col > col1)	/* Wipe out back to there */
		    {
		    CMDpzs ("\010 \010"); /* <BSP><SPC><BSP> */
		    col--;
		    }
		CMDfob();
		}
	    }
	}


    else if (c == '\022')		/* Control-r */
	{
	if (Echofl)			/* If echoing */
	    {
	    CMDpzs ("^R\n");
	    CMDpzs (CSBptr -> CSB_PMT);	/* Output prompt */
	    for (i = 0; i < CSBptr -> CSB_FLN; i++)
		CMDpoc (CSBptr -> CSB_BUF[i]);
	    CMDfob();
	    }
	}

    else if (c == '\025')		/* Control-U */
	{
	CMDpzs ("^U\n");
	CSBptr -> CSB_FLN = 0;		/* Filled length is zero */
	if (Echofl)			/* If echoing */
	    CMDpzs (CSBptr -> CSB_PMT);	/*  re-issue prompt */
	col = CMDccol (CSBptr, 0);	/* Get accurate column number */
	CMDfob();
	if (CSBptr -> CSB_PRS > 0)	/* If backed before parsed area */
	    CSBptr -> CSB_RFL |= _CFRPT; /* Set reparse flag */
	}

    else				/* Anything else */
	{
	if (CSBptr -> CSB_PFL & _CFRAI) /* Raise lowercase? */
	    c = toupper(c);		/* Make it upper */
	CSBptr -> CSB_BUF[CSBptr -> CSB_FLN] = c;
					/* Store char */

	if (c == '\r')			/* If return */
	    {
	    CMDpoc ('\n');
	    return;
	    }

	if (hlpena && (c == '?'))	/* Check for help request */
	    {
	    CMDpoc (c);
	    CMDfob();
	    return;
	    }

	if ((c == '\006')		/* control-F */
	 || (c == '\027')		/* Control-W */
	 || (c == '\033')		/* Escape */
	   )
	    return;			/* Return OK, go parse */

	if (c == '\026')		/* Control-V, literal next char */
	    {
	    c = CMDgtc();		/*  get another one */
	    CSBptr -> CSB_BUF[CSBptr -> CSB_FLN] = c;
	    }

	if (++(CSBptr -> CSB_FLN) >= (CSBptr -> CSB_BSZ-1))
	    {				/*  if full . . */
	    CMDpoc ('\007');		/*  beep at him */
	    CMDfob ();			/* Make sure it is heard */
	    CSBptr -> CSB_FLN--;	/* Don't store it. */
	    }

	else if (c == '\011')		/* tab... */
	    {
	    col = (col+8)&-8;		/* Compute next column */
	    if (Echofl)
		{
		CMDpoc (c);		/* Echo it if ok */
		CMDfob();
		}
	    }

	else if (c < ' ')		/* Control char ? */
	    {
	    col += 2;			/* Update col by 2 */
	    if (Echofl)			/* If echoing */
		{
		CMDpoc ('^');		/* Carat */
		CMDpoc (c+'@');		/* Valid char */
		CMDfob();
		}
	    }

	else				/* Any other char */
	    {
	    col++;			/* Bump col */
	    if (Echofl)			/* If echoing */
		{
		CMDpoc (c);		/* echo it */
		CMDfob();
		}
	    }
	}
    }
}
/*

*//* CMDccol (CSBptr, cix)

	Count column position

This routine counts the column position for a particular point in the
command buffer input stream.  The prompt buffer is included in this
calculation.


Accepts :

	CSBptr		Address of command state block
	cix		Character index to consider

Returns :

	<value>		Column number, zero-based.

*/

int CMDccol (CSBptr, cix)

CSB		*CSBptr;		/* Addr of CSB */
int		cix;			/* Character index */

{
IND	int	c;			/* Char */
IND	int	w;			/* Which string being considered */
					/* 0 = prompt, 1 = text */
IND	char	*sptr;			/* String pointer */
IND	int	col;			/* Column # accumulator */


col = 0;				/* Init col # */
sptr = CSBptr -> CSB_PMT;		/* Look at prompt buffer first */
for (w = 0; w < 2; w++)			/* Step through 2 strings */
    {
    while ((c = *sptr++) != NUL)	/* Look at the string */
	{
	if (w == 1)			/* If text string */
	    if (cix-- == 0)		/* If at desired index */
		break;			/* be done */
	if (c == '\011')		/* If tab */
	    col = (col+8)&(-8);		/* Calc next position */
	else if (c < ' ')		/* Control char ? */
	    col += 2;			/*  count carat, and char */
	else				/* anything else */
	    col++;			/* counts as one */
	}
    sptr = CSBptr -> CSB_BUF;		/* Do text buffer now */
    }

return (col);				/* Return the value */
}

/*

*//* CMDpoc(c)

	Output character.

*/

CMDpoc(c)

int			c;

{
(*Cocrtc)(c);
}
