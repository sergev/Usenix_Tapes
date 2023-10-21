#ifndef lint
static char rcsid[] = "$Header: send.c,v 1.1 84/08/25 17:11:29 chris Exp $";
#endif

#include <stdio.h>
#include <sys/param.h>

#ifdef PLUS5
#include <sys/types.h>
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

extern struct sadr_in earsin;

/*
** Send a status packet.
*/
sendstat(flags)
int flags;
{
	struct packet p;

	if (nodaemon)
	    return;
	p.p_flag = P_STATE;
	p.p_slot = me->u_slot;

	me->u_state.s_flag = flags;
#ifdef	NOASGNSTRUCT
	bcopy(&me->u_state, &p.p_data.pu_state, sizeof(struct state));
#else
	p.p_data.pu_state = me->u_state;
#endif

	if (sendto(ear, (char *) &p, sizeof(p), 0,
		(struct sadr *) &earsin, sizeof(struct sadr_in)) < 0) {
		perror("sendto");
		exit(1);
	}
}

/*
** Send an act packet.
*/
sendact(flag, slot, from)
int flag, slot, from;
{
	struct packet p;

	if (nodaemon)
		return;

	p.p_flag = P_ACT;
	p.p_slot = me->u_slot;

	p.p_data.pu_act.a_flag = flag;
	p.p_data.pu_act.a_victem = slot;
	p.p_data.pu_act.a_killer = from;

	p.p_data.pu_act.a_incarnation = players[slot].u_state.s_incarnation;

	if (sendto(ear, (char *) &p, sizeof(p), 0,
		(struct sadr *) &earsin, sizeof(struct sadr_in)) < 0) {
		perror("sendto");
		exit(1);
	}
}

/*
** Inform the daemon that the current player is quitting
*/
sendquit()
{
	struct packet p;

	if (nodaemon)
		return;

	p.p_flag = P_USER;
	p.p_slot = me->u_slot;

	me->u_flag = U_DEAD;				/* I'm dead */

#ifdef	NOASGNSTRUCT
	bcopy(me, &p.p_data.pu_user, sizeof(struct user));
#else
	p.p_data.pu_user = *me;
#endif

	if (sendto(ear, (char *) &p, sizeof(p), 0,
		(struct sadr *) &earsin, sizeof(struct sadr_in)) < 0) {
		perror("sendto");
		exit(1);
	}
}
