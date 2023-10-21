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

int dupk (unit,oldkey,newkey)
  int unit ;
  int oldkey, newkey ;
{
	struct mkey k ;
	long int loc ;
	register int siz ;

	if ( unit == EXISTING )
		return (ERROR) ;

	MapKey (oldkey,&k) ;

	if ( GetBlk (k.b) != k.b || Iblk.i_siz[k.e] <= 0 )
		return (ERROR) ;
	
	loc = Iblk.i_loc[k.e] ;
	siz = Iblk.i_siz[k.e] ;

	MapKey (newkey,&k) ;

	if ( GetBlk (k.b) != k.b )
		if ( MakBlk (k.b) != k.b )
			error ("Dupk","unable to create key %d!",newkey) ;

	if ( Iblk.i_siz[k.e] > 0 )
		error ("Dupk","key %d already exists!",newkey) ;

	Iblk.i_loc[k.e] = loc ;
	Iblk.i_siz[k.e] = siz ;
	Modified++ ;

	return (siz) ;
}
