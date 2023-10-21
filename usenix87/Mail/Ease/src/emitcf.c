/*	$Header: /usr/src/local/etc/ease/RCS/emitcf.c,v 1.3 85/11/22 20:14:11 jss Exp $	*/

/*
 *	emitcf.c  -- This file contains routines associated with the writing
 *		     and formatting of a translated sendmail configuration file.
 *
 *	author	  -- James S. Schoner, Purdue University Computing Center,
 *			  	       West Lafayette, Indiana  47907
 *
 *  	date	  -- July 9, 1985
 *
 *	Copyright (c) 1985 by James S. Schoner
 *
 *	All rights reserved.
 *
 */

#include <stdio.h>
#include "symtab.h"

#define REGLINE 60	/* length of output lines which may be continued */
#define MAXLINE 256	/* liberal maximum line length			 */

extern short Rformat;			/* read-format flag for a class  */
extern char *MacScan ();
extern char  MakeMac ();
extern void  PrintError (),
	     FatalError (),
	     PrintWarning (),
	     ErrorReport ();

void  PrintDef ();

static char ClassCH;			/* printable class macro char    */

/*
 *	EmitDef () -- Emit a definition line (Ease block definition) in cf 
 *		      format.
 *
 */
void
EmitDef (blockdef, targ, defstr1, defstr2)
register enum bdefs blockdef;	/* type of definition   	 */
register struct he *targ;	/* target to be defined 	 */
char *defstr1, *defstr2;	/* one or two definition strings */
{
	/*
	 *  This routine is about as pretty as a translated ease file...
	 *  Each type of line (Ease block) is handled case by case below.
	 *
	 */
	switch (blockdef) {
		case def_macro:		printf ("D%c", MakeMac (targ, ID_MACRO));
					PrintDef (def_macro, MacScan (defstr1));
					if (ISMACRO(targ->idd))
						PrintWarning ("Redefining macro %s.\n", targ->psb);
					targ->idd |= ID_MACRO;  /* signal definition */
					break;

		case def_class:		if (Rformat)	/* read format */
						printf ("F");
					else
						printf ("C");
					printf ("%c", ClassCH = MakeMac (targ, ID_CLASS));
					if (Rformat) {	/* read format */
						printf ("%s\n", defstr1);
						Rformat = FALSE;
					} else
						PrintDef (def_class, defstr1);
					if (ISCLASS(targ->idd))
						PrintWarning ("Redefining class %s.\n", targ->psb);
					targ->idd |= ID_CLASS;  /* signal definition */
					break;

		case def_option:	printf ("O%c", *defstr1);
					PrintDef (def_option, defstr2);
					break;

		case def_prec:		printf ("P%s=%d\n", targ->psb, targ->idval.prec);
					break;

		case def_trusted:	printf ("T");
					PrintDef (def_trusted, defstr1);
					break;

		case def_header:	printf ("H");
					if (defstr1 != NULL)
						printf ("?%s?", defstr1);
					PrintDef (def_header, defstr2);
					break;

		case def_mailer:	if (ISMAILER(targ->idtype)) {
						if (ISMAILER(targ->idd))
							PrintWarning ("Redefining mailer %s.\n", targ->psb);
					} else if (ISTYPED(targ->idtype)) {
						PrintError ("Redeclaration of identifier as mailer:", targ->psb);
						return;
					}
					targ->idd |= ID_MAILER;  /* signal definition */
					printf ("M%s, ", targ->psb);
					PrintDef (def_mailer, defstr1);
					break;

		case def_ruleset:	printf ("R");
					PrintDef (def_ruleset, defstr1);
					break;

		default:		FatalError ("Bad case in EmitDef ()", (char *) NULL);
	}
}


/*
 *  	PrintContinued () -- Print a line definition (buf) by splitting it over
 *			     more than one line.  The two definition types 
 *			     accepted for this method of continuation are class
 *			     and trusted user lists, indicated in the argument 
 *			     btype 
 *
 */
