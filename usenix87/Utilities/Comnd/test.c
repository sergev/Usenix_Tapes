/* The following is a test program for a subroutine set which provides
   a simple emulation of the TOPS-20 COMND JSYS for C programs.  This
   program tests some of the basic features, and illustrates how the
   subroutine package is used. */

#include "comnd.h"			/* Include interface definitions */
					/* for the COMND package */
#include "stdio.h"			/* Include standard system defn's */
#include "setjmp.h"			/* Include defn for the setjmp/longjump */
					/* facility of the C language */

/* Declare forward routines that our program uses; mainly those routines
   which are included in the command dispatch table.  Since they will be in
   the table, they must be defined. */

   int		datcmd();		/* The DATE command */
   int		exicmd();		/* The EXIT command */
   int		hlpcmd();		/* The HELP command */
   int		numcmd();		/* The NUMBER command */
   int		uicmd();		/* Unimplemented command */

/* Declare various simple variables our program uses. */


char		Atmbuf[100] = {0};	/* Atom buffer used by CSB */
jmp_buf		Topenv = {0};		/* Setjump buffer used in command
					   parsing. */
char		Txtbuf[100] = {0};	/* Text buffer used by CSB (later) */


/* Define our command table and dispatch table.  The command table is a
   table of pointers to strings; these strings do not have to be in any
   order; but remember they are printed (for help) in the order given
   here. */

/* Note that in the command table, only a few of the entries are
   reasonable.  The rest of the keywords are in the table to illustrate
   the way that keywords are matched by the COMND subroutines. */

char		*Cmdtbl[] = {		/* Command keyword table */
				"ABLE",
				"BAKER",
				"BARKER",
				"BASKET",
				"CHANCELLOR",
				"CHESTER",
				"DATE",
				"DOG",
				"EXIT",
				"HELP",
				"NUMBER",
				NULL	/* The table ends with a null */
			    };

int		(*Cmddsp[])() = {	/* Dispatch table for commands */
			uicmd,		/* ABLE */
			uicmd,		/* BAKER */
			uicmd,		/* BARKER */
			uicmd,		/* BASKET */
			uicmd,		/* CHANCELLOR */
			uicmd,		/* CHESTER */
			datcmd,		/* DATE */
			uicmd,		/* DOG */
			exicmd,		/* EXIT */
			hlpcmd,		/* HELP */
			numcmd		/* NUMBER */
				};


/* Define our Command State Block.  A program may have any number of
   these, and typically have a couple (one for main level commands and
   another for subcommands, etc.).  This program only uses one, because
   it is not very complex. */


   CSB		Topcsb = {0,		/* Flags to pass to COMND subroutine */
			  0,		/* Flags returned by COMND */
			  &Topenv,	/* Address of setjmp buffer; used in
			  		   transfering control if a reparse
					   is necessary */
			  0,		/* Input designator (ignored) */
			  0, 		/* Output designator (ignored) */
			  "TEST> ",	/* Prompt string */
			  &Txtbuf,	/* Address of text buffer */
			  100,		/* Size of text buffer (bytes) */
			  &Atmbuf,	/* Address of atom buffer */
			  100,		/* Size of atom buffer (bytes) */
			  0};		/* The rest of the block is used for
			  		   returned values and does not have
					   to be initialized here */


/* Define the various Command Function Blocks that we will use.  Each
   function block defines something to be parsed.  The first definition is
   expanded with comments; the rest are simply defined. */


   CFB		Cmdcfb = {_CMKEY,	/* Function code (_CMKEY=keyword) */
			  _CFDPP|_CFHPP, /* Flags;  _CFDPP indicates that
			  		    we've supplied a default string;
					    _CFHPP indicates that we've
					    supplied our own help text to be
					    used in addition to the standard
					    help.  _CFSDH would suppress the
					    standard help as well. */
			   0,		/* This would be an address of another
			   		   CFB to be used in satisfying the
					   parse.  No alternatives here */
			   &Cmdtbl,	/* Data for the function; addr of
			   		   keyword table, here. */
			   "Command, ",	/* Help text that we supply */
			   "BASKET"	/* Default string. */
			   };

	/* CFB for HELP... this illustrates how CFBs can be chained to give
	   alternative parse paths. */
