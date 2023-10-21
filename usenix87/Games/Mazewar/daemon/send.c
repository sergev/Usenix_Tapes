#ifndef lint
char rcsid[] = "$Header: send.c,v 1.1 84/08/25 17:04:55 lai Exp $";
#endif

#include <stdio.h>
#include <sys/types.h>


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

extern struct sadr_in sins[MAXPLAYER];		/* sockets of players */

/*
 * send all of the user structs to user at this slot
 */
send_all(slot)
	register int slot;
{
	struct packet p;

	/*
	 * First send the user his own packet, so he knows who he is...
	 */
	p.p_flag = P_USER;		/* this is a user struct */
	p.p_slot = slot;			/* of this player */
#ifdef	NOASGNSTRUCT
	bcopy(&players[slot], &p.p_data.pu_user, sizeof(struct user));
#else
	p.p_data.pu_user = players[slot];
#endif

	if (sendto(ear, (char *)&p, sizeof(struct packet), 0,
		(char *)&sins[slot], sizeof(struct sadr_in)) < 0)
		syserr("send_all(%d): sendto() of new user: ear %d", 
			slot, ear);

	if (players[slot].u_flag == U_BADVERSION)
		return;

	/*
	 * Now send him the entire array of players
	 */
	if (sendto(ear, (char *)players, sizeof(players), 0,
		(char *)&sins[slot], sizeof(struct sadr_in)) < 0)
		syserr("send_all(%d): sendto() of player array: ear %d", 
			slot, ear);
}

/*
 * Send a user's packet to everyone
 */
send_user(slot)
	register int slot;
{
	register int i;
	struct packet p;

	/*
	 * Build up the packet
	 */
	p.p_flag = P_USER;			/* this is a user struct */
	p.p_slot = slot;			/* of this player */
#ifdef	NOASGNSTRUCT
	bcopy(&players[slot], &p.p_data.pu_user, sizeof (struct user));	
#else
	p.p_data.pu_user = players[slot];
#endif

	for (i = 0; i < MAXPLAYER; i++)
		if ((i != slot) && (players[i].u_flag & U_ALIVE))
			if (sendto(ear, (char *)&p, sizeof(struct packet), 0,
				(char *)&sins[i], sizeof(struct sadr_in)) 
				< 0)
				syserr("send_user(%d): to user %d", slot, i);
}

/*
 * Send a user's state to everyone
 */
send_state(slot)
	register int slot;
{
	register int i;
	struct packet p;

	/*
	 * Build up the packet
	 */
	p.p_flag = P_STATE;			/* this is a state struct */
	p.p_slot = slot;			/* of this player */

#ifdef	NOASGNSTRUCT
	bcopy(&players[slot].u_state, &p.p_data.pu_state, sizeof(struct state));
#else
	p.p_data.pu_state = players[slot].u_state;
#endif

	for (i = 0; i < MAXPLAYER; i++)
		if ((i != slot) && (players[i].u_flag & U_ALIVE))
			if (sendto(ear, (char *)&p, sizeof(struct packet), 0,
				(char *)&sins[i], 
				sizeof(struct sadr_in)) < 0)
				syserr("send_user(%d): to user %d", slot, i);
}

/*
 * Send an act packet to all of the users
 */
send_act(a)
	register struct act *a;
{
	struct packet p;

	/*
	 * Build up the packet
	 */
	p.p_flag = P_ACT;			/* this is a act struct */
	p.p_slot = a->a_victem;			/* of this player */

#ifdef	NOASGNSTRUCT
	bcopy(a, &p.p_data.pu_act, sizeof(struct act));
#else
	p.p_data.pu_act = *a;
#endif

	if (sendto(ear, (char *)&p, sizeof(struct packet), 0,
		(char *)&sins[a->a_victem], sizeof(struct sadr_in)) < 0)
		syserr("send_act(%d): ear: %d", a->a_victem, ear);
}
