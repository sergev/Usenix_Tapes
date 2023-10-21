/*	$Header: /usr/src/local/etc/ease/RCS/strops.c,v 1.2 85/10/29 23:45:39 jss Exp $	*/

/*
 *	strops.c   -- Contains string operation routines used for constructing
 *		      definitions in cf format.
 *
 *	author	   -- James S. Schoner, Purdue University Computing Center,
 *				        West Lafayette, Indiana  47907
 *
 *	date	   -- July 9, 1985
 *
 *	Copyright (c) 1985 by James S. Schoner
 *
 *	All rights reserved.
 *
 */

#include <stdio.h>
#include <strings.h>
#include "symtab.h"

#define MAXTOKPOS   99		/* maximum number of token positions */
#define MAXNAME	    1024	/* maximum length of an identifier   */

extern struct he *LookupSymbol ();
extern char       MakeMac ();
extern void	  FatalError (),
		  PrintError (),
		  ErrorReport ();

short  Rformat = FALSE;			/* class read format flag	  */
static char   *Ptok   = "$  ";		/* positional token structure     */
static char   *Cfield = "$= ";		/* class reference structure	  */
static char   *Ofield = "$-";		/* one token match structure	  */
static char   *Zfield = "$*";		/* zero or more tokens structure  */
static char   *Pfield = "$+";		/* one or more tokens structure	  */
static char   *Mtest  = "$? ";		/* conditional macro test string  */


/*
 *	ConvOpt () -- Convert an Ease option identifier (optid) by returning a
 *		      string representation of the cf format.  
 *
 */
char *
ConvOpt (optid) 
register enum opts optid;
{
	switch (optid) {
		case opt_A  :	return ("A");
		case opt_a  :	return ("a");
		case opt_B  :	return ("B");
		case d_opt_b:	return ("b");
		case opt_c  :	return ("c");
		case opt_D  :	return ("D");
		case opt_d  :	return ("d");
		case opt_e  :
		case e_opt_e:	return ("e");
		case opt_F  :	return ("F");
		case opt_f  :	return ("f");
		case opt_g  :	return ("g");
		case opt_H  :	return ("H");
		case opt_i  :
		case d_opt_i:	return ("i");
		case opt_L  :	return ("L");
		case opt_m  :
		case e_opt_m:	return ("m");
		case opt_N  :	return ("N");
		case opt_o  :	return ("o");
		case e_opt_p:	return ("p");
		case opt_Q  :	return ("Q");
		case d_opt_q:	return ("q");
		case opt_r  :	return ("r");
		case opt_S  :	return ("S");
		case opt_s  :	return ("s");
		case opt_T  :	return ("T");
		case opt_t  :	return ("t");
		case opt_u  :	return ("u");
		case opt_v  :	return ("v");
		case opt_W  :	return ("W");
		case e_opt_w:	return ("w");
		case opt_x  :	return ("x");
		case opt_X  :	return ("X");
		case e_opt_z:	return ("z");
		default     :	FatalError ("Bad case in ConvOpt ()", (char *) NULL);
	}
	/*NOTREACHED*/
}


/*
 *	ConvFlg () -- Convert an Ease mailer flag identifier (flgid) by 
 *		      string representation of the cf format.  
 *
 */
char *
ConvFlg (flgid)
register enum flgs flgid;	/* flag identifier */
{
	switch (flgid) {
		case flg_f:	return ("f");
		case flg_r:	return ("r");
		case flg_S:	return ("S");
		case flg_n:	return ("n");
		case flg_l:	return ("l");
		case flg_s:	return ("s");
		case flg_m:	return ("m");
		case flg_F:	return ("F");
		case flg_D:	return ("D");
		case flg_M:	return ("M");
		case flg_x:	return ("x");
		case flg_P:	return ("P");
		case flg_u:	return ("u");
		case flg_h:	return ("h");
		case flg_A:	return ("A");
		case flg_U:	return ("U");
		case flg_e:	return ("e");
		case flg_X:	return ("X");
		case flg_L:	return ("L");
		case flg_p:	return ("p");
		case flg_I:	return ("I");
		case flg_C:	return ("C");
		default   :	FatalError ("Bad case in ConvFlg ()", (char *) NULL);
	}
	/*NOTREACHED*/
}


