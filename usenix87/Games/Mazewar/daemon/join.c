#ifndef lint
static char rcsid[] = "$Header: join.c,v 1.1 84/08/25 17:04:53 lai Exp $";
#endif

#include <sys/types.h>
#include <stdio.h>

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

extern struct sadr_in sins[MAXPLAYER];

join(p, from, fromlen)
	register struct packet *p;
	struct sadr_in *from;
	int fromlen;
{
	register int i;
	long time();
	char *curtime();

#ifdef DEBUG
	if (debug)
		verify(from);
#endif

	if (p->p_data.pu_user.u_flag & U_DEAD) {		/* died */
#ifdef	NOASGNSTRUCT
		bcopy(&p->p_data.pu_user, &players[p->p_slot],
			sizeof(struct user));
#else
		players[p->p_slot] = p->p_data.pu_user;
#endif

		send_user(p->p_slot);

#ifdef DEBUG
		if (debug)
			syslog("player %d (%s) quit/died\n",
				players[p->p_slot].u_slot,
				players[p->p_slot].u_name);
#endif

		return (0);
	}
		
	for (i = 0; players[i].u_flag & U_ALIVE; i++)
		if (i >= MAXPLAYER)
			return (-1);

	/*
	 * Stick the new user in the right place in the array of players
	 */
#ifdef	NOASGNSTRUCT
	bcopy(&p->p_data.pu_user, &players[i], sizeof(struct user));
#else
	players[i] = p->p_data.pu_user;
#endif
	players[i].u_slot = i;

#ifdef DEBUG
	if (debug)
		syslog("New player #%d, %s (%s)\n", 
			i, players[i].u_name, p->p_data.pu_user.u_name);
#endif

	/*
	 * Put his calling address into the array of socket addresses
	 */
	bcopy(from, &sins[i], fromlen);

	if (!ok_version(p)) {
		players[i].u_flag = U_BADVERSION;
		send_all(i);
		syslog("Bad version (%d:%d) from %s@%s at %s\n",
			players[i].u_version, players[i].u_magic,
			players[i].u_name, players[i].u_hostname, curtime());
		return(-1);
	}

	send_all(i);

	send_user(i);

	lastup[i] = time((char *)0);

	return(0);
}

ok_version(p)
	register struct packet *p;
{
	extern int version;

	if (p->p_data.pu_user.u_version < version)
		return(0);

	if (p->p_data.pu_user.u_magic != V_MAGIC)
		return(0);

	return(1);
}
