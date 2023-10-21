/*
**		    Copyright (c) 1985	Ken Wellsch
**
**     Permission is hereby granted to all users to possess, use, copy,
**     distribute, and modify the programs and files in this package
**     provided it is not for direct commercial benefit and secondly,
**     that this notice and all modification information be kept and
**     maintained in the package.
**
*/

#include "mdefs.h"

static short int nxtint =     0 ;
static short int nxtrep =   500 ;
static short int nxtlab =  1000 ;
static short int nxtvrb =  2000 ;
static short int nxtplc =  4000 ;
static short int nxttxt =  7000 ;
static short int nxtobj =  8000 ;
static short int nxtvar = 11000 ;


int act ()
{
	register struct symstr *p ;
	register int key ;
	int interval, limit ;
	extern struct symstr *lookup () ;

	if ( gettok (token,MAXLINE) != OK )
	{
		synerr ("Action","Missing Verb name.") ;
		return ;
	}

	if ( ( p = lookup (token) ) == NIL )
	{
		synerr ("Action","%s -- Undefined Verb.",token) ;
		return ;
	}

	if ( type (p->s_val) == OBJECT )
	{
		if ( p->s_aux < MAXOTEXT )
			p->s_aux = MAXOTEXT ;
		interval = MAXOBJECTS ;
		limit = MAXOTEXT + MAXOCODE ;
	}
	else
	{
		interval = MAXVERBS ;
		limit = MAXVCODE ;
	}

	key = (p->s_val) + ( (p->s_aux) * interval ) ;

	if ( type (key) != VERB && type (key) != OBJECT )
	{
		synerr ("Action","%s -- that's no action!",token) ;
		return ;
	}

	if ( (++(p->s_aux)) >= limit )
		error ("Action","too many code definitions for %s (%d)!",
			token,MAXOCODE) ;

	if ( gettok (token,MAXLINE) == OK )
	{
		pbstr (token) ;
		pbstr (" KEYWORD ") ;
	}

	compile (key) ;
	return ;
}

int at ()
{
	register struct symstr *p ;
	register int key ;
	extern struct symstr *lookup () ;

	if ( gettok (token,MAXLINE) != OK )
	{
		synerr ("At","Missing Place name.") ;
		return ;
	}

	if ( ( p = lookup (token) ) == NIL )
	{
		synerr ("At","%s -- Undefined Place.",token) ;
		return ;
	}

	key = (p->s_val) + ((p->s_aux) * MAXPLACES) ;

	if ( (++(p->s_aux)) >= MAXPCODE )
		error ("At","too many code definitions for %s (%d)!",
			token,MAXPCODE) ;

	if ( type (key) != PLACE )
	{
		synerr ("At","%s -- that's no Place!",token) ;
		return ;
	}

	flushline () ;
	compile (key) ;
	return ;
}

int def ()
{
	register struct symstr *p ;
	extern struct symstr *lookup () ;

	while ( gettok (token,MAXLINE) == OK )
	{
		if ( ( p = lookup (token) ) == NIL )
		{
			synerr ("Define","%s -- Undefined symbol.",token) ;
			return ;
		}
		p->s_mod = KEEP ;
	}
	return ;
}

#include <strings.h>

int inc ()
{
	register char *s ;

	if ( gettok (token,MAXLINE) != OK )
	{
		synerr ("Include","Missing file name.") ;
		return ;
	}

	if ( push (inunit) == ERROR )
	{
		synerr ("Include","include's nested too deeply!") ;
		return ;
	}

	if ( ( inunit = openf (token) ) == ERROR )
	{
		synerr ("Include","%s -- unable to open file.",token) ;
		inunit = pop () ;
		return ;
	}

	if ( ( s = rindex (token,'/') ) == (char *)0 )
		errout ("%s:\n",token) ;
	else
		errout ("%s:\n",&(s[1])) ;

	flushline () ;
	return ;
}

int init ()
{
	flushline () ;
	compile (nxtint) ;
	nxtint ++ ;
	ninit ++ ;
	return ;
}

int lab ()
{
	extern struct symstr *define () ;

	if ( gettok (token,MAXLINE) != OK )
	{
		synerr ("Label","Missing Label name.") ;
		return ;
	}

	if ( define (token,nxtlab,status(nxtlab)) == NIL )
	{
		synerr ("Label","%s -- duplicate label name.",token) ;
		return ;
	}

	flushline () ;
	compile (nxtlab) ;
	nxtlab++ ;
	return ;
}

int null ()
{
	extern struct symstr *define () ;

	while ( gettok (token,MAXLINE) == OK )
	{
		if ( define (token,NULLWORD,KEEP) == NIL )
		{
			synerr ("Null","%s -- duplicate Null symbol",token) ;
			return ;
		}
	}
	return ;
}

int obj ()
{
	register struct symstr *p ;
	register int m, n ;
	extern struct symstr *define () ;

	if ( gettok (token,MAXLINE) != OK )
	{
		synerr ("Object","Missing Object name.") ;
		return ;
	}

	if ( ( p = define (token,nxtobj,status(nxtobj)) ) == NIL )
	{
		synerr ("Object","%s -- duplicate Obect name.",token) ;
		return ;
	}

	flushline () ;
	n = 0 ;
	m = getline (token,MAXLINE) ;
	if ( m == NEXT )
	{
		n++ ;
		m = getline (token,MAXLINE) ;
	}
	if ( m == OK && token[0] == NEWLINE )
	{
		while ( getline (token,MAXLINE) == OK )
			;
		m = getline (token,MAXLINE) ;
		n++ ;
	}
	for ( ; m != MAJOR && m != EOF ; m = getline (token,MAXLINE) )
	{
		clrtext () ;
		do
		{
			if ( strncmp (token,NULLOBJECT,3) == 0 )
			{
				clrtext () ;
				break ;
			}
			apptext (token) ;
		}
		while ( getline (token,MAXLINE) == OK ) ;

		outtext (nxtobj+((n++)*MAXOBJECTS)) ;
	}

	if ( n > 0 )
		n-- ;

	p->s_aux = n ;

	nxtobj++ ;
	nobj ++ ;

	return ;
}

