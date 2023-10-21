#ifndef lint
static char rcsid[] = "$Header: ualarm.c,v 1.1 84/08/25 17:11:32 chris Exp $";
#endif

#include <stdio.h>
#ifdef PLUS5
#include "../berk/time.h"
#endif

#include <sys/time.h>
#include "../h/defs.h"
#include "../h/struct.h"
#include "../h/extern.h"
#include <signal.h>

/*
 * Generate a SIGALRM signal in ``usecs'' microseconds
 * If ``reload'' is non-zero, keep generating SIGALRM
 * every ``reload'' microseconds.
 */
static int	(*ugoto)() = NULL;
static int	bang = 0;
static int	going = 0;

unsigned ualarm(reload,where)
register unsigned reload;
register int (*where)();
{
	int ualarm2();
	ugoto = where;
	bang = reload;
	if(going!=0) return;
	signal(SIGALRM,ualarm2);
	alarm(bang);
	++going;
}

ualarm2(i)
int i;
{
	int ualarm2();
	if(ugoto!=NULL)
		(*ugoto)();
	signal(SIGALRM,ualarm2);
	alarm(bang);
}

/* ualarm(usecs, reload)
	register unsigned long usecs;
	register unsigned long reload;
{
	struct itimerval new, old;

	timerclear(&old.it_interval);
	timerclear(&old.it_value);

	new.it_interval.tv_usec = reload % 1000000;
	new.it_interval.tv_sec = reload / 1000000;
	
	new.it_value.tv_usec = usecs % KEYOK;
	new.it_value.tv_sec = usecs / KEYOK;

	if (setitimer(ITIMER_REAL, &new, &old) == 0)
		return;
	return;
}
 */
