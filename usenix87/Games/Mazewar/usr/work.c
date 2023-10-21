#ifndef lint
static char rcsid[] = "$Header: work.c,v 1.1 84/08/25 17:11:37 chris Exp $";
#endif

#ifdef PLUS5
#include <signal.h>
#endif

#include <sys/param.h>
#include <stdio.h>
#include <sys/time.h>
#include <errno.h>
#include "../h/defs.h"
#include "../h/struct.h"
#include "../h/extern.h"

#define bit(i)			(1 << (i))
#define BLOCK			((struct timeval *)0)
#define MASK(sig)		(1 << ((sig) - 1))
#define SIGHOLD(signo) 		(sigblock(MASK(signo)))
#define SIGRELSE(signo) 	(sigsetmask(sigblock(0) &~ MASK(signo)))

/*
** Work waits in select for input from either the daemon (ear) or the
** keyboard and deal with it.
*/
work()
{
	register int savemask = 0;
	register int nready;
	int readmask;
	extern int errno;

	savemask |= bit(fileno(stdin));
	if (!nodaemon)
	    savemask |= bit(ear);

	for (;;) {
		readmask = savemask;

		nready = select(NOFILE, &readmask, (int *)0, (int *)0, BLOCK);
		if (nready < 0) {
			if (errno != EINTR)
				perror("select");
		}
		else if (nready > 0) {
			(void) SIGHOLD(SIGALRM);
			/* Keyboard */
			if (readmask & bit(fileno(stdin)))
				service();
			/* daemon */
			if ((!nodaemon) && (readmask & bit(ear)))
				read_packet(ear);
			(void) SIGRELSE(SIGALRM);
			srefresh();
		}
	}
}
