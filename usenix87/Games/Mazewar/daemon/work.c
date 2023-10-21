#ifndef lint
static char rcsid[] = "$Header: work.c,v 1.1 84/08/25 17:05:00 lai Exp $";
#endif

#include <stdio.h>
#include <sys/signal.h>
#include <sys/types.h>
#include <errno.h>

#ifdef PLUS5
#include "../berk/btypes.h"
#include "../berk/uio.h"
#include "../berk/socket.h"
#include "../berk/in.h"
#else
#include <sys/uio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#endif

#include "../h/defs.h"
#include "../h/struct.h"
#include "../h/extern.h"

#define bit(i)	(1 << (i))
#define MASK(sig)		(1 << ((sig) - 1))
#define SIGHOLD(signo)		(sigblock(MASK(signo)))
#define SIGRELSE(signo)		(sigsetmask(sigblock(0) &~ MASK(signo)))

int ear;
extern int errno;

work()
{
	struct packet p;
	struct sadr_in from;
	int fromlen;

	for (;;) {
		fromlen = sizeof(from);
		if (recvfrom(ear, &p, sizeof(struct packet), 0,
			&from, &fromlen) < 0) {
			if (errno != EINTR)
				syserr("recvfrom");
			continue;
		}

		SIGHOLD(SIGALRM);
		switch (p.p_flag) {
			case P_USER:
				join(&p, &from, fromlen);
				break;
			case P_ACT:
				act(&p);
				break;
			case P_STATE:
				state(&p);
				break;
			default:
				syslog("work(): bad case '%d' in switch",
					p.p_flag);
				break;
		}
		SIGRELSE(SIGALRM);
	}
}

extern char *program;

nice_exit()
{
	register int i;
	static struct act a;

	for (i = 0; i < MAXPLAYER; i++)
		if (players[i].u_flag & U_ALIVE) {
			a.a_flag = A_DAEMON;
			a.a_victem = i;
			a.a_killer = i;
			send_act(&a);
		}

	syslog("%s: nice exit at %s\n", program, curtime());
	exit(0);
}
