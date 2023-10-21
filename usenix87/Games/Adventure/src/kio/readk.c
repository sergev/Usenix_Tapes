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

int readk (unit,key,buffer,size)
  int unit, key, size ;
  char *buffer ;
{
	struct mkey k ;

	MapKey (key,&k) ;

	if ( GetBlk (k.b) != k.b || Iblk.i_siz[k.e] == 0 )
		return (ERROR) ;

	if ( Iblk.i_siz[k.e] > size )
		error ("Readk","buffer too small (%d<%d)!",size,Iblk.i_siz[k.e]) ;

	return (GetRec(Iblk.i_loc[k.e],buffer,Iblk.i_siz[k.e])) ;
}
