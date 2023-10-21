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

int MakBlk (b)
  int b ;
{
	register int i ;

	if ( Modified )
		return (ERROR) ;

	for ( i = 0 ; i < MAXENTRIES ; i++ )
	{
		Iblk.i_loc[i] = 0 ;
		Iblk.i_siz[i] = 0 ;
	}

	Sblk[b] = IndLoc++ ;
	CurBlk = b ;
	return (b) ;
}
