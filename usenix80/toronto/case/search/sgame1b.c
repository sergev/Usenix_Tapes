
/*
 * search, Greg Ordy
 * This file contains the code to execute some of the
 * players commands, as well as some utility routines
 * Copyright (c) 1978
 */

#include	"sgame.h"
int     TDIST;
char	devname[]	{"/dev/tty "};
#define		pntfile		"/nmpc/usr/ordy/bin/spoint"

movealien (who)
{

    register struct alien  *pnt;
    register struct player *plpnt;
    char    dx,
            dy;
    pnt = &alien[who];
    pnt -> count--;

    if (pnt -> onscr == OFF || pnt -> whotoget == 0)
    {
	if (pnt -> count == 0)
	{
	    pnt -> count = rand () % 20 + 100;
	    pnt -> cix = rand () % 3 - 1;
	    pnt -> ciy = rand () % 3 - 1;
	    if (pnt -> cix == 0 && pnt -> ciy == 0)
		pnt -> ciy = -1;
	}
	pnt -> cx =+ pnt -> cix;
	pnt -> cy =+ pnt -> ciy;
	return;
    }

    plpnt = &player[pnt -> whotoget - POFFSET];
    pnt -> onscr = OFF;
    if (pnt -> count <= 0)
    {
	pnt -> count = 10;
	dx = plpnt -> curx - pnt -> cx;
	dy = plpnt -> cury - pnt -> cy;
	if (dx < 9 && dx > -9 && dy < 5 && dy > -5)
	{
	    pnt -> fx = plpnt -> offx;
	    pnt -> fy = plpnt -> offy;
	    if (plpnt -> status == DEAD)
	    {
		pnt -> whotoget = 0;
		return;
	    }
	    if (dx == 0)
	    {
		if (dy > 0)
		    launch (who, SO, KOFFSET, pnt->fx, pnt->fy, 0, pnt->cx, pnt->cy, 0);
		else
		    launch (who, UP, KOFFSET, pnt->fx, pnt->fy, 0, pnt->cx, pnt->cy, 0);
		psycho(who);
		goto ma1;
	    }
	    if (dx == dy)
	    {
		if (dx > 0)
		    launch (who, SE, KOFFSET, pnt->fx, pnt->fy, 0, pnt->cx, pnt->cy, 0);
		else
		    launch (who, NW, KOFFSET, pnt->fx, pnt->fy, 0, pnt->cx, pnt->cy, 0);
		psycho(who);
		goto ma1;
	    }
	    if (dx == -dy)
	    {
		if (dx > 0)
		    launch (who, NE, KOFFSET, pnt->fx, pnt->fy, 0, pnt->cx, pnt->cy, 0);
		else
		    launch (who, SW, KOFFSET, pnt->fx, pnt->fy, 0, pnt->cx, pnt->cy, 0);
		psycho(who);
		goto ma1;
	    }
	    if (dy == 0)
	    {
		if (dx > 0)
		    launch (who, EA, KOFFSET, pnt->fx, pnt->fy, 0, pnt->cx, pnt->cy, 0);
		else
		    launch (who, WE, KOFFSET, pnt->fx, pnt->fy, 0, pnt->cx, pnt->cy, 0);
		psycho(who);
		goto ma1;
	    }
    ma1: 
	    if (dx > 0)
	    {
		if (dy > 0)
		    pnt -> cx++;
		else
		    pnt -> cy--;
	    }
	    else
	    {
		if (dy > 0)
		    pnt -> cy++;
		else
		    pnt -> cx--;
	    }
	    return;
	}
	if (dx < 0)
	    pnt -> cx--;
	if (dx > 0)
	    pnt -> cx++;

	if (dy < 0)
	    pnt -> cy--;
	if (dy > 0)
	    pnt -> cy++;

    }
    if (plpnt -> offx > 3 || plpnt -> offx < -3)
	pnt -> cx =+ pnt -> cix;
    else
	pnt -> cx =+ plpnt -> offx;
    if (plpnt -> offy > 3 || plpnt -> offy < -3)
	pnt -> cy =+ pnt -> ciy;
    else
	pnt -> cy =+ plpnt -> offy;
}

