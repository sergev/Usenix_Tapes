#ifndef lint
static char rcsid[] = "$Header: signal.c,v 2.1 85/04/10 17:32:00 matt Stab $";
#endif
/*
 *
 * search
 *
 * multi-player and multi-system search and destroy.
 *
 * Original code by Dave Pare	1984
 * Ported & improved
 *      by Matt Crawford	1985
 *
 * signal handlers for the daemon
 *
 * Copyright (c) 1984
 *
 * $Log:	signal.c,v $
 * Revision 2.1  85/04/10  17:32:00  matt
 * Major de-linting and minor restructuring.
 * 
 */

#include <stdio.h>
#include <sys/types.h>
#include <time.h>
#include <sys/file.h>
#include <signal.h>
#include <nlist.h>
#include "defines.h"
#include "structs.h"


trap(sig)
int sig;
{
	extern	void	done();
	extern	t_player player[NPLAYER];
	extern	int pfd;
	extern	int errfile;
	t_player *p;
	char	buf[80];


	for (p = player; p < &player[NPLAYER]; p++)
		if (p->status.alive == TRUE)
			done(p);
	close(pfd);
	sprintf(buf, "trapped interrupt %d\n", sig);
	errlog(buf);
	close(errfile);
	exit(sig);
	/*NOTREACHED*/
}

core_dump()
{
	extern	void	done();
	extern	t_player	player[NPLAYER];
	t_player	*p;

	errlog("dumping core...\n");
	signal(SIGQUIT, SIG_IGN);
	for (p = player; p < &player[NPLAYER]; p++)
		if (p->status.alive == TRUE)
			done(p);
	signal(SIGQUIT, SIG_DFL);
	kill(getpid(), SIGQUIT);
	/*NOTREACHED*/
}


/*
 *  Put the delay in the game.  tune to taste.
 *  There is not much one can do here, because SVR2 doesn't make
 *
 */
void
set_delay() {
	static long lasttime = 0;

	if (time (0L) == lasttime)
		sleep (1);
	lasttime = time (0L);
/*
	extern	int nplayer;
	long usecs;
	usecs = 100000L - (nplayer * 15000L);
	if (usecs < 10000L)
		return;
*/
}
