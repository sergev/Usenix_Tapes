#ifndef lint
static char *RCSid = "$Header: isignal.c,v 1.1 87/04/15 21:29:17 josh Exp $";
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
 * $Source: /usr/pcb/josh/other/network/RCS/isignal.c,v $
 * $Revision: 1.1 $
 * $Date: 87/04/15 21:29:17 $
 * $State: Exp $
 * $Author: josh $
 * $Locker: josh $
 *
 *------------------------------------------------------------------
 *
 * $Log:	isignal.c,v $
 * Revision 1.1  87/04/15  21:29:17  josh
 * Initial revision
 * 
 *
 *------------------------------------------------------------------
 */


#include "net.h"

/*
 * Here we go... the IOSIG handler.  If this is called, we assume we got a
 * IOSIG signal and something is coming down one of the pipes.  Only testing
 * will prove if it is otherwise 
 */

iosighand()
{

	static          int flags[2];
	int             n;

	flags[0] = netblk.imask[0];
	flags[1] = netblk.imask[1];

	select(64, flags, 0, 0, 0);

	for (n = 0; n < 64; n++)
		if (nisset(flags,n))
			(netblk.protab[n]) (n);
}

isignal(x, proc)
	int             x;
	int             (*proc) ();
{
	if (proc) {

		naddfd(netblk.imask, x);

		fcntl(x, F_SETFL, FASYNC);

#ifndef SIGIO
#define SIGIO SIGPOLL
#endif NSIG

		signal(SIGIO, iosighand);

		netblk.protab[x] = proc;

	} else {
		ndelfd(netblk.imask, x);
		fcntl(x, F_SETFL, 0);
	}
}
