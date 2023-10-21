#ifndef lint
static char rcsid[] = "$Header: cmds1.c,v 2.3 85/08/06 22:28:20 matt Exp $";
#endif
/*
 *
 * search
 *
 * multi-player and multi-system search and destroy.
 *
 * Original by Greg Ordy	1979
 * Rewrite by Sam Leffler	1981
 * Socket code by Dave Pare	1983
 * Ported & improved
 *      by Matt Crawford	1985
 *
 * routines to execute player commands
 *
 * Copyright (c) 1979
 *
 * $Log:	cmds1.c,v $
 * Revision 2.3  85/08/06  22:28:20  matt
 * Change handling of "r", "b", "g", "j", "q" commands to
 * provide better feedback, using per-player message buffer.
 * 
 * Revision 2.2  85/05/30  22:43:05  matt
 * You can't send a "Temporary malfunction" message to an alien!!
 * 
 * Revision 2.1  85/04/10  17:30:46  matt
 * Major de-linting and minor restructuring.
 * 
 */

#include "defines.h"
#include "structs.h"

int     TDIST = TSIZE;		/* max distance a torpedoe travels */

/*
 * Launch a torpedo.
 * "firer" shot it, using the "where" key, firing in the <ox, oy> direction.
 * "dd" indicates distance until it dies.
 * "mess" indicates if a launch message should be given.
 * "xl" and "yl" give the origin for calculating distance.
 * "type" identifies the type of the firer.
 */
void launch(firer, where, type, ox, oy, mess, xl, yl, dd)
register thing *firer;
char where, xl, yl, ox, oy;
{
	extern	void pstatus();
	extern	t_torps torps[NTORP];
	register char x,
		y;
	register t_torps *tp,
		*last;

	last = &torps[NTORP];
	x = ox; y = oy;
	switch (where) {
	    case NO: y--; break;
	    case NE: x++; y--; break;
	    case EA: x++; break;
	    case SE: x++; y++; break;
	    case SO: y++; break;
	    case SW: x--; y++; break;
	    case WE: x--; break;
	    case NW: x--; y--; break;
	    default:
		if (x == ox && y == oy) {
		    pstatus(&firer->u_player, "Illegal direction\n");
		    return;
		}
	}
	for (tp = torps; tp < last; tp++) {
	    if (tp->owner != NOTHING) continue;
	    if (type==PLAYER && firer->u_player.energy<TPCOST) {
		    if (mess) {
			    pstatus(&firer->u_player, "Need more energy!");
			    firer->u_player.cmdpend = 0;
		    }
		    return;
	    }
	    tp->owner = firer; tp->torx = xl; tp->tory = yl;
	    tp->tcount = dd != 0 ? dd : TDIST;
	    tp->xinc = x; tp->yinc = y;
	    if (type != PLAYER) return;
	    firer->u_player.energy -= TPCOST;
	    firer->u_player.cmdpend = 0;
	    if (mess) pstatus(&firer->u_player, "Launch!");
	    return;
	}
	if (type != PLAYER) return;
	pstatus(&firer->u_player, "Temporary malfunction!");
	firer->u_player.cmdpend = 0;
}

/*
 * Show coordinates of an object locked in on channel "channel".
 */
void home(p, channel)
register t_player *p;
char channel;
{
	extern	void pstatus();
	void	showhome();

	p->cmdpend = 0;
	if (channel < '1' || channel > '3') {
		pstatus(p, "Invalid channel number!");
		return;
	}
	channel -= '1';
	if (p->home[channel] == NOTHING) {
		pstatus(p, "Channel not locked on object!");
		return;
	}
	showhome(p, channel);
}

/*
 * Show the coordinates of a homing radar channel on the screen --
 *  assumes the channel is valid.
 */
static void showhome(p, channel)
register t_player *p;
register int channel;
{
	extern	void put();
	extern	t_sbase sbase[NBASE];
	extern	t_alien alien[NALIEN];
	extern	t_player player[NPLAYER];
	extern	t_torps torps[NTORP];
	thing	*val;		/* should be union */

	switch (channel) {

	case 0: 
		move(H1DATAX, H1DATAY, p);	/* macro */
		break;
	case 1: 
		move(H2DATAX, H2DATAY, p);
		break;
	case 2: 
		move(H3DATAX, H3DATAY, p);
		break;
	}
	val = p->home[channel];
	if (isalien(val))
		put(p, "%d  %d", val->u_alien.cx, -val->u_alien.cy);
	else if (isplayer(val))
		put(p, "%d  %d", val->u_player.curx, -val->u_player.cury);
	else if (isbase(val))
		put(p, "%d  %d", val->u_sbase.xpos, -val->u_sbase.ypos);
	else
		put(p, "%d  %d", val->u_torps.torx, -val->u_torps.tory);
}

/*
 * Set channel "channel" so that it's locked in on the object
 *  currently "underneath" player "p".
 */
void sethome(p, channel)
register t_player *p;
char channel;
{
	extern	void pstatus();
	void	showhome();

	p->cmdpend = 0;
	if (channel < '1' || channel > '3') {
		pstatus(p, "Invalid channel number!");
		return;
	}
	channel -= '1';
	if (p->whocent == NOTHING) {
		pstatus(p, "Not over an object!");
		return;
	}
	p->home[channel] = p->whocent;
	showhome(p, channel);
	pstatus(p, "Locked on %c", gettype(p->whocent));
}

