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

int writek (unit,key,buffer,size)
  int unit, key, size ;
  char *buffer ;
{
	struct mkey k ;
	extern long int PutRec () ;

	if ( size < 1 || key < 0 || unit == EXISTING )
		return (ERROR) ;

	MapKey (key,&k) ;

	if ( GetBlk (k.b) != k.b )
		if ( MakBlk (k.b) != k.b )
			error ("Writek","unable to create key %d!",key) ;

	if ( Iblk.i_siz[k.e] > 0 )
		error ("Writek","key %d already exists!",key) ;

	Iblk.i_loc[k.e] = PutRec (buffer,size) ;
	Iblk.i_siz[k.e] = size ;
	Modified++ ;
	return (size) ;
}
