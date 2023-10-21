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

int flushline ()
{
	register int c ;

	while ( ( c = Getc () ) != '\n' )
		if ( c == EOF )
			break ;
	return ;
}

int type (key)

  register int key ;

{
	if ( key < 0 || key > MAXKEY )
		return (0) ;
	return (clss[key/1000]) ;
}

#define STKSIZ		5

static int stack[STKSIZ] ;
static short int sp = 0 ;

int push (value)
  int value ;
{
	if ( sp >= STKSIZ )
		return (ERROR) ;
	stack[sp++] = value ;
	return (value) ;
}

int pop ()
{
	if ( sp < 1 )
		return (ERROR) ;
	return ( stack[--sp] ) ;
}
