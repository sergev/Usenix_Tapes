#ifndef lint
static char rcsid[] = "$Header: packet.c,v 1.1 84/08/25 17:11:22 chris Exp $";
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

read_packet(fd)
	int fd;
{
	struct packet p;
	struct sadr_in from;
	int fromlen;

#ifdef DEBUG_2
	if (debug)
	    debugs("Incoming packet\n");
#endif
	/* Read in a new packet */
	if (recvfrom(fd, (char *)&p, sizeof(struct packet), 0,
		(struct sadr *) &from, &fromlen) < 0) 
		(void) syserr("recvfrom");

	switch (p.p_flag) {
		case P_USER:
			userpacket(&p);
			break;
		case P_ACT:
			actpacket(&p);
			break;
		case P_STATE:
			statepacket(&p);
			break;
		default:
			fprintf(stderr,"read_packet(): bad case '%d' in switch",
				p.p_flag);
			break;
	}
}

extern int bitmaze[];

/* This packet type says a player has entered the game or quit the game */
userpacket(p)
	register struct packet *p;
{

#ifdef	NOASGNSTRUCT
	bcopy(&p->p_data.pu_user, &players[p->p_slot], sizeof(struct user));
#else
	players[p->p_slot] = p->p_data.pu_user;
#endif
#ifdef DEBUG_2
	if (!debug)
#endif
	    draw_status(p->p_slot);

	if (p->p_data.pu_user.u_flag & U_DEAD)
	    /* Delete a quitter */
	    delplayer(p->p_slot, players[p->p_slot].u_x, players[p->p_slot].u_y);
	else
	    /* Add a wonderful new player */
	    addplayer(p->p_slot, players[p->p_slot].u_x, players[p->p_slot].u_y);

#ifdef DEBUG_2
	if (debug && (p->p_data.pu_user.u_flag & U_DEAD)) {
		char buf[BUFSIZ];

		(void) sprintf(buf, "Hey!  #%d (%s) died/quit!\n",
			p->p_slot, p->p_data.pu_user.u_name);
		debugs(buf);
	}
#endif
}

/* This packet tells of shooting or a keep alive from daemon */
actpacket(p)
	register struct packet *p;
{
	switch (p->p_data.pu_act.a_flag) {
		case A_SHOT:
			/*
			 * Avoid being hit by unflushed packets
			 * aimed at our previous incarnation
			 */
			if (p->p_data.pu_act.a_incarnation !=
					me->u_state.s_incarnation)
				break;
			me->u_hp--;
			/* Handle deaders. */
			if (me->u_hp < 1) {
				/*
				 *  Send killer his score
				 */
				sendact(A_KILL, 
					p->p_data.pu_act.a_killer, 
					me->u_slot);
				/* Reinitialize player */
				me->u_kills -= 5;
				me->u_hp = HITPOINTS;

				/*
				 *  Draw an explosion and sleep for a bit
				 *  to show the player he's been killed.
				 *
				 *  This will do something for both ascii
				 *  and SUN mode, but is a little slow in
				 *  the ascii mode.
				 */
				draw_my_death();

				/*
				 *  Set me up in a new position.
				 */
				initposition(me);
				takill();
				me->u_state.s_incarnation++;
				sendstat(MOVE|CHANGE);
			}
			else
			    /* Otherwise just inform others of my hp's */
			    sendstat(CHANGE);
#ifdef DEBUG_2
			if (!debug)
#endif
			    draw_status(me->u_slot);
			break;
		case A_KILL:
			/*
			** Hey!  I just got a kill!
			*/
			me->u_kills += 10;
			/* me->u_hp += 5; */
			sendstat(CHANGE);
#ifdef DEBUG_2
			if (!debug)
#endif
			    draw_status(me->u_slot);
			break;
		case A_QUERY:
			/*
			 * The daemon is asking us if we are alive.
			 * we want to send an act packet to the daemon
			 * to signify that we are, indeed, alive.
			 */
			sendact(A_QUERY, me->u_slot, me->u_slot);
			break;
		case A_DAEMON:
			/*
			** The daemon has informed us that it has died.
			*/
			quit(0);
	}
}

statepacket(p)
	register struct packet *p;
{
	
	/*
	** If this packet shows that the player has moved, we must remove
	** him from his old position in the bitmaze and below we will put
	** him back in.
	*/

	/*
	** The bitmaze is kept for fast lookups for the nearest person.
	*/
	if (p->p_data.pu_state.s_flag & MOVE)
	    delplayer(p->p_slot, players[p->p_slot].u_x, players[p->p_slot].u_y);
#ifdef	NOASGNSTRUCT
	bcopy(&p->p_data.pu_state, &players[p->p_slot].u_state, 
		sizeof(struct state));
#else
	players[p->p_slot].u_state = p->p_data.pu_state;
#endif
	/*
	** Put the player into his new position.
	*/
	if (p->p_data.pu_state.s_flag & MOVE)
	    addplayer(p->p_slot, players[p->p_slot].u_x, players[p->p_slot].u_y);
	/*
	** If the player's status line info has changed, force an update of
	** his status line entry
	*/
	if (players[p->p_slot].u_state.s_flag & CHANGE)
#ifdef DEBUG_2
	    if (!debug)
#endif
		draw_status(p->p_slot);
}
