#ifndef lint
static char *RCSid = "$Header: openpty.c,v 1.1 87/04/15 21:29:20 josh Exp $";
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
 * $Source: /usr/pcb/josh/other/network/RCS/openpty.c,v $
 * $Revision: 1.1 $
 * $Date: 87/04/15 21:29:20 $
 * $State: Exp $
 * $Author: josh $
 * $Locker: josh $
 *
 *------------------------------------------------------------------
 *
 * $Log:	openpty.c,v $
 * Revision 1.1  87/04/15  21:29:20  josh
 * Initial revision
 * 
 *
 *------------------------------------------------------------------
 */


#include "netw.h"
#include "openpty.h"
#include <sys/ioctl.h>

_loadtty(cond)
TCOND *cond;
{
        ioctl(0, TIOCGETP, (char *) &cond->sgttyb);
        ioctl(0, TIOCGETC, (char *) &cond->tchars);
        ioctl(0, TIOCGLTC, (char *) &cond->ltchars);
        ioctl(0, TIOCGETD, (char *) &cond->l);
        ioctl(0, TIOCLGET, (char *) &cond->lb);
}

settty(cond)
TCOND cond;
{
        ioctl(0, TIOCSETP, (char *) &cond.sgttyb);
        ioctl(0, TIOCSETC, (char *) &cond.tchars);
        ioctl(0, TIOCSLTC, (char *) &cond.ltchars);
        ioctl(0, TIOCSETD, (char *) &cond.l);
        ioctl(0, TIOCLSET, (char *) &cond.lb);
}

/* 
openpty:  
	executes a function with a pty between the two sides.
	It works very much like popen() except the process
	is on a completly seperate tty.  The returned
	file discripter works exactly like a two way
	socket
*/
	
openpty(cond,task)
TCOND cond;
	int             (*task) ();
{

	struct ptydesc  proc;
	int t;

	if (_openpty(&proc) == -1)
		return (-1);


	if (fork()) {
		close(proc.pt_tfd);
		return (proc.pt_pfd);
	}

	t = open("/dev/tty", 2);

	if (t >= 0) {
		ioctl(t, TIOCNOTTY, 0);
		close(t);
	}
	dup2(proc.pt_tfd, 0);
	dup2(proc.pt_tfd, 1);
	dup2(proc.pt_tfd, 2);

	for (t = 3; t < 64; t++)
		(void) close(t);

	settty(cond);

	(task) (); 
	exit(0);
}
