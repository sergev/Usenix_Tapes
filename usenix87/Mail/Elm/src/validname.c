/**			validname.c			**/

/** This routine takes a single address, no machine hops or
    anything, and returns 1 if it's valid and 0 if not.  The
    algorithm it uses is the same one that uux uses, namely:

	1. Is there a file '/usr/mail/%s'?  
	2. Is there a password entry for %s?
	
   (C) Copyright 1986 Dave Taylor 
**/

#include "defs.h"

#include <stdio.h>

#ifndef NOCHECK_VALIDNAME
#  ifdef BSD4.1
#    include <sys/pwd.h>
#  else
#    include <pwd.h>
#  endif
#endif

int
valid_name(name)
char *name;
{
	/** does what it says above, boss! **/

#ifdef NOCHECK_VALIDNAME

	return(1);		/* always say it's okay! */

#else
	struct passwd *getpwname();
	char filebuf[SLEN];

	sprintf(filebuf,"%s/%s", mailhome, name);
	
	if (access(filebuf, ACCESS_EXISTS) == 0)
	  return(1);

	if (getpwnam(name) != NULL)
	  return(1);

	return(0);

#endif

}
