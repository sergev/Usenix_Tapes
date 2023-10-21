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

long int MapBlk (b)
  int b ;
{
	long int loc ;

	if ( Sblk[b] == EMPTY )
		return (ERROR) ;
	loc = ((long) Sblk[b]) * sizeof(Iblk) + sizeof(Sblk) ;
	return (loc) ;
}