/*
 *	ConvMat () -- Convert an Ease mailer attribute (mat) by returning a
 *		      string representation of the cf format.  
 *
 */
char *
ConvMat (mat)
register enum mats mat;		/* mailer attribute flag */
{
	switch (mat) {
		case mat_path		: return ("P");
		case mat_flags		: return ("F");
		case mat_sender		: return ("S");
		case mat_recipient	: return ("R");
		case mat_argv		: return ("A");
		case mat_eol		: return ("E");
		case mat_maxsize	: return ("M");
		default			: FatalError ("Bad case in ConvMat ()", (char *) NULL);
	}
	/*NOTREACHED*/
}


/*
 *	MacScan () -- Scan a string (pstring) for macros, replacing the Ease
 *		      form with the one-character form required by cf format.
 *
 */
char *
MacScan (pstring)
char *pstring;		/* macro expandable string */
{
	register char *searchptr;	/* string search pointer 	*/
	register char *bptr, *eptr;	/* macro begin and end pointers */
	char macname [MAXNAME];		/* macro name buffer		*/

	if ((searchptr = pstring) == NULL)
		return ((char *) NULL);
	while (*searchptr != '\0') 	/* find and rewrite all macros  */
		if (*searchptr == '\\') {
			searchptr = searchptr + 2;
			continue;
		} else if (*searchptr++ == '$' && *searchptr == '{') {
			if (sscanf (searchptr + 1, "%[^}]", macname) != 1) {
				PrintError ("Invalid macro format:", searchptr + 1);
				return ((char *) NULL);
			} 
			*searchptr++ = MakeMac (LookupSymbol (macname), ID_MACRO);
			bptr = eptr = searchptr;
			while (*eptr++ != '}')	/* delete old macro chars */
				/* empty */ ;
			do
				*bptr++ = *eptr;
			while (*eptr++ != '\0');
		}
	return (pstring);
}


/*
 *	MakeRStr () -- Construct and return a pointer to a class read string 
 *		       using the filename fname and read format rformat.  
 *
 */
char *
MakeRStr (fname, rformat)
char *fname,			/* file name for class read */
     *rformat;			/* format for class read    */
{
	register char *res;	/* resultant read string    */

	Rformat = TRUE;		/* set read format flag     */
	if (rformat == NULL)
		return (fname);
	res = (char *) realloc (fname, strlen (fname) + strlen (rformat) + 2);
	if (res == NULL)
		FatalError ("System out of string space in MakeRStr ()", (char *) NULL);
	res = strcat (res, " ");	/* construct read string */
	res = strcat (res, rformat);
	free (rformat);
	return (res);
}


/*
 *	ListAppend () -- Append string list2 to string list1 using the 
 *			 separator sep.  A pointer to the newly constructed
 *			 string is returned.
 *
 */
char *
ListAppend (list1, list2, sep)
char *list1,			/* first string 	*/
     *list2,			/* second string  	*/
     *sep;			/* string separator	*/
{
	register char *res;	/* resultant string	*/

	res = (char *) malloc (strlen (list1) + strlen (list2) + strlen (sep) + 1);
	if (res == NULL)
		FatalError ("System out of string space in ListAppend ()", (char *) NULL);
	res = strcpy (res, list1);
	if (list1 != NULL)	/* use separator if first string not null */
		res = strcat (res, sep);
	res = strcat (res, list2);
	return (res);
}


/*
 *	MakeCond () -- Construct a macro conditional string in cf format.  The
 *		       conditional is based on the macro testmac, with an "if
 *		       set" result ifstring, which may contain an optional 
 *		       "if not set" result string appended to it.
 *
 */
char *
MakeCond (testmac, ifstring)
struct he *testmac;		/* macro for conditional testing 	     */
char 	  *ifstring; 		/* "if macro set" result string(s)  	     */
{
	register char *res;	/* resultant conditional string		     */

	Mtest[2] = MakeMac (testmac, ID_MACRO);	   /* get one-char macro rep */
	res = (char *) malloc (strlen (ifstring) + 6);
	if (res == NULL)
		FatalError ("System out of string space in MakeCond ()", (char *) NULL);
	res = strcpy (res, Mtest);
	res = strcat (res, ifstring);		/* build result part	  */
	res = strcat (res, "$.");		/* end of conditional     */
	free (ifstring);
	return (res);
}


