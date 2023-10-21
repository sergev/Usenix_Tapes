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

int say (key)
  register int key ;
{
	register int rec ;

	if ( key == NULLTXT )
		return ;
	if ( key == BLANKTXT )
	{
		printf ("\n") ;
		return ;
	}
	if ( key == OKTXT )
	{
		printf ("Ok.\n") ;
		return ;
	}

	if ( ( rec = saykey (key) ) < 0 )
		return ;

	if ( readk (dbunit,rec,textbuf,TBUFSIZE) < 1 )
		return ;
	
	printf ("%s",textbuf) ;
	return ;
}

int sayval (key,val)
  int key, val ;
{
	register int rec, i, endb ;

	rec = saykey (key) ;

	if ( ( endb = readk (dbunit,rec,textbuf,TBUFSIZE) ) < 1 )
		error ("Sayval","bad text key %d!",key) ;
	
	for ( i = 0 ; i < endb ; i++ )
	{
		if ( textbuf[i] == REPLACE )
		{
			printf ("%d",val) ;
			continue ;
		}
		putchar(textbuf[i]) ;
	}
	return ;
}

int saykey (key)
  register int key ;
{
	register int rec, b ;

	switch (class(key))
	{
		case OBJECT:
			if ( where (key) != INHAND )
			{
				if ( eval (key) < 0 )
					return (ERROR) ;
				rec = key + ( (eval(key) + 1) * MAXOBJECTS ) ;
			}
			else
				rec = key ;
			break ;

		case PLACE:
			b = bitval (status) ;
			if ( (b&LOOKING) )
				rec = key + MAXPLACES ;
			else
				if ( (b&BRIEF) && (bitval(key)&BEEN) )
					rec = key ;
				else
					if ( (b&FAST) )
						rec = key ;
					else
						rec = key + MAXPLACES ;
			break ;

		case TEXT:
			rec = key ;
			break ;

		default:
			error ("Saykey","bad key %d!",key) ;
			break ;
	}
	return (rec) ;
}

int saynam (key,op)
  int key, op ;
{
	register int i, j ;
	int rec, endb ;
	char word[20] ;

	for ( j = 0 ; j < LINELEN ; j++ )
		if ( op == argwd[j] || op == ref(argwd[j]) )
			break ;
	if ( j < LINELEN )
		(void) strncpy (word,lex[j],20) ;
	else
		if ( findnam (op,word) != op )
			error ("Saynam","bad name %d!",op) ;
	
	rec = saykey (key) ;
	if ( ( endb = readk (dbunit,rec,textbuf,TBUFSIZE) ) < 1 )
		error ("Saynam","bad key %d!",rec) ;

	up2low (word) ;

	for ( i = 0 ; i < endb ; i++ )
	{
		if ( textbuf[i] == REPLACE )
		{
			printf ("%s",word) ;
			continue ;
		}
		putchar (textbuf[i]) ;
	}
	return ;
}

int findnam (val,s)
  int val ;
  char *s ;
{
	register struct symstr *p ;
	register int i ;

	for ( i = 0 ; i < TABSIZ; i++ )
		for ( p = symtab[i] ; p != NIL ; p = p->s_nxt )
		{
			if ( p->s_val == val )
			{
				(void) strcpy (s,p->s_nam) ;
				return (val) ;
			}
		}
	return (ERROR) ;
}

int up2low (word)
  register char *word ;
{
	for ( ; *word ; word++ )
		if ( (*word) >= 'A' && (*word) <= 'Z' )
			*word += ( 'a' - 'A' ) ;
	return ;
}
