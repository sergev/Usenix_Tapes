/*	$Header: /usr/src/local/etc/ease/RCS/symtab.c,v 1.2 85/10/29 23:46:48 jss Exp $	*/

/*
 *  	symtab.c   -- Contains Ease Translator symbol table routines.
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
#include <ctype.h>
#include "symtab.h"

#define ERRORMAILER "error"		/* predefined error mailer name */

extern void FatalError (),
	    PrintWarning ();

struct he *LookupSymbol ();

struct Defmac {				/* predefined macro struct def  */
	char *macname;
	char  macrep;
};

struct he *SymTab[SST];			/* hash table base array        */
static struct Defmac MacDefs[] = {	/* predefined macros	        */
			{"m_smtp",	'e'},
			{"m_oname",	'j'},
			{"m_ufrom",	'l'},
			{"m_daemon",	'n'},
			{"m_addrops",	'o'},
			{"m_defaddr",	'q'},
			{"m_sitename",	'w'},
			{"m_odate",	'a'},
			{"m_adate",	'b'},
			{"m_hops",	'c'},
			{"m_udate",	'd'},
			{"m_saddr",	'f'},
			{"m_sreladdr",	'g'},
			{"m_rhost",	'h'},
			{"m_qid",	'i'},
			{"m_pid",	'p'},
			{"m_protocol",	'r'},
			{"m_shostname", 's'},
			{"m_ctime",	't'},
			{"m_ruser",	'u'},
			{"m_version",	'v'},
			{"m_sname",	'x'},
			{"m_stty",	'y'},
			{"m_rhdir",	'z'},
			{"sentinel",	'\0'}
};


/*
 *	DefScan () -- Scan symbol table to find macros, classes, mailers, 
 *		      and rulesets which have been referenced or declared, but
 *		      not defined.  A warning is printed for each such 
 *		      occurence.  This routine is usually called at the end
 *		      of a successful Ease translation.
 *
 */
void
DefScan ()
{
	register int stindex;		/* symbol table hash index   */
	register struct he *hcsearch;	/* hash chain search pointer */

	for (stindex = 0; stindex < SST; stindex++) {
		if ((hcsearch = SymTab[stindex]) != NULL)
			while (hcsearch != NULL) {
				if ((ISMACRO(hcsearch->idtype) && 
				     isupper(hcsearch->idval.idc)) &&
				     !ISMACRO(hcsearch->idd))
					PrintWarning ("Macro not defined: %s\n", hcsearch->psb);
				if (ISCLASS(hcsearch->idtype) && !ISCLASS(hcsearch->idd))
					PrintWarning ("Class not defined: %s\n", hcsearch->psb);
				if (ISMAILER(hcsearch->idtype) && !ISMAILER(hcsearch->idd))
					PrintWarning ("Mailer not defined: %s\n", hcsearch->psb);
				if (ISRULESET(hcsearch->idtype) && !ISRULESET(hcsearch->idd))
					PrintWarning ("Ruleset not defined: %s\n", hcsearch->psb);
				hcsearch = hcsearch->phe;
			}
	}
}
				     

/*
 *	InitSymbolTable () -- Invoked by main () to initialize the symbol table.
 *
 */
void
InitSymbolTable ()
{
	int i;

	for (i = 0; i < SST; i++)		/* initialize base array */
		SymTab[i] = NULL;
}


/*
 *	PreLoad () -- Invoked by main () to preload special macro names 
 *		      and mailer declarations.
 *
 */
void
PreLoad ()
{
	struct Defmac *macptr;
	struct he     *symptr;

	/* preload special (lower-case) macros */
	for (macptr = &MacDefs[0]; (*macptr).macrep != '\0'; macptr++) {
		symptr = LookupSymbol ((*macptr).macname);
		symptr->idtype |= ID_MACRO;
		symptr->idval.idc = (*macptr).macrep;
	}

	/* preload error mailer declaration */
	symptr = LookupSymbol (ERRORMAILER);
	symptr->idtype |= ID_MAILER;
	symptr->idd |= ID_MAILER;
}
	

/*
 *	LookupSymbol () -- Returns a pointer to the hash entry already 
 *			   existing, or newly created, which corresponds 
 *			   to string sb.
 *
 */
struct he *
LookupSymbol (sb)
char sb[];			/* string buffer containing identifier */
{
	struct he *phe;		/* hash entry search pointer  */
	int	  hc;		/* hash code of sb identifier */

	phe = SymTab[hc = HashCode (sb)];
	while (phe != NULL)			/* find hash entry for sb */
		if (!strcmp (phe->psb, sb))
			return (phe);
		else
			phe = phe->phe;
	/* make new symbol table entry */
	if ((phe = (struct he *) malloc (sizeof (struct he))) == NULL)
		FatalError ("System out of space in LookupSymbol ()", (char *) NULL);
	if ((phe->psb = (char *) malloc (strlen (sb) + 1)) == NULL)
		FatalError ("System out of space in LookupSymbol ()", (char *) NULL);
	strcpy (phe->psb, sb);
	phe->idval.idc = '\0';
	phe->idtype = ID_UNTYPED;
	phe->idd = ID_UNTYPED;
	phe->phe = SymTab[hc];
	return (SymTab[hc] = phe);
}


/*
 *	RemoveSymbol () -- Removes the symbol table entry phe from the 
 *			   symbol table.
 *
 */
void
RemoveSymbol (phe)
struct he *phe;	   /* pointer to hash entry to be removed from symbol table */
{
	int hc;	   		/* hash code of entry phe       */
	struct he *sphe;	/* search pointer for entry phe */

	if (phe == NULL)
		return;
	else {			/* search and remove entry phe  */
		sphe = SymTab[hc = HashCode (phe->psb)];
		free (phe->psb);
		if (sphe == phe)
			SymTab[hc] = phe->phe;
		else
			while (sphe != NULL)
				if (sphe->phe == phe) {
					sphe->phe = phe->phe;
					return;
				} else
					sphe = sphe->phe;
	}
}


/*
 *	HashCode () -- Returns the hash code of the string in sb by adding 
 *		       the character values and applying mod by the hash 
 *		       table size.
 *
 */
int
HashCode (sb)
char sb[];
{
	int ccSum = 0;			/* sum of char values in string sb */
	int i;

	for (i = 0; sb[i]; i++)		/* add char codes for sb chars     */
		ccSum += sb[i];
	return (ccSum % SST);		/* return sum mod table size	   */
}