/*
 *	MakePosTok () -- Construct and return a positional token string 
 *			 representation from the parameter num.
 *
 */
char *
MakePosTok (num)
register int num;	        /* numerical value of positional token */
{
	if (num > MAXTOKPOS) {
		ErrorReport ("Positional token too large.\n");
		return ((char *) NULL);
	} else {
		if (num > 9) {	/* two-digit positional token */
			Ptok[1] = '0' + (num / 10);
			Ptok[2] = '0' + (num % 10);
			Ptok[3] = '\0';
		} else {
			Ptok[1] = '0' + num;
			Ptok[2] = '\0';
		}
	return (Ptok);
	}
}


/*
 *	Bracket () -- Construct and return a cf string form of the 
 *		      canonicalization of the string identifier passed in
 *		      the string parameter psb if dflag is true, else
 *		      simply bracket the identifier without dollar signs
 *		      for numeric hostname specifications.
 *
 */
char *
Bracket (psb, dflag)
char *psb;			/* identifier to be canonicalized */
short dflag;			/* dollar flag 			  */
{
	register char *res;	/* resultant cf form 		  */
	register short extra;	/* extra space needed for malloc  */
	
	extra = dflag ? 5 : 3;
	res = (char *) malloc (strlen (psb) + extra);
	if (res == NULL)
		FatalError ("System out of string space in Bracket ()", (char *) NULL);
	if (dflag)
		res = strcpy (res, "$[");
	else
		res = strcpy (res, "[");
	res = strcat (res, psb);
	if (dflag)
		res = strcat (res, "$");
	res = strcat (res, "]");
	return (res);
}


/*
 *	MakeRSCall () -- Construct and return a cf string form of a call
 *			 to a ruleset (cid), which would pass to it the
 *			 remainder of a rewriting address (rwaddr).
 *
 */
char *
MakeRSCall (cid, rwaddr)
register struct he *cid;	/* called ruleset identifier	     */
register char *rwaddr;		/* remainder of rewriting address    */
{
	register char *res;	/* resultant cf string for the call  */
	
	if (!ISRULESET(cid->idtype)) {	/* check validity of ruleset */
		PrintError ("Undefined ruleset identifier:", cid->psb);
		return ((char *) NULL);
	}
	res = (char *) malloc (strlen (cid->idval.rsn) + strlen (rwaddr) + 3);
	if (res == NULL)
		FatalError ("System out of string space in MakeRSCall ()", (char *) NULL);
	res = strcpy (res, "$>");	/* construct the call string */
	res = strcat (res, cid->idval.rsn);
	res = strcat (res, rwaddr);
	return (res);
}


/*
 *	MakeField () -- Construct and return the cf string format for a
 *			field variable.  The match count (count), an optional
 *			class (class), and a match repetition flag (fstar)
 *			are used to determine what type of field string to
 *			construct.
 *
 */
char *
MakeField (count, class, fstar)
register int count;		/* match count (0 or 1) */
register struct he *class;	/* optional class type  */
register short fstar;		/* repetition flag	*/
{
	switch (count) {
		case 0:	  if (class == NULL)	/* any token is valid */
				if (fstar)
					return (Zfield);
				else {
					ErrorReport ("Invalid field type.\n");
					return ((char *) NULL);
				}
			  else {		/* match 0 from class */
				Cfield[1] = '~';
				Cfield[2] = MakeMac (class, ID_CLASS);
				return (Cfield);
			  }
		case 1:	  if (class == NULL)	/* any token is valid */
				if (fstar)
					return (Pfield);
				else
					return (Ofield);
			  else {		/* match 1 from class */
				Cfield[1] = '=';
				Cfield[2] = MakeMac (class, ID_CLASS);
				return (Cfield);
			  }
		default:  ErrorReport ("Invalid field type.\n");
	}
	/*NOTREACHED*/
}
