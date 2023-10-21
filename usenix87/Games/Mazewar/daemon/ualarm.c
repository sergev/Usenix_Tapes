#ifndef lint
static char rcsid[] = "$Header: ualarm.c,v 1.1 84/08/25 17:04:58 lai Exp $";
#endif

#ifdef PLUS5
#include "../berk/btime.h"
#endif

#include <sys/time.h>
#include <stdio.h>
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

ghostbuster()
{
	register int i;
	long now, time();
	struct act a;
	extern char *curtime();

	now = time((char *)0);

	for (i = 0; i < MAXPLAYER; i++) {
		if ((players[i].u_flag & U_ALIVE) == 0)
			continue;
		if (now > lastup[i] + GHOST_TIMEOUT) {
			players[i].u_flag = 0;		/* you lose */
			syslog("ghostbuster(): zapped %s@%s (%d): %ld sc: %s\n",
					players[i].u_name, 
					players[i].u_hostname,
					i,
					now - lastup[i],
					curtime());
			send_user(i);
		} else {
			a.a_flag = A_QUERY;
			a.a_victem = i;
			send_act(&a);
		}
	}
}