psycho(who)
{
	register char	x,y;
	char ox,oy;
	if(((rand() >> 5) % 6) != 5)
		return;
	x = alien[who].cx;
	y = alien[who].cy;
	ox = alien[who].fx;
	oy = alien[who].fy;
	launch(who,SO,KOFFSET, ox, oy, 0, x, y, 8);
	launch(who,UP,KOFFSET, ox, oy, 0, x, y, 8);
	launch(who,SE,KOFFSET, ox, oy, 0, x, y, 8);
	launch(who,NW,KOFFSET, ox, oy, 0, x, y, 8);
	launch(who,NE,KOFFSET, ox, oy, 0, x, y, 8);
	launch(who,SW,KOFFSET, ox, oy, 0, x, y, 8);
	launch(who,EA,KOFFSET, ox, oy, 0, x, y, 8);
	launch(who,WE,KOFFSET, ox, oy, 0, x, y, 8);
}

nuke (who,c)
char c;
{

	register struct player *play;
	register char	x,y;
	char ox,oy;
	char	c1;
	static int	nukedir[9][4] = {
		{  0,-1, 1, 0},
		{ -1, 0, 1, 0},
		{ -1, 0, 0,-1},
		{  0,-1, 0, 1},
		{  0, 0, 0, 0},
		{  0,-1, 0, 1},
		{  0, 1, 1, 0},
		{ -1, 0, 1, 0},
		{ -1, 0, 0, 1}};
	c1 = c;
	play = &player[who];
	play ->cmdpend = 0;
	ox = play->offx;
	oy = play->offy;
	launch(who, c, POFFSET, ox, oy, 0, play->curx, play->cury, 10);
	if(play->nkcnt-- <= 0)
	{
	    play->nkcnt = 0;
	    return;
	}
	c = c - '1';
	if (c < 0 || c > 8)
		return;
	x = nukedir[c][0];
	y = nukedir[c][1];
	x += play->curx;
	y += play->cury;
	launch(who, c1, POFFSET, ox, oy, 0, x, y, 8);
	x = nukedir[c][2];
	y = nukedir[c][3];
	x += play->curx;
	y += play->cury;
	launch(who, c1, POFFSET, ox, oy, 1, x, y, 8);
}

startup (typeof)
{
    register int    i,
                    j,
                    k;
    for (i = 0; i != NPLAYER; i++)
	if (player[i].status == DEAD)
	    break;
    player[i].name = fileform.plname;
    player[i].ppid = fileform.plpid;
    player[i].ttype = (int) fileform.pltype;
    player[i].puid = fileform.pluid;
    player[i].status = ALIVE;
    devname[8] = player[i].name;
    player[i].fildes = open (devname, 2);
    if (typeof == NEW)
    {
	player[i].curx = rand () % 50 + 25;
	player[i].cury = rand () % 50 + 25;
    }
    else
    {
	player[i].curx = ppx + (rand () % 10) + 5;
	player[i].cury = ppy + (rand () % 10) + 5;
    }
    player[i].offx = 0;
    player[i].offy = 0;
    player[i].mflg = 1;
    player[i].scabcount = 0;
    player[i].orb = 0;
    player[i].pltime = ptime;
    player[i].cmdpend = 0;
    player[i].nkcnt = 0;
    player[i].qkill = 0;
    player[i].pkill = 0;
    player[i].phits = 0;
    player[i].ahits = 0;
    player[i].crash = 0;
    player[i].energy = 250;
    player[i].maxe = 250;
    player[i].aflg = 0;
    player[i].points = 0;
    for (j = 0; j != 3; j++)
	player[i].home[j] = 0;
    if ((fileform.pluid & 0377) == SCAB)
	makescab (i);
    return (i);
}

