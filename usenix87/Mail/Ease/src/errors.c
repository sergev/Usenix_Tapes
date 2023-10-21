/*	$Header: /usr/src/local/etc/ease/RCS/errors.c,v 1.2 85/10/29 23:40:20 jss Exp $	*/

/*
 *  	errors.c   -- Contains error initialization and reporting routines.
 *
 *  	author     -- James S. Schoner, Purdue University Computing Center,
 *				        West Lafayette, Indiana  47907
 *
 *  	date       -- July 9, 1985
 *
 *	Copyright (c) 1985 by James S. Schoner
 *
 *	All rights reserved.
 *
 */

#include <stdio.h>

extern int  ErrorCount;	 /* error count	               */
extern char FNbuf[];	 /* input file name   	       */
extern int  Lcount;	 /* line count	     	       */
FILE *DIAGf = {stderr};  /* file for diagnostic output */


/*
 *	ErrorReport () -- Prints source file name (FNbuf), line number (Lcount),
 *			  and error message (sbErr) for each invokation.
 *
 */
void
ErrorReport (sbErr)
char *sbErr;
{
	fprintf (DIAGf, "%s, line %d: %s", FNbuf, Lcount, sbErr);
	ErrorCount++;
}


/*
 *	FatalError () -- Translator fatal error routine which prints 
 *			 error message (sbErr) and an argument (sbArg).
 *
 */
void
FatalError (sbErr, sbArg)
char *sbErr,
     *sbArg;
{
	fprintf (DIAGf, "%s, line %d: Fatal Error In Translator: %s %s\n", 
		 FNbuf, Lcount, sbErr, sbArg);
	exit (1);
}


/*
 *	yyerror () -- Prints source file name (FNbuf), line number (Lcount),
 *		      and error message (sbErr) for each invokation.
 *
 */
void
yyerror (sbErr)
char *sbErr;
{
	fprintf (DIAGf, "%s, line %d: %s\n", FNbuf, Lcount, sbErr);
	ErrorCount++;
}


/*
 *	PrintError () -- Prints source file name (FNbuf), line number
 *			 (cline), error message (sbErr), and argument
 *			 (sbArg) for each invokation.
 *
 */
void
PrintError (sbErr, sbArg)
char *sbErr;
char *sbArg;
{
	fprintf (DIAGf, "%s, line %d: %s %s.\n", FNbuf, Lcount, sbErr, sbArg);
	ErrorCount++;
}


/*
 *	PrintWarning () -- Prints a warning message with source file
 *			   name (FNbuf), line number (Lcount), warning
 *			   (sbWarn), and a possible identifier (sbID).
 *
 */
void
PrintWarning (sbWarn, sbID)
char *sbWarn;
char *sbID;
{
	fprintf (DIAGf, "%s, line %d: Warning: ", FNbuf, Lcount);
	if (sbID != NULL)
		fprintf (DIAGf, sbWarn, sbID);
	else
		fprintf (DIAGf, sbWarn);
}


/*
 *	InitError () -- Initialize line count (Lcount) to one and error count
 *		        (ErrorCount) to zero.
 *
 */
void
InitError ()
{
	Lcount     = 1;
	ErrorCount = 0;
}
