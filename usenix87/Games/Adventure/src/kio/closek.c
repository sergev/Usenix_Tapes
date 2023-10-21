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

int closek (unit)
  int unit ;
{
	(void) close (rfd) ;

	if ( unit == NEW )
	{
		(void) lseek (kfd,0L,0) ;
		if ( write (kfd,Sblk,sizeof(Sblk)) != sizeof(Sblk) )
			error ("Closef","error on updating super block!") ;
	}

	if ( Modified )
		if ( PutBlk(CurBlk) != CurBlk )
			error ("Closef","error updating modified block!") ;

	(void) close (kfd) ;

	return (0) ;
}
