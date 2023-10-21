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

int GetRec (loc,buf,size)
  long int loc ;
  char *buf ;
  short int size ;
{
	(void) lseek (rfd,loc,0) ;
	if ( read (rfd,buf,size) != size )
		return (ERROR) ;
	buf[size] = 0 ;
	return (size) ;
}