rmsg (who, letter)
char    letter;
{
    register int    i,
                    j,
                    k;
    char    c;
    if (r_owner != (who + POFFSET) && r_dst != 0)
    {
	cursor (STDATA, who);
	rite ("Wait for interference to clear!    ", 30);
	player[who].cmdpend = 0;
	bflush (who);
	return;
    }
    r_owner = who + POFFSET;
    if (r_dst == 0)
    {
	for (k = 0; k != NPLAYER; k++)
	{
	    if (player[k].name == letter && player[k].status != DEAD)
	    {
		r_dst = letter;
		return;
	    }
	}
	cursor (STDATA, who);
	rite ("Inactive channel requested!           ", 30);
	player[who].cmdpend = 0;
	bflush (who);
	return;
    }
    if (letter != '\n')
    {
	if (letter == SCABLETTER)
	{
	    letter = '^';
	    makescab (who);
	}
	if(player[who].cmdpend > 35)
	    return;
	*rmp++ = letter;
	player[who].cmdpend++;
	return;
    }
    r_owner = 0;
    j = player[who].cmdpend - 1;
    k = 40 - j;
    if (k < 1)
	k = 1;
    c = player[who].name - 040;
    for (i = 0; i != NPLAYER; i++)
    {
	if (player[i].name == r_dst)
	{
	    cursor (MSDATA, i);
	    rite ("Personal from ", 14);
	    rite (&c, 1);
	    rite ("--", 2);
	    rite (&rmessb[0], j);
	    rite ("                                                ", k);
	    bflush (i);
	    player[who].cmdpend = 0;
	    cursor (MSDATA, who);
	    rite ("From radio room-- Message sent!               ", 35);
	    bflush (who);
	    rmp = rmessb;
	    r_dst = 0;
	    return;
	}
    }
    player[who].cmdpend = 0;
    rmp = rmessb;
    r_dst = 0;
}
bflush (who)
{
    write (player[who].fildes, queue, qlen);
    qlen = 0;
}
rite (where, howmany)
char   *where;
{
    register char  *p1,
                   *p2,
                    count;
    p1 = where;
    p2 = &queue[qlen];
    qlen =+ howmany;
    for (count = 0; count != howmany; count++)
	*p2++ = *p1++;
}
cursor (x, y, who)
{
    //where type is 1 - tektronics 4023 2 - beehive 3 - adds 4 - adm3
	char    buf[10];
    switch (player[who].ttype)
    {
	case 1: 
	    buf[0] = '\034';
	    buf[1] = x + 32;
	    buf[2] = y + 32;
	    rite (buf, 3);
	    break;
	case 2: 
	    buf[0] = '\033';
	    buf[1] = 'F';
	    buf[2] = '0';
	    buf[3] = x / 10 + '0';
	    buf[4] = x % 10 + '0';
	    buf[5] = '0';
	    buf[6] = y / 10 + '0';
	    buf[7] = y % 10 + '0';
	    rite (buf, 8);
	    break;
	case 3: 
	    buf[0] = '\033';
	    buf[1] = 'Y';
	    buf[2] = y + 32;
	    buf[3] = x + 32;
	    rite (buf, 4);
	    break;
	case 4: 
	    buf[0] = '\033';
	    buf[1] = '=';
	    buf[2] = y + 32;
	    buf[3] = x + 32;
	    rite (buf, 4);
	    break;
	default: 
	    write (2, "bad terminal type to cursor\n", 26);
    }
}