CFB		Hlpcfb = {_CMTOK, _CFHPP|_CFSDH, &Cmdcfb, "*",
			  "\"*\" for help on all topics", 0};

	/* Initialization CFB */
CFB		Inicfb = {_CMINI, 0, 0, 0, 0, 0};

	/* CFB for guide words */
CFB		Noicfb = {_CMNOI, 0, 0, 0, 0, "guide words"};

	/* CFB for confirmation */
CFB		Cfmcfb = {_CMCFM, 0, 0, 0, 0, 0};

	/* CFB for date parse */
CFB		Datcfb = {_CMDAT, _CFDTD|_CFDTT, 0, 0, 0, 0};

	/* CFB for decimal number parse */
CFB		Numcfb = {_CMNUM, 0, 0, 10, 0, 0};
/*

*/

/* The main routine. */


main()

{
IND	int	i;			/* Scratch */
IND	char	**kptr;			/* Keyword ptr ptr */


/* Enter command loop. */

while (TRUE)
    {

/* The first part of COMND parsing is to initialize the parse.  This is
   done with a CFB with function code of _CFINI */

    COMND (&Topcsb, &Inicfb);		/* Init the parse */


/* Call setjmp to mark the point where a reparse of the command string would
   take place.  Since we've supplied this setjmp buffer address to COMND (by
   putting its address in our CSB), COMND will transfer control here whenever
   a reparse should take place.  If the setjmp mechanism is not used, the
   program must always check for a return code of _CRRPT, indicating that
   a reparse is necessary.  The setjmp mechanism is the far simpler method. */

    setjmp (Topenv);			/* Trap reparse */

/* Now parse a command keyword.  This is done by calling COMND with the
   appropriate command function block. */

    if (!COMNDi (&Topcsb, &Cmdcfb))	/* Parse a command */
	continue;			/*  continue if failed.  (see the */
					/*  routine COMNDI() below) */

/* Now determine what keyword was parsed.  The return value (in CSB_RVL of
   the command state block) is the address of the keyword table entry which
   was parsed.  Thus it is a pointer to a pointer to the keyword. */

    kptr = (char **) (Topcsb.CSB_RVL._ADR);
					/* Get the table entry address */
    i = kptr - &Cmdtbl[0];		/* Get the command table index */


/* i now has the command index; simply dispatch via the command dispatch
   table to the appropriate processing routine. */

    (*Cmddsp[i])();			/* Call the routine */

/* End of command loop. */

    }

}
/*

*/

/* datcmd - the DATE command */

datcmd ()

{
IND	int	id,it,m,d,y,mm,hh;	/* Date/time values */
IND	int	*rslptr;		/* Result pointer */

/* Issue a call to our "noise" subroutine (below) to parse guide words. */

if (!noise("is"))			/* Do guide word parse */
    return;				/* And return if it failed */

/* Parse the command argument */

if (!COMNDi(&Topcsb, &Datcfb))		/* Do COMND call and check failure */
    return;

/* Issue call to our "confrm" routine to confirm the command. */

if (!confrm())				/* Call our general confirm routine */
    return;				/* If not confirmed, just return. */


rslptr = &Atmbuf[0];
id = rslptr[0];				/* Get results */
it = rslptr[1];
cvided (id, it, &m, &d, &y, &hh, &mm);
printf ("Returned %02d/%02d/%04d %02d:%02d\n", m, d, y, hh, mm);

}
/*

*/

/* exicmd - the EXIT command */

exicmd ()

{
/* Issue a call to our "noise" subroutine (below) to parse guide words. */

if (!noise("program"))			/* Do guide word parse */
    return;				/* And return if it failed */

/* Issue call to our "confrm" routine to confirm the command. */

if (!confrm())				/* Call our general confirm routine */
    return;				/* If not confirmed, just return. */

exit();					/* Exit the program. */
}
/*

*/

/* hlpcmd - the HELP command */

