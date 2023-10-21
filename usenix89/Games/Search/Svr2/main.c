#ifndef lint
static char rcsid[] = "$Header: main.c,v 2.2 85/08/06 22:26:37 matt Exp $";
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
 * main game iteration loop
 *
 * Copyright (c) 1979
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <sys/file.h>
#include <time.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include "defines.h"
#include "structs.h"

extern int sock;
static struct message message;

/*
 * Major data bases
 */
t_torps		torps[NTORP];		/* torpedoes data base */
t_sbase		sbase[NBASE];		/* star bases data base */
t_planet	planet;			/* planet data base */
t_alien		alien[NALIEN];		/* aliens data base */
t_burst		bursts[NBURST];		/* shrapnel data base */
t_player	player[NPLAYER];	/* players */

int		dtabsiz;		/* descriptor table size */
int		pfd;			/* playerfile fd */
int		errfile;		/* error log file */
int		nplayer;		/* # of players currently in game */
int		nobody;			/* is anybody playing? */

t_player	*whoscab;		/* identity of current scab */
int		TDIST;			/* max distance a torpedo can go */
char		xxi;
char		yyi;
char		ppx;			/* planet x-coord */
char		ppy;			/* planet y-coord */
char		dx;
char		dy;
long		ptime;			/* search playing time counter */

int		NALIENS = NALIEN;
extern		int errno;
/*
 * Solar flare stuff
 */
int		sfcount;		/* solar flare timer */
int		sfflag;			/* solar flare in progress */

int		gutsflag;		/* playing in guts mode */

int		numbnode;
struct plist	*avl;
t_player	*wholist;
char		visual[NPLAYER][NPLAYER];/* group visibility matrix */
int		tim_inp;