/*
 * Get a character representing the thing 't'.
 */
gettype(t)
union thing_u *t;
{
    if (isbase(t)) return '*';
    else if (isplayer(t)) return 'A'+&t->u_player-player;
    else if (isalien(t)) return t->u_alien.aname;
    else return ' ';
}


/*
 * Invisibility toggle -- useless during solar flares.
 */
void invisible (p)
register t_player *p;
{
	extern	int sfflag,
		gutsflag;
	extern	void pmesg(),
		put();

	move(INDATAX, INDATAY, p);	/* macro */
	if (p->status.invis == FALSE) {
		if (sfflag == ON) {
			put(p, "off");
			pmesg(p, "From radar-- Solar flare up!!");
			return;
		} else if (gutsflag) {
			put(p, "off");
			pmesg(p, "No invisibility in GUTS mode");
			return;
		}
		put(p, "on ");
		p->status.invis = TRUE;
	} else {
		put(p, "off");
		p->status.invis = FALSE;
	}
}

/*
 * Dock at a starbase -- causes refueling to take place.
 */
void dock(p)
register t_player *p;
{
	extern	void pstatus(),
		cput();

	if (!isbase(p->whocent)) {
		pstatus(p, "Not over a starbase!");
		return;
	}
	pstatus(p, "Successful docking!");
	if (p->maxe < p->maxmaxe) p->maxe = p->maxe+25;
	if (p->maxe > p->maxmaxe) p->maxe = p->maxmaxe;
	p->energy = p->maxe;
	cput(EGYDATA, p, "%d", p->energy);
}

/*
 * Autopilot on a channel which has previously been locked onto
 *  an object -- normally this is a starbase, but it could be a
 *  player, alien, or even a planet.
 */
void autopi(p, channel)
register t_player *p;
char channel;
{
	extern	t_planet planet;
	extern	void pstatus();
	thing	*val;
	register char *x,
		*y;

	p->cmdpend = 0;
	if (channel < '1' || channel > '3') {
		pstatus(p, "Invalid channel number!");
		return;
	}
	channel -= '1';
	val = p->home[channel];
	if (val == NOTHING) {
		pstatus(p, "Channel not locked on object!");
		return;
	}
	if (isalien(val)) {
		x = & val->u_alien.cx;
		y = & val->u_alien.cy;
	} else if (isplayer(val)) {
		x = & val->u_player.curx;
		y = & val->u_player.cury;
	} else if (isbase(val)) {
		x = & val->u_sbase.xpos;
		y = & val->u_sbase.ypos;
	} else {			/* planet !?!?!? */
		x = & planet.places[0][0];
		y = & planet.places[0][1];
	}

	p->apx = x;
	p->apy = y;
	p->status.ap = TRUE;
	p->status.orb = FALSE;
	pstatus(p, "Autopilot process begun to %c", gettype(val));
}

/*
 * Broadcast a message.
 */
void bcast(p, letter)
register t_player *p;
char letter;
{
	extern	void pmesg(),
		echomsg(),
		donemsg(),
		makescab();
	register t_player *pl;

	if (letter != '\n' && letter != '\r') {
		if (letter == SCABLETTER) {	/* no tty tricks! */
			makescab(p);
			letter = '^';
		}
		echomsg(p, letter);
	} else {

		for (pl = player; pl < &player[NPLAYER]; pl++)
			if (pl->status.alive == TRUE)
				pmesg(pl, "\07From %s -- %s", p->plname, p->mesgbuf);
		p->cmdpend = 0;
		donemsg(p);
	}
}

/*
 * broadcast and radio message echoing routines
 */

void
gathermsg(p)
register t_player *p;
{
	if (p->mesgdst >= NPLAYER)
		pmesg(p, "From radio room-- enter message to group");
	else if (p->mesgdst > 0)
		pmesg(p, "From radio room-- enter message to %c (%s)",
			p->mesgdst-1+'A', player[p->mesgdst-1].plname);
	else
		pmesg(p, "From radio room-- enter message");
	prompt(p, "TEXT: ");
	p->mesglen = 0;
	p->mesgbuf[0] = '\0';
	bflush(p);
#ifdef DEBUG
	errlog ("GATHERMSG: after bflush.\n");
#endif DEBUG
}

/*
 * echoing is relative to the prompt string given above
 */
#define INPUTSTART PROMPTX+6
#define	INPUTY	   PROMPTY

void
echomsg(p, c)
register t_player *p;
char c;
{
	if (c=='\b' || c=='\177') {
		if (p->mesglen) {
			cput(p->mesglen+INPUTSTART, INPUTY, p, " \b");
			p->mesglen--;
		}
		p->mesgbuf[p->mesglen] = '\0';
	} else if (c=='@' || c=='\025') {
		gathermsg(p);
		return;
	} else {
		/* unprintable cacters show up as blanks */
		if (c < ' ')
			c = ' ';

		if (p->mesglen < 35) {
			p->mesgbuf[p->mesglen++] = c;
			p->mesgbuf[p->mesglen] = '\0';
			cput(p->mesglen+INPUTSTART, INPUTY, p, "%c", c);
		}
	}

	bflush(p);
}

void
donemsg(p)
register t_player *p;
{
	prompt(p, "");
	p->mesgdst = 0;
	p->mesglen = 0;
	p->mesgbuf[0] = '\0';
	bflush(p);
#ifdef DEBUG
	errlog ("DONEMSG: after bflush.\n");
#endif DEBUG
}
