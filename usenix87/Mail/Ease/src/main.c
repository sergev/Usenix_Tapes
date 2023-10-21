/*	$Header: /usr/src/local/etc/ease/RCS/main.c,v 1.2 85/10/29 23:43:38 jss Exp $	*/

/*
 *  	main.c     -- Main procedure for Ease Translator.
 *
 *  	author     -- James S. Schoner, Purdue University Computing Center
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

extern FILE *DIAGf;			/* diagnostic file */
extern void InitError (), 
	    InitSymbolTable (),
	    DefScan (),
	    FatalError (),
            PreLoad ();

int ErrorCount;				/* translation error count */
void GetArgs ();			/* gets arguments to "et"  */

/*
 *	main () -- Main procedure for the Ease Translator et.  If no files are 
 *	       	   given as arguments to et, stdin is translated and written to 
 *	           stdout.  If one file is given, it is translated and written 
 *	           to stdout.  If two files are given, the first is translated
 *	           and written to the second.  If the first filename is "-",
 *	           standard input is assumed.  A translation is performed on 
 *	           valid Ease input only, producing a regular sendmail 
 *		   configuration file. 
 *
 */
void
main (argc, argv)
int argc;		/* argument count for "et"  */
char *argv[];		/* argument vector for "et" */
{
	InitError ();			/* initialize error conditions */
	InitSymbolTable ();		/* initialize the symbol table */
	PreLoad ();			/* preload special identifiers */
	GetArgs (argc, argv);		/* set up argument files       */
	(void) yyparse ();		/* perform translation	       */
	if (fflush (stdout) == EOF)
		FatalError ("Cannot flush output stream/file", (char *) NULL);
	DefScan ();		        /* warn about undefined idents */
	if (ErrorCount)
		fprintf (DIAGf, "\n\n*** %d error(s) detected.\n", ErrorCount);
	exit (ErrorCount);
}


/*
 *	GetArgs () -- Processes arguments to the Ease translator "et".  The
 *		      arguments are files (margv) which may replace either/both
 *		      of the files standard input and standard output.  The 
 *		      following cases are possible:
 *			
 *		      -- et f.e f.cf
 *				Translate Ease file f.e and write result
 *				to f.cf.
 *
 *		      -- et f.e
 *				Translate Ease file f.e and write result to
 *				standard output.
 *
 *		      -- et - f.cf
 *				Translate standard input and write result to
 *				f.cf.
 *
 *		      -- et
 *				Translate standard input and write result to
 *				standard output.
 *
 *		      Finally, a message indicating the volatility of the 
 *		      Ease output is written.
 *
 */
void
GetArgs (margc, margv)
register int   margc;		/* argument count  */
register char *margv[];		/* argument vector */
{
	switch (margc) {
		case 1 : break;
		case 2 :
		case 3 : if (strcmp (margv[1], "-") && (freopen (margv[1], "r", stdin) == NULL))
				FatalError ("Cannot open input stream/file:", margv[1]);
			 if ((margc == 3) && (freopen (margv[2], "w", stdout) == NULL))
				FatalError ("Cannot open output file:", margv[2]);
			 break;
		default: FatalError ("Usage: et [infile [outfile]]", (char *) NULL);
			 break;
	}
	printf ("###################################################\n");
	printf ("##                                               ##\n");
	printf ("##  WARNING: THIS FILE IS TO BE MODIFIED BY      ##\n");
	printf ("##           THE EASE TRANSLATOR (ET) ONLY.      ##\n");
	printf ("##                                               ##\n");
	printf ("##           ALL OTHER MODIFICATIONS WILL        ##\n");
	printf ("##           DISAPPEAR THE NEXT TIME ET IS RUN.  ##\n");
	printf ("##                                               ##\n");
	printf ("##           MAKE MODIFICATIONS TO THE EASE      ##\n");
	printf ("##           SOURCE ONLY.                        ##\n");
	printf ("##                                               ##\n");
	printf ("###################################################\n");
}
