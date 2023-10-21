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

#include "kio.h"

#define READWRITE	2

int creatk (name)
  char *name ;
{
	register int i ;

	(void) MakNam (name) ;

	if ( ( kfd = creat (knam,0600) ) < 0 ||
	     ( rfd = creat (rnam,0600) ) < 0 )
		error ("Creatk","unable to create files %s!",name) ;

	for ( i = 0 ; i < MAXIBLK ; i++ )
		Sblk[i] = EMPTY ;

	(void) lseek (kfd,0L,0) ;
	if ( write (kfd,Sblk,sizeof(Sblk)) != sizeof(Sblk) )
		error ("Creatk","%s -- error on initializing!",knam) ;

	(void) close (kfd) ;
	(void) close (rfd) ;

	if ( ( kfd = open (knam,READWRITE) ) < 0 ||
	     ( rfd = open (rnam,READWRITE) ) < 0 )
		error ("Creatk","%s -- error reopening!",name) ;

	return (NEW) ;
}