main(argc, argv)
int argc;
char  *argv[];
{
    int cost;
    extern	t_player *newplayer();
    extern	void	apilot(), autopi(), bcast(), bflush(), cput(),
		    dock(), done(), enteritem(), facts(), fnode(),
		    gathermsg(), groupm(), home(), init(), initplayer(),
		    invisible(), joing(), launch(), lookg(), magnif(),
		    makeburst(), makeflare(), makescab(), movealien(),
		    moveburst(), nuke(), orbit(), orbmove(), outgame(),
		    phaser(), pmesg(), prompt(), pstatus(), quitg(),
		    rmsg(), seeall(), seescab(), sethome(), set_delay(),
		    uplst(), xploit();
		    
    register t_player *p;
    register t_player *pl;
    register t_alien *pa;
    register t_burst *pb;
    register t_sbase *ps;
    register t_torps *pt;
    t_player *np;
    char	x, y;
    int	mf;
    int	gutsvote;
    char	buf[255];
    char	xx, yy, c;
    int	port_num = DEFAULT_IN_PORT;
    int	nfds;
    int	mask;
    int	save_mask;

    /*
     * set up data bases
     */
    setuid(geteuid());
    init();
    if (fork())
	    exit(0);
    /*
     * set up sockets: port number doesn't matter
     * if INET isn't defined
     */
    if (!initsocks())
	    exit(1);
    /*
     * block indefinitely until the first player enters the game
     * then start the player loop
     */
    nobody = 1;
    /*
     * Main loop -- works in a hard run polling the input socket
     * of each player.  Each turn around the loop defines a time
     * unit in search.
     */
    for (;;) {
	ptime++;		/* time marches on ... */
	if (--sfcount == 0) {	/* ring a ding, time for a flare */
		makeflare();
		NALIENS = NALIEN-nplayer;
	}
	/*
	 * if nobody's playing, wait until someone generates
	 * some sort of request
	 */
	if ((ptime % 16) == 0) {
		np = newplayer(nobody);
		if (np != NULL) {
			nplayer++;
			initplayer(np);
			errlog("new player %s uid %d\n",
			    np->plname, np->uid);
			for (p = player; p < &player[NPLAYER]; p++)
				if (p->status.alive == TRUE && p != np)
					pmesg(p, "From scanners-- NEW PLAYER");
			if (gutsflag)
				pmesg(np, "This game is in GUTS mode");
		}
	} else
	    set_delay();
	/*
	 * Command scanning loop...
	 */
	nobody = 0;
	gutsvote = !gutsflag;
	for (p = player; p < &player[NPLAYER]; p++) {
		if (p->status.alive == FALSE) {
			nobody++;
			continue;
		}
		gutsvote = gutsflag ? gutsvote || p->status.guts
				    : gutsvote && p->status.guts;
		if (p->status.ap == TRUE)	/* update autopilot */
			apilot(p);
		if (p->status.orb == TRUE)	/* update orbiting */
			orbmove(p);
		x = p->offx;
		y = p->offy;
		if (!p->ninput)
			if (pinput(p) < 0)
				continue;
		if (p->ninput <= 0)
			goto constvel;
		c = (*p->pinputq++)&0177;
		p->ninput--;
		/*
		 * Two letter commands (e.g. launch torpedo) are
		 *  marked as a command pending, so that when the
		 *  second char comes in it can be processed entirely.
		 */
		if (p->cmdpend > 0) {
			switch (p->cmd) {
			case 'h': 
				home(p, c);
				break;
			case '5':
			case 't': 
				p->status.ap = FALSE;
				launch((thing *)p, c, PLAYER, x, y,
					1, p->curx, p->cury, 0);
				break;
			case 'g':
				groupm(p, c);
				break;
			case 'q':
				quitg(p, c);
				break;
			case 'n':
				nuke(p, c);
				break;
			case 'j':
				joing(p, c);
				break;
			case 's': 
				sethome(p, c);
				break;
			case 'b': 
				bcast(p, c);
				break;
			case 'a': 
				autopi(p, c);
				break;
			case 'r': 
				rmsg(p, c);
				break;
			case 'm': 
				magnif(p, c);
				break;
			case 'Q':
			    if (c == 'P' && geteuid() == OWNERID &&
				fork() == 0) {
				execl("/usr/src/local/search/search",
				    "search", 0);
			    }
			    if (c == 'P')
				pmesg(p, "Starting player search\n");
			    if (c != 'Y' && c != 'y') {
				    p->cmd = p->cmdpend = 0;
				    prompt(p, "");
				    goto constvel;
			    }
			    done(p);
			    break;
			}
			goto constvel;
		}
		switch (c) {
		case 'e': 
			if (p->energy != p->preve) {
				cput(EGYDATA, p, "%d", p->energy);
				p->preve = p->energy;
			}
			break;
		case 'p': 
			if (p->points != p->prevp) {
				cput(PTDATA, p, "%d", p->points);
				p->prevp = p->points;
			}
			break;
		case 'f': 
			facts(p);
			break;
		case 'l':
			lookg(p);
			break;
		case 'x': 
			xploit(p);
			break;
		case '0': 
			if (p->whocent == 0)
				break;
			if (isalien(p->whocent)) {
				pa = (t_alien *)p->whocent;
				/*
				 * Register a hit on an alien
				 */
				pa->hit++; p->maxmaxe++;
				if (pa->hit > 5) {
				    pa->stats = DEAD;
				    p->maxmaxe++;
				}
				pa->cix = -1;
				pa->ciy = (rand()%3)-1;
				p->points += 4;
				message.mtype = p->uid+1;
				message.text[0] = '\007';
				message.text[1] = 0;
				msgsnd(sock, &message, 2+8, 0);
			}
			break;
		case 'i': 
			invisible(p);
			break;
		case 'c': 
			cput(TMDATA, p, "%d", (int)ptime);
			break;
		case 'o': 
			orbit(p);
			x = p->offx;
			y = p->offy;
			break;
		case 'z': 
			seescab(p);
			break;
		case 'd': 
			dock(p);
			break;
		case 'v': 
			cput(VLDATA, p, "%d %d", p->offx, -p->offy);
			break;
		case 'G':
			p->status.guts = TRUE;
			break;
		case 'W':
			p->status.guts = FALSE;
			break;
		case '\014':	/* ^L */
		case '@': 
			fnode(p->plstp);
			initplayer(p);
			break;
		case 'Q':
			prompt(p, "Really quit?");
			goto savecmd;
		case 'q':
		case 'j':
			prompt(p, "enter group members");
			goto savecmd;
		case 'b':
		case 'g':
			gathermsg(p);
			/*FALL THRU*/
		case 'r':
		case 'h':
		case 't':
		case 'a':
		case 'm':
		case 's':
		case 'n':
		case '5':
		savecmd:
			p->cmdpend++;
			p->cmd = c;
			break;
		default: 
			goto vel;
		}
		goto constvel;
	vel: 
		/*
		 * Velocity changes handled here...
		 */
		p->status.ap = p->status.orb = FALSE;
		switch (c) {
		case NO: y--; break;
		case NE: x++; y--; break;
		case EA: x++; break;
		case SE: x++; y++; break;
		case SO: y++; break;
		case SW: x--; y++; break;
		case WE: x--; break;
		case NW: x--; y--; break;
		default: goto constvel;
		}
		cost = max(energycost(p->offx,p->offy),
			   energycost(x,y));
		if (cost > p->energy) {
		    pmesg(p, "Out of energy\n");
		    x = p->offx; y = p->offy;
		} else {
		    p->offx = x;
		    p->offy = y;
		    p->energy -= cost + 1; /* consvel will add one */
		    cput(VLDATA, p, "%d %d", p->offx, -p->offy);
		}
constvel:
		/*
		 * Update energy total for player:
		 *  Non accelerating adds one unit/iteration
		 *  magnification costs energy proportional to M-factor
		 *  invisibility costs INCOST/iteration
		 */
		p->curx = p->curx+x;
		p->cury = p->cury+y;
		p->energy++;
		if (p->mflg > 1) p->energy -= p->mflg;
		if (p->status.invis == TRUE)
			p->energy = p->energy-INCOST;
		if (p->maxe > p->maxmaxe) p->maxe = p->maxmaxe;
		if (p->energy > p->maxe) p->energy = p->maxe;
		if (p->energy < 0) outgame(p);
		if (p->status.crash == TRUE) {
			pstatus(p, "Crash into planet--all is lost!!");
			if (p->points >= 50)
				p->points -= 50;
			p->status.killed = TRUE;
			done(p);
		}
		/*
		 * Update energy display only every 16 iterations --
		 *  otherwise, it would be constantly updated at
		 *  great expense
		 */
		if (p->energy != p->preve && (ptime&017) == 0) {
			cput(EGYDATA, p, "%d", p->energy);
			p->preve = p->energy;
		}
		/*
		 * Friction for high speeds.
		 */
		if ((ptime&017) == 0) {
			p->offx = friction(p->offx);
			p->offy = friction(p->offy);
			cput(VLDATA, p, "%d %d", p->offx, -p->offy);
		}
		if (p->points != p->prevp) {
			cput(PTDATA, p, "%d", p->points);
			p->prevp = p->points;
		}
	}
	if (nobody < NPLAYER)
		nobody = 0;
	if (gutsvote && !gutsflag)
		seeall("\07Entering GUTS mode-- beware!");
	else if (gutsflag && !gutsvote)
		for (p = player; p < &player[NPLAYER]; p++)
			if (p->status.alive == TRUE)
				pmesg(p, "Entering WIMP mode");
	gutsflag = gutsvote;
	moveburst();
	/*
	 * Update the state of the aliens...
	 */
	for (pa = alien; pa < &alien[NALIEN]; pa++) {
		if (pa->stats == DEAD)
			continue;
		if (pa->type == WANDER) {
			pa->count--;
			/* time to change direction */
			if (pa->count == 0) {
				pa->cix = (rand()%3)-1;
				pa->ciy = (rand()%3)-1;
				pa->count = (rand()%50)+30;
			}
		}
		/* shankers "dance" */
		if (pa->type == SHANK && pa->aname == NAMESH)
			movealien(pa);
		else {
			pa->cx = pa->cx + pa->cix;
			pa->cy = pa->cy + pa->ciy;
		}
	}
	/*
	 * Update motion of torpedoes.
	 * When looking for hits, try every point of movement.
	 */
	for (pt = torps; pt < &torps[NTORP]; pt++) {
	    int fromx, fromy;
	    int tdx, tdy, apa;
	    int tdiffx, tdiffy;

	    if (pt->owner == 0)
		continue;
	    tdx = sign(pt->xinc); tdy = sign(pt->yinc);
	    tdiffx = pt->xinc*sign(tdx);
	    tdiffy = pt->yinc*sign(tdy);
	    pt->torx += pt->xinc;
	    pt->tory += pt->yinc;
	    pt->tcount--;
	    /*
	     * Check for alien hits
	     */
	    for (pa = alien; pa < &alien[NALIEN]; pa++) {
		int numit;
		if (pa->stats == DEAD)
		    continue;
		fromx = pt->torx - pt->xinc;
		fromy = pt->tory - pt->yinc;
		apa = tdiffy - tdiffx; numit = 30;
		do {
		    if (numit-- == 0) {
			exit(1);
			break;
		    }
		    if (apa < 0) {
			fromx += tdx; apa += tdiffy;
		    } else if (apa > 0) {
			fromy += tdy; apa -= tdiffx;
		    } else {
			fromy += tdy; fromx += tdx;
			apa = tdiffy-tdiffx;
		    }
		    if (pa->cx == fromx && pa->cy == fromy) {
			pa->hit++;
			if (pa->hit > 5) {
			    makeburst(pa->cx, pa->cy);
			    pa->stats = DEAD;
			}
			/* shot by a shanker */
			if(isalien(pt->owner)) break;
			pl = (t_player *)pt->owner;
			/* 6 points for any hit on an alien */
			pl->points += 6; pl->ahits++;
			pl->maxmaxe += 1 + (pa->stats == DEAD);
			/* X-aliens are worth 20 points */
			if (pa->type == WANDER) pl->points += 20;
			/*
			 * move off in a random direction
			 *  after a hit
			 */
			pa->cix = (rand()%3)-1;
			pa->ciy = (rand()%3)-1;
			if (pa->cix == 0 && pa->ciy == 0)
			    pa->cix = pa->ciy = 1;
			/*
			 * Hit a shanker -- they fight back
			 */
			if (pa->type == SHANK) {
			    pa->aname = NAMESH;
			    pa->whotoget = pt->owner;
			    pl->points += 20; pa->count = 0;
			    pmesg(pl, "From a shanker-- bonzai!");
			}
			pstatus(pl, "\007Torpedo hit on alien!");
			pt->owner = 0; break;
		    }
		} while (fromx != pt->torx && fromy != pt->tory);
	    }
	    if (pt->tcount == 0) pt->owner = 0;
	}
	/*
	 * Main screen update loop...
	 */
	for (p = player; p < &player[NPLAYER]; p++) {
	    if (p->status.alive == FALSE)
		    continue;
	    wholist = p;
	    x = p->curx;
	    y = p->cury;
	    mf = p->mflg;
	    p->whocent = 0;
	    dx = x-ppx;
	    dy = y-ppy;
	    /*
	     * Display any portion of Quartone visible
	     */
	    if (dx > -18 && dx < 18 && dy > -10 && dy < 10) {
		    register int pp;

		    for (pp = 0; pp != 17; pp++) {
			    dx = planet.places[pp][0];
			    dy = planet.places[pp][1];
			    if (!onscreen (dx, dy, x, y))
				    continue;
			    if (dx == x && dy == y)
				    p->status.crash = TRUE;
			    xx = (dx-x)/mf+XWIND;
			    yy = (dy-y)/mf+YWIND;
			    enteritem(xx, yy, NAMEP);
		    }
	    }
	    /*
	     * Shrapnel around?
	     */
	    for (pb = bursts; pb < &bursts[NBURST]; pb++) {
		    if (pb->cbactive == 0)
			    continue;
		    dx = x-pb->cbx;
		    dy = y-pb->cby;
		    if (dx > -21 && dx < 21 && dy > -13 && dy < 13) {
			    register int is;

			    for (is = 0; is != 9; is++) {
				    dx = pb->shrap[is][0];
				    dy = pb->shrap[is][1];
				    if (dx == 0 && dy == 0)
					    continue;
				    if (!onscreen (dx, dy, x, y))
					    continue;
				    /* a piece hit -- mark it */
				    if (dx == x && dy == y)
					    p->status.bur = TRUE;
				    xx = (dx-x)/mf+XWIND;
				    yy = (dy-y)/mf+YWIND;
				    enteritem(xx, yy, '.');
			    }
		    }
	    }
	    /*
	     * Any star bases visible?
	     */
	    for (ps = sbase; ps < &sbase[NBASE]; ps++) {
		    if (!onscreen (ps->xpos, ps->ypos, x, y))
			    continue;
		    if (ps->xpos == x && ps->ypos == y)
			    p->whocent = (thing *)ps;
		    xx = (ps->xpos-x)/mf+XWIND;
		    yy = (ps->ypos-y)/mf+YWIND;
		    enteritem(xx, yy, '*');
	    }
	    /*
	     * Show other players -- take care of groups
	     */
	    for (pl = player; pl < &player[NPLAYER]; pl++) {
		    register int shift;

		    if (pl == p)
			    continue;
		    if (pl->status.alive == FALSE)
			    continue;
		    if (!onscreen (pl->curx, pl->cury, x, y))
			    continue;
		    if (pl->status.invis == TRUE) {
			    if (!visual[pl-player][p-player])
				    continue;
			    shift = 'a'-'A';
		    } else
			    shift = 0;
		    if (pl->curx == x && pl->cury == y) {
			    p->whocent = (thing *)pl;
			    pl->whocent = (thing *)p;
		    }
		    xx = (pl->curx-x)/mf+XWIND;
		    yy = (pl->cury-y)/mf+YWIND;
		    c = (pl-player)+'A'+shift;
		    enteritem(xx, yy, c);
	    }
	    /*
	     * Show visible aliens...
	     */
	    for (pa = alien; pa < &alien[NALIEN]; pa++) {
		    if (pa->stats == DEAD)
			    continue;
		    if (!onscreen (pa->cx, pa->cy, x, y))
			    continue;
		    if (pa->cx == x && pa->cy == y)
			    p->whocent = (thing *)pa;
		    xx = (pa->cx-x)/mf+XWIND;
		    yy = (pa->cy-y)/mf+YWIND;
		    enteritem(xx, yy, pa->aname);
		    if (pa->type == SHANK && (t_player *)(pa->whotoget) == p)
			    pa->onscr = ON;
	    }
	    /*
	     * Track torpedoes...
	     */
	    for (pt = torps; pt < &torps[NTORP]; pt++) {
		    if (pt->owner == 0)
			    continue;
		    if (!onscreen (pt->torx, pt->tory, x, y))
			    continue;
		    if (pt->torx == x && pt->tory == y)
			    p->whocent = (thing *)pt;
		    xx = (pt->torx-x)/mf+XWIND;
		    yy = (pt->tory-y)/mf+YWIND;
		    enteritem(xx, yy, '+');
	    }
	    /*
	     * Update the display -- attempts minimal changes
	     */
	    uplst();
	    if (p->offx != 0)
		    cput(POS1DATA, p, "%d", p->curx); 
	    if (p->offy != 0)
		    cput(POS2DATA, p, "%d", -p->cury);
	    if (p->status.bur == TRUE) {	/* shrapnel damage */
		    pmesg(p, "From damage control-- Shrapnel!!");
		    cput(INDATA, p, "off");
		    p->status.bur = FALSE;
		    p->status.invis = FALSE;
		    p->energy -= SHRAPCOST;
		    if (p->energy < 0)
			    p->energy = 0;
		    p->maxe -= SHRAPCOST;
	    }
	    /*
	     * Check for damage
	     */
	    if (isalien(p->whocent) && ((t_alien *)p->whocent)->type == WANDER) {
		    /*
		     * X-alien collision
		     */
		    p->maxe = p->maxe-15;
		    if (p->energy > p->maxe)
			    p->energy = p->maxe;
		    pmesg(p, "From engine room-- Collision!!");
	    }
	    /*
	     * Torpedo hit, and not in orbit
	     */
	    if (istorp(p->whocent) &&
		(gutsflag || p->status.orb == FALSE)) {
		pt = (t_torps *)p->whocent;
		/*
		 * If it's a shanker, let it get back to
		 *  wandering -- it's done its job
		 */
		if (isalien(pt->owner)) {
		    phaser((t_alien *)pt->owner, p);
		    pt->owner = 0;
		    continue;
		}
		/*
		 * Torpedo hit by a player
		 */
		pl = (t_player *)pt->owner;
		if (pl == p) {
		    pt->owner = 0;
		    pmesg(p, "From damage control-- SELF HIT!!!");
		    p->points -= 75;
		    p->maxmaxe--;
		    if (p->points < 0)
			p->points = 0;
		    continue;
		}
		pt->owner = 0;
		pstatus(pl, "\007Hit on %c", (p-player)+'A');
		pstatus(p, "\007Torpedo damage suffered!");
		pl->points += 50; pl->phits++; pl->maxmaxe++;
		p->maxe -= 50;
		if (p->energy > p->maxe) p->energy = p->maxe;
		if (p->maxe <= 0) {
		    int	booty = p->points / 5;
		    pmesg(p, "From damage control-- FATAL HIT!");
		    pmesg(pl, "From navigation-- TOTAL DESTRUCTION!");
		    pl->points += 5 + booty;
		    p->points -= booty;
		    pl->maxmaxe++; pl->pkills++;
		    /*
		     * Time for a new scab??
		     */
		    if ((pl->pkills%4) == 0)
			    makescab(pl);
		    makeburst(p->curx, p->cury);
		    p->status.killed = TRUE;
		    done(p);
		}
	    }
	}
	/*
	 * trying to minimize the number of writes to the sockets.
	 */
	for (p = player; p < &player[NPLAYER]; p++) {
	    if (p->status.alive == TRUE)
		    bflush(p);
	}
    }
}

energycost(dx, dy)
register dx, dy;
{
    dx <<= 1; dy <<= 1;
    if (dx<0) dx = -dx;
    if (dy<0) dy = -dy;

    if (dx>dy) return dx+dy/2;
    else return dy+dx/2;
}

friction(n)
{
    return n - n/4;
}

max(a,b)
{
    return a>b ? a : b;
}

sign(n)
int n;
{
    if (n<0) return -1;
    else return n ? 1 : 0;
}
