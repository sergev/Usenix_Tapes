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

int MakNam (name)
  char *name ;
{
	(void) strncpy (knam,name,10) ;
	(void) strncpy (rnam,name,10) ;
	(void) strncat (knam,".key",20) ;
	(void) strncat (rnam,".rec",20) ;

	return ;
}
