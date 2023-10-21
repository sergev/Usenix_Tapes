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

#include "adefs.h"

int command ()
{
	register int i, k ;
	int cnt, val ;

	linlen = 0 ;
	for ( i = 0 ; i < LINELEN ; i++ )
		linewd[i] = -1 ;
	
	cnt = readln (lex) ;

	for ( i = 0 ; i < cnt ; i++ )
	{
		low2up (lex[i]) ;
		val = find(lex[i]) ;
		if ( class(val) == NULL )
			continue ;
		linewd[linlen++] = val ;
	}

	for ( i = 0 ; i < LINELEN ; i++ )
	{
		switch (class(linewd[i]))
		{
			case OBJECT:
			case PLACE:
				k = bitval(linewd[i]) ;
				break ;

			case VERB:
				k = XVERB ;
				break ;

			default:
				k = 0 ;
				break ;
		}
		if ( linewd[i] < 0 )
			k = BADWORD ;
		setval (argwd[i],linewd[i]) ;
		setbit (argwd[i],k) ;
	}

	setval (status,linlen) ;
	if ( linlen <= 0 )
		return ;
	
	switch ( class(linewd[0]) )
	{
		case VERB:
			setbit(status,(bitval(status)|XVERB)) ;
			break ;

		case OBJECT:
			setbit(status,(bitval(status)|XOBJECT)) ;
			break ;

		case PLACE:
			setbit(status,(bitval(status)|XPLACE)) ;
			break ;
	}

	return ;
}

int low2up (word)
  register char *word ;
{
	for ( ; *word ; word++ )
		if ( (*word) >= 'a' && (*word) <= 'z' )
			*word += ( 'A' - 'a' ) ;
	return ;
}

#undef NULL
#undef EOF
#include <stdio.h>
#define LINESIZE	134

int readln (words)
  char words[LEXLEN][LEXSIZ] ;
{
	register int cnt, i ;
	register char *b ;
	char buffer[LINESIZE] ;

	printf ("? ") ;

	if ( fgets (buffer,LINESIZE,stdin) == NULL )
		exit (1) ;

#ifdef STATUS
	if ( strcmp (buffer,"~status\n") == 0 )
	{
		pcstat () ;
		(void) strcpy (buffer,"LOOK\n") ;
	}
#endif STATUS

	cnt = 0 ;
	for ( cnt = 0, b = buffer ; *b != '\n' && *b != '\0' ; )
	{
		while ( *b == ' ' || *b == '\t' )
			b++ ;
	
		for ( i = 0 ; *b != ' ' && *b != '\t' && *b != '\n' && *b != '\0' ; b++ )
			if ( i < (LEXSIZ-1) )
				words[cnt][i++] = *b ;
	
		words[cnt][i] = EOS ;
		if ( i > 0 )
			if ( ++cnt >= LEXLEN )
				break ;
	}
	return (cnt) ;
}