magnif (who, power)
char    power;
{
    register struct player *pnt;
    pnt = &player[who];
    pnt -> cmdpend = 0;
    if (power < '1' || power > MAXMAG)
    {
	cursor (STDATA, who);
	rite ("M-factor request out of range!                   ", 35);
	bflush (who);
	return;
    }
    pnt -> mflg = power - '0';
    cursor (MFDATA, who);
    rite (&power, 1);
    bflush (who);
}
makeflare ()
{
    register int    i,
                    j;
    if (sfflag == ON)
    {
	sfcount = 500 + (rand () % 128);
	sfflag = OFF;
	TDIST = TSIZE - (2 * nplayer);
	return;
    }
    sfflag = ON;
    for (i = 0; i != NPLAYER; i++)
    {
	if (player[i].status == DEAD)
	    continue;
	cursor (INDATA, i);
	rite ("off", 3);
	cursor (MSDATA, i);
	rite ("\07From radar-- Solar flare starting!!!           ", 35);
	bflush (i);
	player[i].status = ALIVE;
    }
    sfcount = 30 + (rand () % 100);
}
makeentry (who)
{
    register struct player *pnt;
    struct
    {
	int     playeruid;
	        long playertime;
	int     playerkills;
	int     playerhits;
	int     playerahits;
	        long playerpoints;
    }
            pentry;
    pnt = &player[who];
    if ((pntfd = open (pntfile, 2)) > 0)
    {
	seek (pntfd, 0, 2);
	pentry.playeruid = pnt -> puid;
	pentry.playertime = ptime - pnt -> pltime;
	pentry.playerkills = pnt -> pkill;
	pentry.playerhits = pnt -> phits;
	pentry.playerahits = pnt -> ahits;
	pentry.playerpoints = pnt -> points;
	write (pntfd, &pentry, sizeof pentry);
	close (pntfd);
    }
}
makescab (who)
int     who;
{
    register int    i,
                    j,
                    k;
    player[who].scabcount++;
    if (player[who].scabcount > 2)
	enterscab (who);
    for (i = 0; i != NALIEN; i++)
    {
	if (alien[i].type != SHANK)
	    continue;
	alien[i].aname = NAMESH;
	alien[i].whotoget = who + POFFSET;
    }
    whoscab = who + POFFSET;
}
seescab (who)
int     who;
{
    register int    i;
    char    c;

    cursor (STDATA, who);
    if (whoscab == 0)
    {
	rite ("Currently no SCAB.                                     ", 40);
	bflush (who);
	return;
    }
    rite ("Current SCAB-- ", 15);
    c = player[whoscab - POFFSET].name - 040;
    rite (&c, 1);
    rite ("                                            ", 25);
    bflush (who);
}
enterscab (who)
int     who;
{
    register int    i;
}
makeburst (bx, by)
char    bx,
        by;
{
    int     i;
    register struct bursts *bp;
    for (i = 0; i != NBURST; i++)
    {
	if (bursts[i].cbactive != 0)
	    continue;
	bp = &bursts[i];
	bp -> cbactive = 1;
	bp -> cbx = bx;
	bp -> cby = by;
	bp -> shrap[0][0] = bx;
	bp -> shrap[0][1] = by;
	bp -> cbcnt = 100;
	return;
    }
}
movebursts ()
{
    register int    i,
                    j;
    char    xi,
            yi,
            qrt;
    register struct bursts *bp;
    for (i = 0; i != NBURST; i++)
    {
	if (bursts[i].cbactive == 0)
	    continue;
	bp = &bursts[i];
	if (bp -> cbcnt == 100)
	{
	    bp -> cbcnt--;
	    break;
	}
	if (bp -> cbcnt == 98)
	{
	    bp -> cbcnt = 50;
	    for (j = 1; j != 4; j++)
	    {
		qrt = (rand () >> 4) % 8;
		xi = bp -> cbx;
		yi = bp -> cby;
		switch (qrt)
		{
		    case 0: 
			xi++;
			break;
		    case 1: 
			xi++;
			yi--;
			break;
		    case 2: 
			yi--;
			break;
		    case 3: 
			xi--;
			yi--;
			break;
		    case 4: 
			xi--;
			break;
		    case 5: 
			xi--;
			yi++;
			break;
		    case 6: 
			yi++;
			break;
		    case 7: 
			xi++;
			yi++;
			break;
		}
		bp -> shrap[j][0] = xi;
		bp -> shrap[j][1] = yi;
		bp -> shrapd[j] = qrt;
	    }
	    break;
	}
	if (bp -> cbcnt > 48)
	{
	    bp -> cbcnt--;
	    break;
	}
	if (bp -> cbcnt == 48)
	{
	    bp -> cbcnt = (rand () >> 4) % (16 - nplayer);
	    qrt = bp -> shrapd[1];
	    xi = bp -> shrap[1][0];
	    yi = bp -> shrap[1][1];
	    getdir (qrt, xi, yi);
	    bp -> shrap[0][0] = xxi;
	    bp -> shrap[0][1] = yyi;
	    getdir (qrt, xi, yi);
	    bp -> shrap[4][0] = xxi;
	    bp -> shrap[4][1] = yyi;
	    qrt = bp -> shrapd[2];
	    xi = bp -> shrap[2][0];
	    yi = bp -> shrap[2][1];
	    getdir (qrt, xi, yi);
	    bp -> shrap[5][0] = xxi;
	    bp -> shrap[5][1] = yyi;
	    getdir (qrt, xi, yi);
	    bp -> shrap[6][0] = xxi;
	    bp -> shrap[6][1] = yyi;
	    qrt = bp -> shrapd[3];
	    xi = bp -> shrap[3][0];
	    yi = bp -> shrap[3][1];
	    getdir (qrt, xi, yi);
	    bp -> shrap[7][0] = xxi;
	    bp -> shrap[7][1] = yyi;
	    getdir (qrt, xi, yi);
	    bp -> shrap[8][0] = xxi;
	    bp -> shrap[8][1] = yyi;
	    break;
	}
	bp -> cbcnt--;
	if (bp -> cbcnt == 3)
	{
	    for (j = 1; j != 4; j++)
	    {
		bp -> shrap[j][0] = 0;
		bp -> shrap[j][1] = 0;
	    }
	    bp -> shrap[0][0] = 0;
	    bp -> shrap[0][1] = 0;
	    break;
	}
	if (bp -> cbcnt == 0)
	{
	    for (j = 4; j != 9; j++)
	    {
		bp -> shrap[j][0] = 0;
		bp -> shrap[j][1] = 0;
	    }
	    bp -> cbactive = 0;
	    break;
	}
    }
}
getdir (dir, xi, yi)
char    dir,
        xi,
        yi;
{
    register char   dx,
                    dy;
    dx = ((rand () >> 3) % 3) - 1;
    dy = ((rand () >> 3) % 3) - 1;
    switch (dir)
    {
	case 0: 
	    if (dx < 0)
		dx = 2;
	    break;
	case 1: 
	    if (dx < 0)
		dx = 2;
	    if (dy > 0)
		dy = -2;
	    break;
	case 2: 
	    if (dy > 0)
		dy = -2;
	    break;
	case 3: 
	    if (dx > 0)
		dx = -2;
	    if (dy > 0)
		dy = -2;
	    break;
	case 4: 
	    if (dx > 0)
		dx = -2;
	    break;
	case 5: 
	    if (dx > 0)
		dx = -2;
	    if (dy < 0)
		dy = 2;
	    break;
	case 6: 
	    if (dy < 0)
		dy = 2;
	    break;
	case 7: 
	    if (dx < 0)
		dx = 2;
	    if (dy < 0)
		dy = 2;
	    break;
    }
    xxi = xi + dx;
    yyi = yi + dy;
}

facts (who)
{
    register    rate;
    register struct player *pnt;
    pnt = &player[who];
    rate = ((long) pnt -> points * 100 + (long) pnt -> phits * 100 + (long) pnt -> pkill * 1000) / (ptime - pnt -> pltime);
    cursor (STDATA, who);
    rite ("hits: ", 6);
    rite (itoa (pnt -> phits), 3);
    rite ("kills: ", 7);
    rite (itoa (pnt -> pkill), 3);
    rite ("level: ", 7);
    rite (itoa (rate), 3);
    rite ("                        ", 11);
    bflush (who);
}
