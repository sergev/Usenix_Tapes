/*	$Header: /usr/src/local/etc/ease/RCS/idman.c,v 1.2 85/10/29 23:41:38 jss Exp $	*/

/*
 *  	idman.c	-- Contains routines for manipulating identifiers and their
 *		   symbolic associations.
 *
 *  	author	-- James S. Schoner, Purdue University Computing Center,
 *				     West Lafayette, Indiana  47907
 *
 *  	date	-- July 9, 1985
 *
 *	Copyright (c) 1985 by James S. Schoner
 *
 *	All rights reserved.
 *
 */

#include <stdio.h>
#include "symtab.h"

extern struct he *LookupSymbol ();
extern void	  FatalError (),
		  ErrorReport (),
		  PrintWarning (),
		  PrintError ();

short Uchar = 'A';			/* for unique macro names */


/*
 *	UniqMac () -- Assigns and returns a unique one-character macro
 *		      name (upper-case) for an Ease macro name.
 *
 */
char
UniqMac (phe)
struct he *phe;		/* symbol table entry for Ease macro */
{
	if ((phe->idval.idc = Uchar++) > 'Z')
		FatalError ("Too many macro names (26 max)", (char *) NULL);
	return (phe->idval.idc);
}


/*
 *	BindID () -- Binds either a ruleset or precedence identifier (phe) to
 * 		     an integer (vid).  The id type distinction is passed in
 *		     the parameter idt.
 *
 */
void
BindID (phe, vid, idt)
register struct he *phe;	/* symbol table entry for an identifier    */
int vid;			/* value of the identifier		   */
unsigned idt;			/* identifier type (ruleset or precedence) */
{
	if (ISTYPED(phe->idtype)) {	/* should be undefined */
		PrintWarning ("Redeclaration of %s.\n", phe->psb);
		phe->idtype = ID_UNTYPED;
	}
	phe->idtype |= idt;		/* make defined	       */
	if (ISRULESET(phe->idtype)) {
		if (vid > VALRSNMAX) {
			ErrorReport ("Ruleset number too large.\n");
			return;
		} else if (vid < 0) {
			ErrorReport ("Ruleset number must be non-negative.\n");
			return;
		}
		sprintf (phe->idval.rsn, "%d", vid);
	} else 
		phe->idval.prec = vid;
}


/*
 *	CheckRS () -- Checks validity of a ruleset identifier (phe) and 
 *		      returns the ruleset string to which the identifier
 *		      is bound.  If the ruleset identifier is invalid, the
 *		      null string is returned.
 *
 */
char *
CheckRS (phe)
struct he *phe;		/* symbol table entry for ruleset identifier */
{
	if (!ISRULESET(phe->idtype)) {
		if (!ISTYPED(phe->idtype))
			PrintError ("Ruleset identifier not bound to a number:", phe->psb);
		else
			PrintError ("Identifier not of ruleset type:", phe->psb);
		return (NULL);
	} else
		return (phe->idval.rsn);
}


/*
 *	MakeMac () -- Declare a macro name (pmac) as a class and/or macro type 
 *		      (targtype) and return the unique cf character assigned 
 *		      to it.
 *
 */
char
MakeMac (pmac, targtype)
register struct he *pmac;	/* symbol table entry for macro identifier */
unsigned targtype;		/* target declaration type for the macro   */
{
	/*
	 *	An Ease macro may be declared as both a singular macro and
	 *	a class macro.
	 *
	 */
	if (ISMACRO(pmac->idtype) || ISCLASS(pmac->idtype)) {
		pmac->idtype |= targtype;
		return (pmac->idval.idc);
	}
	if (ISTYPED(pmac->idtype)) {	/* not a macro or class id */
		PrintError ("Redeclaring or using as macro or class:", pmac->psb);
		return ('\0');
	}
	pmac->idtype |= targtype;	/* previously untyped; declare here */
	return (UniqMac (pmac));
}
	

/*
 *	GetField () -- Returns a field type string given a field 
 *		       identifier (fid).
 *
 */
char *
GetField (fid)
register struct he *fid;	/* field identifier */
{
	if (!ISFIELD(fid->idtype)) {
		PrintError ("Field type not defined for", fid->psb);
		return (NULL);
	} else 
		return (fid->idval.fstring);
}


/*
 *	CheckMailer () -- Declares a mailer identifier (mid) as type mailer,
 *			  checking that the identifier was not previously 
 *			  declared a different type. 
 *
 */
char *
CheckMailer (mid)
register struct he *mid;
{
	if (ISTYPED (mid->idtype) && !ISMAILER (mid->idtype)) {
		PrintError ("Redeclaration as mailer:", mid->psb);
		return (NULL);
	}
	mid->idtype |= ID_MAILER;
	return (mid->psb);
}


/*
 *	AssignType () -- Assigns to each field identifier in fidlist the
 *			 type (in string form) fidtype.  This is accomplished
 *			 by making each field identifier symbol table entry
 *			 "point" to the type found in fidtype.
 *
 */
void
AssignType (fidlist, fidtype)
register char *fidlist;		/* field identifier list, blank separated */
char *fidtype;			/* field identifier type string		  */
{
	register struct he *fid;	/* pointer to a field identifier  */
	char *fres;			/* field type result string	  */
	register char *srch;		/* fidlist search pointer	  */
	char  sep;			/* fidlist separator character    */

	fres = (char *) malloc (strlen (fidtype) + 1);
	if (fres == NULL)
		FatalError ("System out of string space in AssignType ()", (char *) NULL);
	strcpy (fres, fidtype);		/* make clean copy of string type */

	/*
	 *	Search for all field identifiers and make the type assignment. 
 	 *
	 */
	srch = fidlist;
	while (*srch != '\0') {
		while ((*++srch != ' ') && (*srch != '\0'))
			/* null */ ;
		if (*fidlist != '\0') {		        /* found a field id       */
			sep = *srch;
			*srch = '\0';
			fid = LookupSymbol (fidlist);	/* get symbol table entry */
			if (ISFIELD(fid->idtype)) {
				if (strcmp (fid->idval.fstring, fres))
					PrintWarning ("Redefinition of field type for %s.\n", fid->psb);
			} else if (ISTYPED(fid->idtype)) {
				PrintError ("Redeclaration of identifier as a field:", fid->psb);
				return;
			}
			fid->idtype |= ID_FIELD;	/* type the identifier    */
			fid->idval.fstring = fres;	/* type the field	  */
			if ((*srch = sep) != '\0')
				fidlist = ++srch;
		}
	}
}
