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

#define READONLY	0

int openk (name)
  char *name ;
{
	(void) MakNam (name) ;

	if ( ( kfd = open (knam,READONLY) ) < 0 ||
	     ( rfd = open (rnam,READONLY) ) < 0 )
		return (ERROR) ;

	if ( read (kfd,Sblk,sizeof(Sblk)) != sizeof(Sblk) )
		error ("Openk","unable to read super block!") ;

	return (EXISTING) ;
}
