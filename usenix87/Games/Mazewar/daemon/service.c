#ifndef lint
static char rcsid[] = "$Header: service.c,v 1.1 84/08/25 17:04:56 lai Exp $";
#endif

#include "../h/defs.h"
#include "../h/struct.h"
#include "../h/extern.h"

act(p)
	register struct packet *p;
{
	long time();

	/*
	 * someone is responding to our query about whether they are
	 * still there or not - obviously they are, so save the current
	 * time in the lastup array...
	 */
	if (p->p_data.pu_act.a_flag & A_QUERY) {
		lastup[p->p_slot] = time();
		return;
	}

	send_act(&p->p_data.pu_act);
}

state(p)
	register struct packet *p;
{
	/*
	 * stick the state struct into the players user struct
	 */
#ifdef	NOASGNSTRUCT
	bcopy(&p->p_data.pu_state, &players[p->p_slot].u_state, 
		sizeof(struct state));
#else
	players[p->p_slot].u_state = p->p_data.pu_state;
#endif


	/*
	 * send this data to all of the players
	 */
	send_state(p->p_slot);
}
