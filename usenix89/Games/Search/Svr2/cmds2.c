#ifndef lint
static char rcsid[] = "$Header: cmds2.c,v 2.2 85/08/06 22:29:33 matt Exp $";
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
 * more routines to execute player commands
 *
 * Copyright (c) 1979
 *
 * $Log:	cmds2.c,v $
 * Revision 2.2  85/08/06  22:29:33  matt
 * Change handling of "r", "b", "g", "j", "q" commands to
 * provide better feedback, using per-player message buffer.
 * 
 * Revision 2.1  85/04/10  17:30:53  matt
 * Major de-linting and minor restructuring.
 * 
 */

#include "defines.h"
#include "structs.h"

static int nukedir[9][4] = {
	{  0,-1, 1, 0}, { -1, 0, 1, 0}, { -1, 0, 0,-1},
	{  0,-1, 0, 1}, {  0, 0, 0, 0}, {  0,-1, 0, 1},
	{  0, 1, 1, 0}, { -1, 0, 1, 0}, { -1, 0, 0, 1}
};

/*
 * Launch a nuke.
 */
void
nuke(p, c)
register t_player *p;
char c;
{
	extern	void launch();
	register char x,
		y;
	char	ox,
		oy,
		c1 = c;

	p->cmdpend = 0;
	ox = p->offx;
	oy = p->offy;
	launch((thing *)p, c, PLAYER, ox, oy, 0, p->curx, p->cury, 10);
	if(p->nkcnt-- <= 0) {
		p->nkcnt = 0;
		return;
	}
	c -= '1';
	if (c < 0 || c > 8)
		return;
	x = nukedir[c][0] + p->curx;
	y = nukedir[c][1] + p->cury;
	launch((thing *)p, c1, PLAYER, ox, oy, 0, x, y, 8);
	x = nukedir[c][2] + p->curx;
	y = nukedir[c][3] + p->cury;
	launch((thing *)p, c1, PLAYER, ox, oy, 1, x, y, 8);
}

/*
 * Send a radio message.
 */
void
rmsg(p, letter)
register t_player *p;
char letter;
{
	extern	void pstatus(),
		pmesg(),
		echomsg(),
		gathermsg(),
		donemsg(),
		makescab();
	extern	t_player player[NPLAYER];
	register int i;
	register t_player *pl;

	if (p->mesgdst == 0) {
		if (letter >= 'a')
			letter -= 'a';
		else if (letter >= 'A')
			letter -= 'A';
		else
			letter = NPLAYER;
		if (letter < 0 || letter >= NPLAYER)
			pmesg(p, "From radio room-- frequency jammed!");
		else if (player[letter].status.alive == TRUE) {
			p->mesgdst = letter+1;
			gathermsg(p);
			return;
		} else
			pstatus(p, "Inactive channel requested!");
		p->cmdpend = 0;
		p->mesgdst = 0;
		return;
	}
	if (letter != '\n' && letter != '\r') {
		if (letter == SCABLETTER) {
			letter = '^';
			makescab(p);
		}
		echomsg(p, letter);
	} else {
		pl = &player[p->mesgdst-1];
		if (pl->status.alive != TRUE)
			pmesg(p, "From radio room-- Too late!  %c is dead!",
				p->mesgdst-1+'A');
		else {
			pmesg(p, "From radio room -- Message sent!");
			pmesg(pl, "Private from %s-- %s", p->plname, p->mesgbuf);
		}

		p->cmdpend = 0;
		donemsg(p);
	}
}

/*
 * Change the magnification factor on the viewing port.
 */
void
magnif(p, power)
register t_player *p;
char power;
{
	extern	void pstatus(),
		cput();

	p->cmdpend = 0;
	if (power < '1' || power > MAXMAG) {
		pstatus(p, "M-factor request out of range!");
		return;
	}
	p->mflg = power-'0';
	cput(MFDATA, p, "%c", power);
}

/*
 * Show current facts about a player.
 */
void
facts(p)
register t_player *p;
{
	extern	void	pstatus();
	extern	long	ptime;
	long	rate;

	rate = ((long)p->points*100+(long)p->phits*100+
		(long)p->pkills*1000)/(ptime-p->pltime);
	pstatus(p, "hits: %dp %da kills: %d level: %d", p->phits,
		p->ahits, p->pkills, rate);
}

/*
 * Exploit aliens living on quartone so that nukes may be
 *  fired.
 */
void
xploit(p)
register t_player *p;
{
	extern	void pmesg(),
		makescab();
	register int i;

	if(p->status.orb == FALSE) {
		pmesg(p, "Must be in orbit around planet");
		return;
	}
	if(p->status.invis == TRUE) {
		pmesg(p, "Must be visible to exploit");
		return;
	}
	if(p->nkcnt >= p->maxmaxe/20) {
		pmesg(p, "Maximum modifications are complete");
		return;
	}
	p->energy -= 20;
	p->nkcnt++;
	i = ((rand()>>4)%40)+60;
	p->qkill += i;
	pmesg(p, "Dead: %d total: %d", i, p->qkill);
	/*
	 * ouch!  if we kill more than 8000, make the player a scab!
	 */
	if(p->qkill > 8000)
		makescab(p);
}
