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

long int PutRec (buf,size)
  char *buf ;
  int size ;
{
	long int loc ;

	loc = RecLoc ;
	(void) lseek (rfd,loc,0) ;
	if ( write (rfd,buf,size) != size )
		return (ERROR) ;
	RecLoc += ((long) size) ;
	return (loc) ;
}