int place ()
{
	register struct symstr *p ;
	extern struct symstr *define () ;

	if ( gettok (token,MAXLINE) != OK )
	{
		synerr ("Place","Missing Place name.") ;
		return ;
	}

	if ( ( p = define (token,nxtplc,status(nxtplc)) ) == NIL )
	{
		synerr ("Place","%s -- duplicate Place name.",token) ;
		return ;
	}
	flushline () ;

	if ( getline (token,MAXLINE) == OK )	/* short description */
	{
		if ( strncmp (token,LINKPLACE,3) == 0 )
		{
			plink (token,SHORTDESC) ;
			while ( getline (token,MAXLINE) == OK )
				;
		}
		else
		{
			clrtext () ;
			do
			{
				apptext (token) ;
			}
			while ( getline (token,MAXLINE) == OK ) ;
			outtext (nxtplc) ;
		}
	}

	if ( getline (token,MAXLINE) == OK )	/* long description */
	{
		if ( strncmp (token,LINKPLACE,3) == 0 )
			plink (token,LONGDESC) ;
		else
		{
			clrtext () ;
			do
			{
				apptext (token) ;
			}
			while ( getline (token,MAXLINE) == OK ) ;
			outtext (nxtplc+MAXPLACES) ;
		}
	}

	p->s_aux = 2 ;
	nxtplc ++ ;
	nplace ++ ;

	return ;
}

int plink (line,which)
  char *line ;
  int which ;
{
	char tok1[32] ;
	char tok2[32] ;
	register struct symstr *p ;
	register int oldkey, newkey ;
	extern struct symstr *lookup () ;

	pbstr (line) ;

	if ( gettok (tok1,32) != OK || gettok (tok1,32) != OK )
	{
		synerr ("Plink","invalid link(%s)!",LINKPLACE) ;
		return ;
	}

	if ( tok1[0] == 'd' || tok1[0] == 'D' )
	{
		if ( which == SHORTDESC )
		{
		  synerr ("Plink","impossible to `ditto' short description!") ;
		  return ;
		}
		
		if ( dupk (dbunit,nxtplc,(nxtplc+MAXPLACES)) == ERROR )
		{
			synerr ("Plink","unable to link to %d!",nxtplc) ;
			return ;
		}
		flushline () ;
		return ;
	}

	if ( gettok (tok2,32) != OK )
	{
		synerr ("Plink","missing place name!") ;
		return ;
	}
	flushline () ;

	if ( ( p = lookup (tok2) ) == NIL )
		synerr ("Plink","%s -- undefined place name!",tok2) ;
	
	if ( tok1[0] == 's' || tok1[0] == 'S' )
		oldkey = p->s_val ;
	else
		oldkey = ( (p->s_val) + MAXPLACES ) ;
	
	if ( which == SHORTDESC )
		newkey = nxtplc ;
	else
		newkey = nxtplc + MAXPLACES ;

	if ( dupk (dbunit,oldkey,newkey) == ERROR )
	{
		synerr ("Plink","unable to link %d to %d!",oldkey,newkey) ;
		return ;
	}

	return ;
}

int rep ()
{
	flushline () ;
	compile (nxtrep) ;
	nxtrep ++ ;
	nrep ++ ;

	return ;
}

int syn ()
{
	register int val ;
	extern struct symstr *define () ;

	if ( gettok (token,MAXLINE) != OK )
	{
		synerr ("Synonym","Missing symbol.") ;
		return ;
	}

	val = eval (token) ;

	while ( gettok (token,MAXLINE) == OK )
	{
		if ( define (token,val,status(val)) == NIL )
		{
			synerr ("Synonym","%s -- duplicate name.",token) ;
			return ;
		}
	}
	return ;
}

int text ()
{
	extern struct symstr *define () ;

	if ( gettok (token,MAXLINE) == OK )
	{
		if ( define (token,nxttxt,status(nxttxt)) == NIL )
		{
			synerr ("Text","%s -- duplicate Text name.") ;
			return ;
		}
		flushline () ;
	}

	clrtext () ;

	while ( getline (token,MAXLINE) == OK )
		apptext (token) ;

	outtext (nxttxt) ;

	nxttxt ++ ;

	return ;
}

int var ()
{
	extern struct symstr *define () ;

	while ( gettok (token,MAXLINE) == OK )
	{
		if ( define (token,nxtvar,status(nxtvar)) == NIL )
		{
			synerr ("Variable","%s -- duplicate symbol.",token) ;
			return ;
		}
		nxtvar++ ;
		nvars ++ ;
	}
	return ;
}

int verb ()
{
	extern struct symstr *define () ;

	if ( gettok (token,MAXLINE) != OK )
	{
		synerr ("Verb","Missing symbol.") ;
		return ;
	}

	if ( define (token,nxtvrb,status(nxtvrb)) == NIL )
	{
		synerr ("Verb","%s -- duplicate symbol.",token) ;
		return ;
	}

	while ( gettok (token,MAXLINE) == OK )
	{
		if ( define (token,nxtvrb,status(nxtvrb)) == NIL )
		{
			synerr ("Verb","%s -- duplicate synonym.",token) ;
			return ;
		}
	}
	nxtvrb++ ;
	return ;
}