/* This command illustrates how COMND is used to parse one of multiple
   choices; here we either parse the token "*" or a command keyword. */

hlpcmd ()

{
char		**kptr;			/* Points to keyword */
char		*aptr;			/* Points to argument */

/* Collect the help argument, after giving appropriate guide string */

if (!noise ("on subject"))		/* Give guide string */
    return;				/*  return if it failed */

/* Parse the command argument. */

if (!COMNDi(&Topcsb, &Hlpcfb))		/* Do COMND call and check failure */
    return;

/* Since we supplied alternate CFBs in our COMND call, we have to see which
   one matched the input, and process accordingly.  Here "process" is simply
   to set a pointer to the text we are going to say we can't give help for */

if (Topcsb.CSB_CFB == &Hlpcfb)		/* If matched our token */
    aptr = "*";				/* Set pointer to token string. */
else					/* Otherwise */
    {
    kptr = (char **) Topcsb.CSB_RVL._ADR;
					/* Get ptr to keyword pointer */
    aptr = *kptr;			/* Get addr of string */
    }

if (!confrm())				/* Call our general confirm routine */
    return;				/* If not confirmed, just return. */

/* Now we've got the keyword; this is only a test routine, show the thing
   parsed and say we can't give help for it. */

printf ("Sorry, can not give help for ");
printf (aptr);
printf ("\n");
}
/*

*/

/* numcmd - the NUMBER command */

numcmd ()

{
IND	int	num;			/* Number */

if (!noise ("to print"))		/* Get/give guide string */
    return;				/* Return if invalid */

if (!COMNDi (&Topcsb, &Numcfb))		/* If not ok */
    return;

/* Extract the number from the returned value field in the CSB; then go
   on to confirm the command. */

num = Topcsb.CSB_RVL._INT;		/* Get the number */

if (!confrm())				/* Call our general confirm routine */
    return;				/* If not confirmed, just return. */

printf ("Number is %d\n", num);		/* Print the number, for show */
}
/*

*/

/* uicmd - unimplemented or silly commands */

uicmd ()

{
if (!noise ("nothing"))			/* Give random guide string */
    return;				/* Return if bad parse of it */

if (!confrm())				/* Call our general confirm routine */
    return;				/* If not confirmed, just return. */

printf ("This command is not implemented.\n");
}
/*

*/

/* noise - parses a guide string.  Called with the address of the expected
   guide string; without the parentheses, of course. */

noise (str)

char		*str;			/* Address of string */

{
Noicfb.CFB_DEF = str;			/* Store the string pointer in
					   the guide word CFB */
return (COMNDi(&Topcsb, &Noicfb));	/* Do parse and return the result */
}
/*

*/

/* confrm - get confirmation (call COMND for confirm) */

/* Returns TRUE if OK confirmation; FALSE if not. */
 
confrm ()

{
if (COMNDi (&Topcsb, &Cfmcfb))		/* Get confirmation */
    return (TRUE);			/* Give OK return if success */
printf (" ?Not confirmed\n");		/* Give another error msg */
return (FALSE);				/* Return bad status. */
}
/*

*/

/* COMNDi - interface to the COMND() library routine, giving a message if
   a parse error occurs.  Returns TRUE or FALSE depending on success */

int COMNDi (CSBptr, CFBptr)

CSB		*CSBptr;		/* Address of command state block */
CFB		*CFBptr;		/* Address of command function block */

{
IND	int	i;			/* A counter */
IND	char	*sptr;			/* A string pointer */

if (COMND(CSBptr, CFBptr) == _CROK)	/* If successful parse */
    return (TRUE);			/*  then give good return */
sptr = &CSBptr->CSB_BUF[CSBptr->CSB_PRS]; /* Get addr of unrecognized input */
i = CSBptr->CSB_FLN - CSBptr->CSB_PRS;	/* Get # of chars unrecognized */
printf (" ??Invalid- Can not recognize \"");
while (i--)
    putchar (*sptr++);			/* Print the bad string */
printf ("\"... use ? here.\n");		/* Tell him how to proceed. */
return (FALSE);				/* Give bad return */
}