void
PrintContinued (btype, buf)
enum bdefs     btype;	/* block (line) type for definition */
register char *buf;	/* buffer containing the definition */
{
	register char *tmp;	/* breakpoint search pointer   */
	register char  tc;	/* temporary swap byte         */
	int  buflen;		/* length of definition buffer */	

	buflen = strlen (buf);
	tmp = buf + REGLINE;
	while ((*--tmp != ' ') && (tmp != buf))	 /* look for suitable break */
		/* null */ ;
	if (tmp == buf) {
		for (tmp = buf + REGLINE; (*tmp != ' ') && (tmp - buf != buflen); tmp++)
			/* null */ ;
		if ((tmp - buf) >= MAXLINE)
			PrintWarning ("Member name may be too long.\n", (char *) NULL);
	}
	tc = *tmp;		/* swap break char with null char */
	*tmp = '\0';
	printf ("%s\n", buf);
	if ((*tmp = tc) == '\0')
		return;
	else
		tmp++;
	if (btype == def_class)		/* start next line   */
		printf ("C%c", ClassCH);
	else
		printf ("T");
	if (strlen (tmp) < REGLINE)	/* continue the line */
		printf ("%s\n", tmp);
	else
		PrintContinued (btype, tmp);
}


/*
 *	PrintDef () -- Handles special cases (like line continuation) when 
 *		       printing definitions.
 *
 */
void
PrintDef (btype, dstr)
register enum bdefs btype;	/* block type (output line type) */
register char *dstr;		/* definition string		 */
{
	register char *tmp;

	for (tmp = dstr; *tmp != '\0'; tmp++) 	/* search for line continuations */
		if ((*tmp == '\\') && (*++tmp == '\n'))
			if (btype != def_header) {
				ErrorReport ("Non-header string contains line continuation\n");
				return;
			} else
				break;

	/*
	 *  Perform case by case handling of definition printing.
	 *
	 */
	switch (btype) {
		case def_header :  if (*tmp-- == '\n') {
					*tmp = '\0';
					if (tmp - dstr >= MAXLINE)
						PrintWarning ("Header may be too long.\n", 
							      (char *) NULL);
					printf ("%s\n\t", dstr);
					tmp += 2;
				        PrintDef (def_header, tmp);
				   } else {
					if (strlen (dstr) >= MAXLINE)
						PrintWarning ("Header may be too long.\n", 
							      (char *) NULL);
					printf ("%s\n", dstr);
				   }
				   break;

		case def_mailer :  if (strlen (dstr) >= MAXLINE)
					PrintWarning ("Mailer definition may be too long.\n", 
						      (char *) NULL);
				   printf ("%s\n", dstr);
				   break;

		case def_ruleset:  if (strlen (dstr) >= MAXLINE)
					PrintWarning ("Rewriting rule may be too long.\n", 
						      (char *) NULL);
				   printf ("%s\n", dstr);
				   break;

		case def_option :  if (strlen (dstr) >= MAXLINE)
					PrintWarning ("Option assignment may be too long.\n", 
						      (char *) NULL);
				   printf ("%s\n", dstr);
				   break;

		case def_macro  :  if (strlen (dstr) >= MAXLINE)
					PrintWarning ("Macro assignment may be too long.\n", 
						      (char *) NULL);
				   printf ("%s\n", dstr);
				   break;

		case def_prec   :  if (strlen (dstr) >= MAXLINE)
					PrintWarning ("Precedence relation may be too long.\n", 
						      (char *) NULL);
				   printf ("%s\n", dstr);
				   break;

		case def_trusted:
		case def_class  :  if (strlen (dstr) < REGLINE)
					printf ("%s\n", dstr);
				   else		/* use line continuation feature */
				   	PrintContinued (btype, dstr);
				   break;

		default         :  FatalError ("Invalid case in PrintDef ()", (char *) NULL);
	}
}


/*
 *	StartRuleset () -- Prints a ruleset heading for the ruleset identifier
 *		           contained in the argument rsid.
 *
 */
void
StartRuleset (rsid)
register struct he *rsid;	/* ruleset identifier */
{
	if (!ISRULESET(rsid->idtype))
		if (ISTYPED(rsid->idtype))
			PrintError ("Identifier not of ruleset type:", rsid->psb);
		else
			PrintError ("Ruleset identifier not bound to a number:", rsid->psb);
	else {
		if (ISRULESET(rsid->idd))
			PrintWarning ("Redefining ruleset %s.\n", rsid->psb);
		rsid->idd |= ID_RULESET;
		printf ("S%s\n", rsid->idval.rsn);
	}
}
