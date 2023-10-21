#ifndef lint
static char rcsid[] = "$Header: util.c,v 2.2 85/08/06 22:29:53 matt Exp $";
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
 * utility routines
 *
 * Copyright (c) 1979
 *
 * $Log:	util.c,v $
 * Revision 2.2  85/08/06  22:29:53  matt
 * Change handling of "r", "b", "g", "j", "q" commands to
 * provide better feedback, using per-player message buffer.
 * 
 * Revision 2.1  85/04/10  17:32:11  matt
 * Major de-linting and minor restructuring.
 * 
 * Revision 1.4  85/02/24  22:52:07  matt
 * Make the select() polls into real polls by setting the timeouts
 * to zero.  This cuts down on context switches and speeds the game
 * up IMMENSELY!
 * 
 * Revision 1.3  85/02/11  12:44:13  matt
 * added GUTS mode
 * 
 * Revision 1.2  85/02/09  23:50:55  matt
 * Eliminated the dependence on the value of the mask after
 * select() times out.  Use the return value to distinguish
 * a timeout!!
 * 
 */

#include <stdio.h>
#include <time.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include "defines.h"
#include "structs.h"

struct message message;

void pmesg();
int sock;

/*
 * Out of energy, out of game...
 */
void
outgame(p)
register t_player *p;
{
	extern	void pstatus();
	void	done();

	p->status.alive = FALSE;
	pstatus(p, "Out of energy, out of game!");
	p->status.killed = TRUE;
	done(p);
}


/*
 * Cleanup after player leaves...
 */
void
done(p)
register t_player *p;
{
	extern	void	bflush(),
			cput(),
			fnode(),
			pldisplay(),
			putplayer();
	extern	t_player	player[NPLAYER];
	extern	char	visual[NPLAYER][NPLAYER];
	extern	t_player	*whoscab;
	extern	int	dtabsiz,	nplayer;
	register	t_player	*pl;
	register	int	i,	j;
	int		delay;
	char	junkbuf[4096];
	int	mask,	y;

	nplayer--;
	p->status.alive = FALSE;
	cput(PTDATA, p, "%d", p->points);
	move(0, 23, p);		/* lower left hand corner? */
	bflush(p);
	delay = 0;
	/*
	 * close down the socket:
	 * send an out-of-band message to the player,
	 * stop any more writes to the socket,
	 * read all the muck that's accumulated,
	 * then shut it down for good.
	 */
	message.mtype = p->uid+1;
	message.text[0] = '\0';
	msgsnd(sock, &message, 8+1, 0);
	j = p-player;
	for (i=0,pl=player; i<NPLAYER; i++,pl++) {
		visual[i][j] = visual[j][i] = 0;
		if (pl->status.alive == FALSE)
			continue;
		for (y = 0; y != 3; y++)
			if (pl->home[y] == (thing *)p)
				pl->home[y] = NOTHING;
	}
	if (whoscab == p)
		whoscab = NOBODY;
	(void) putplayer(p);
	(void) fnode(p->plstp);
	(void) pldisplay(p, p->status.killed ? "d" : "q");
}

/*
 * Time for a solar flare...
 */
void makeflare() {
	extern	t_player player[NPLAYER];
	extern	int sfflag,
		gutsflag,
		sfcount,
		nplayer,
		TDIST;
	extern	int rand();
	extern	void seeall();

	if (sfflag == ON) {
		sfcount = 500+(rand()%128);
		sfflag = OFF;
		TDIST = TSIZE-(2*nplayer);
		return;
	}
	sfcount = 30+(rand()%100);
	sfflag = ON;
	if (!gutsflag)
		seeall("\07From radar-- Solar flare starting!!!");
}

/*
 * Autopilot update routine
 */
void apilot(p)
register t_player *p;
{
	extern	void cput();
	register char dx, dy;
	int	xplus, yplus, cost;

	dx = *(p->apx) - p->curx;
	dy = *(p->apy) - p->cury;
	if (dx == 0 && dy == 0) {
		cput(VLDATAX, VLDATAY, p, "%d %d\07", 0, 0);
		p->status.ap = FALSE;
	}
	xplus = dx < 3 && dx > -3 ? 1 : 2;
	yplus = dy < 3 && dy > -3 ? 1 : 2;
	dx = dx < 0 ? -xplus : dx > 0 ? xplus : 0;
	dy = dy < 0 ? -yplus : dy > 0 ? yplus : 0;
	if (abs(dx - p->offx) > 1) dx = p->offx + sign(dx - p->offx);
	if (abs(dy - p->offy) > 1) dy = p->offy + sign(dy - p->offy);
	cost = 0;
	if (dx != p->offx && dy != p->offy) {
	    cost = energycost(dx, dy);
	    if (cost > p->energy) {
		pmesg(p, "Low on energy\n"); return;
	    }
	} else cost = 0;
	p->energy = p->energy+1-cost;
	p->offx = dx; p->offy = dy;
	if (cost) cput(VLDATA, p, "%d %d", p->offx, p->offy);
}

/*
 * errlog logs the errors found by other routines.
 * this is needed since the main program closes stdout
 * so we need to print out what's wrong into a file.
 *
 * Of course, if errlog isn't opened, then we'll never see
 * anything bad (except a core-dump of searchd).
 */

errlog(msg, x0, x1, x2, x3, x4, x5, x6, x7, x8, x9)
char *msg;
{
	extern	int	errfile;
	extern	long	lseek();
	char buff[200];

	if (errfile < 0) {
		signal(SIGQUIT, SIG_DFL);
		kill(getpid(), SIGQUIT);
	}
	sprintf(buff, msg, x0, x1, x2, x3, x4, x5, x6, x7, x8, x9);
	(void)lseek(errfile, 0L, 2);
	write(errfile, buff, strlen(buff));
}

/*
 * seeall makes everyone visible and sends them all the argument
 * string as a message.  This is used by makeflare and when
 * entering guts mode.
 */
void seeall(msg)
char	*msg;
{
	extern	t_player player[NPLAYER];
	extern	void cput();
	register t_player *p;

	for (p = player; p < &player[NPLAYER]; p++) {
		if (p->status.alive == FALSE)
			continue;
		cput(INDATA, p, "off");
		pmesg(p, msg);
		p->status.invis = FALSE;
	}
}

abs(x)
{
    return x<0 ? -x : x;
}
