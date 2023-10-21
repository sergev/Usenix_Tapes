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

main ()
{
	srandom (time(0)+getpid()) ;

	setup () ;
	opendb (ADV) ;
	webster () ;

#ifdef PSTAB
	pstab () ;
#endif PSTAB

	process (INITIAL) ;

	forever
		process (REPEAT) ;
}

int setup ()
{
	register int i ;

	for ( i = 0 ; i < OBJECTS ; i++ )
	{
		objval[i] = 0 ;
		objbit[i] = XOBJECT ;
		objloc[i] = 0 ;
	}
	
	for ( i = 0 ; i < PLACES ; i++ )
	{
		plcbit[i] = XPLACE ;
	}

	for ( i = 0 ; i < VARS ; i++ )
	{
		varval[i] = 0 ;
		varbit[i] = XVERB ;
	}

#ifdef CACHE
	ClrCache () ;
#endif CACHE

	return ;
}

int opendb(name)
  char *name ;
{
	dbunit = openk (name) ;

	if ( dbunit == ERROR )
		error ("Opendb","unable to open %s!",name) ;
	return ;
}
