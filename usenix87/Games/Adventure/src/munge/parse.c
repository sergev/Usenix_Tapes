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

struct cm
{
	char *c_nam ;
	char c_num ;
} ;

struct cm commnd [MAXCOM] =
{
	"ACTION",	ACTION,
	"AT",		AT,
	"DEFINE",	DEFINE,
	"INCLUDE",	INCLUDE,
	"INITIAL",	INITIAL,
	"LABEL",	LABEL,
	"LIST",		LIST,
	"NOLIST",	NOLIST,
	"NULL",		NULL,
	"OBJECT",	OBJECT,
	"PLACE",	PLACE,
	"REPEAT",	REPEAT,
	"SYNONYM",	SYNON,
	"TEXT",		TEXT,
	"VARIABLE",	VARIABLE,
	"VERB",		VERB
} ;


int parse ()
{
	register int m ;

	m = getline (token,MAXLINE) ;
	for ( ; m != EOF ; m = getline (token,MAXLINE) )
	{
		if ( m == MAJOR && gettok (token,MAXLINE) == OK )
			exec (major(token)) ;
	}
	return ;
}

int getline (line,msize)
  char line[] ;
  int msize ;
{
	register int c, i ;

	line[0] = EOS ;

	if ( chkmaj () == MAJOR )
		return (MAJOR) ;

	c = skip () ;
	if ( c == PERCENT )
	{
		Ungetc (BLANK) ;
		return (NEXT) ;
	}

	if ( c == SLASH )
		c = Getc () ;
	
	for ( i = 0 ; i < msize ; i++ )
	{
		line[i] = c ;

		if ( c == NEWLINE )
		{
			line[i+1] = EOS ;
			return (OK) ;
		}
		if ( c == EOF )
			return (EOF) ;
		c = Getc () ;
	}
	Ungetc (BLANK) ;
	line[i] = EOS ;
	synerr ("Getline","line is too long (%d)!",msize) ;

	return (OK) ;
}

int gettok (tok,msize)
  char tok[] ;
  int msize ;
{
	register int c, i ;

	tok[0] = EOS ;
	if ( ( c = skip () ) == NEWLINE || c == EOF )
		return (c) ;

	for ( i = 0 ; i < msize ; i++ )
	{
		tok[i] = c ;
		if ( c == NEWLINE || c == EOF || sep(c) )
		{
			Ungetc (c) ;
			tok[i] = EOS ;
			return (OK) ;
		}
		c = Getc () ;
	}

	Ungetc (BLANK) ;
	tok[i] = EOS ;
	synerr ("Gettok","token is too long (%d)!",msize) ;
	return (OK) ;
}

int major (tok)
  char *tok ;
{
	register int i ;
	struct cm c ;

	for ( i = 0 ; i < MAXCOM ; i++ )
	{
		if ( strncmp (commnd[i].c_nam,tok,MATCHCOM) == 0 )
		{
			if ( i )
			{
				c = commnd[i] ;
				commnd[i] = commnd[0] ;
				commnd[0] = c ;
			}
			return (commnd[0].c_num) ;
		}
	}
	return (ERROR) ;
}

int exec (cmd)
  int cmd ;
{

	switch (cmd)
	{
		case ACTION:
			act () ;
			break ;
		case AT:
			at () ;
			break ;
		case DEFINE:
			def () ;
			break ;
		case INCLUDE:
			inc () ;
			break ;
		case INITIAL:
			init () ;
			break ;
		case LABEL:
			lab () ;
			break ;
		case LIST:
			list = YES ;
			break ;
		case NOLIST:
			list = NO ;
			break ;
		case NULL:
			null () ;
			break ;
		case OBJECT:
			obj () ;
			break ;
		case PLACE:
			place () ;
			break ;
		case REPEAT:
			rep () ;
			break ;
		case SYNON:
			syn () ;
			break ;
		case TEXT:
			text () ;
			break ;
		case VARIABLE:
			var () ;
			break ;
		case VERB:
			verb () ;
			break ;
		default:
			error ("Exec","%d -- Unrecognised Command.",cmd) ;
			break ;
	}
	return ;
}

#define isalpha(c)	(((c)>='a'&&(c)<='z')||((c)>='A'&&(c)<='Z'))

int chkmaj ()
{
	register int c ;

	for ( c = Getc() ; com(c) ; c = Getc() )	/* flush comments */
		while ( (c=Getc()) != NEWLINE )
			if ( c == EOF )
				return (OK) ;
	Ungetc (c) ;
	if ( isalpha(c) )
		return (MAJOR) ;
	return (OK) ;
}

int skip ()
{
	register int c ;

	for ( c = Getc () ; sep(c) ; c = Getc () )
		;

	if ( !com(c) )
		return (c) ;

	flushline () ;
	return (NEWLINE) ;
}
