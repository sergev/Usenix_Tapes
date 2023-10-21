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

int GetBlk (b)
  int b ;
{
	long int loc ;
	extern long int MapBlk() ;

	if ( b == CurBlk )
		return (b) ;
	
	if ( Modified )
	{
		if ( PutBlk (CurBlk) != CurBlk )
			return (ERROR) ;
		Modified = 0 ;
	}

	loc = MapBlk (b) ;
	if ( loc == ERROR )
		return (ERROR) ;

	(void) lseek (kfd,loc,0) ;
	if ( read (kfd,&Iblk,sizeof(Iblk)) != sizeof(Iblk) )
		return (ERROR) ;
	CurBlk = b ;
	return (b) ;
}
