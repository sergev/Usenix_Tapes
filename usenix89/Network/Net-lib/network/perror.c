#ifndef lint
static char *RCSid = "$Header: perror.c,v 1.1 87/04/15 21:29:26 josh Exp $";
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
 * $Source: /usr/pcb/josh/other/network/RCS/perror.c,v $
 * $Revision: 1.1 $
 * $Date: 87/04/15 21:29:26 $
 * $State: Exp $
 * $Author: josh $
 * $Locker: josh $
 *
 *------------------------------------------------------------------
 *
 * $Log:	perror.c,v $
 * Revision 1.1  87/04/15  21:29:26  josh
 * Initial revision
 * 
 *
 *------------------------------------------------------------------
 */

extern char *sys_errlist[];

char *user_errlist[255];

extern int errno;

perror(s)
char *s;
{
	if(errno<255) 
		printf("%s: %s\n",s,sys_errlist[errno]);
	 else 
		printf("%s: %s\n",s,user_errlist[errno-255]);
}
