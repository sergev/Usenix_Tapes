#ifndef lint
static char *RCSid = "$Header: ident.c,v 1.1 87/04/15 21:29:14 josh Exp $";
#endif

/*
 *------------------------------------------------------------------
 * Copyright 1987, Josh Siegel
 * All rights reserved.
 *
 * Josh Siegel (siegel@hc.dspo.gov)
 * Dept of Electrical and Computer Engineering,
 * University of New Mexico,
 * Albuquerque , New Mexico
 * (505) 277-2497
 *
 *------------------------------------------------------------------
 *
 * $Source: /usr/pcb/josh/other/network/RCS/ident.c,v $
 * $Revision: 1.1 $
 * $Date: 87/04/15 21:29:14 $
 * $State: Exp $
 * $Author: josh $
 * $Locker: josh $
 *
 *------------------------------------------------------------------
 *
 * $Log:	ident.c,v $
 * Revision 1.1  87/04/15  21:29:14  josh
 * Initial revision
 * 
 *
 *------------------------------------------------------------------
 */


#include <pwd.h>

/*
whoami:  Returns a string of who the caller is in the
	form name@machine where machine is in the
	form returned by gethostname.

*/
char *hostname()
{
	static char buff[255];

	gethostname(buff,255);

	return(buff);
}

char *whoami()
{
	static char buff[64];
	struct passwd *pass;

	pass=getpwent();

	sprintf(buff,"%s@%s",pass->pw_name,hostname());

	return(buff);
}
