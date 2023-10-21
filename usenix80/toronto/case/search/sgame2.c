#
/*
 *
 * search: multiplayer chase and destroy game
 * Greg Ordy, Case Western Reserve University, 1978
 * Copyright (c)
 *
 * this file contains the main game iteration loop
*/

#include	"stuff.h"
#include	"sgame.h"
#define		MASTER  15
#define		scabfile	"/nmpc/usr/ordy/bin/sscab"
#define		lock		"/tmp/slock"
int     NALIENS NALIEN;
main (argc, argv)

int     argc;
char  **argv;
{

    int     i,
            j,
            k,
	    v;
    char    xx,
            yy;
    register char   x,
                    y;
    int     tvec[2];
    register int    mf;
    char    c;

    if ((lfd = open (lock, 0)) < 0)
    {
	write (2, "Improper use of game, use search invocation\n", 44);
	exit ();
    };
    SCAB = 0;
    if ((scabfd = open (scabfile, 2)) > 0)
    {
	read (scabfd, &scabchar, 1);
	SCAB = scabchar - '0';
	read (scabfd, &scabchar, 1);
	SCAB = SCAB * 10 + scabchar - '0';
	close (scabfd);
    };
    time (tvec);
    srand (tvec[1]);
    bbpnt = bbuffer;

    for (i = 0; i != NPLAYER; i++)
	player[i].status = DEAD;

    sleep (10);

    i = 0;
    while (read (lfd, &fileform, sizeof fileform) == sizeof fileform)
    {
	startup (NEW);
	i++;
    }

    nplayer = i;
    for (i = 0; i != NALIEN; i++)
    {
	alien[i].cx = rand () % 80 + 20;
	alien[i].cy = rand () % 50 + 25;
	alien[i].cix = rand () % 3 - 1;
	alien[i].ciy = rand () % 3 - 1;
	if (alien[i].ciy == 0 && alien[i].cix == 0)
	    alien[i].cix = alien[i].ciy = 1;
	alien[i].aname = NAMEAL;
	alien[i].type = NORM;
    }

    for (i = 0; i != NSHANK; i++)
    {
	alien[i].type = SHANK;
	alien[i].count = rand () % 30 + 15;
    }

    for (i = NSHANK; i != NSHANK + NWAND; i++)
    {
	alien[i].type = WANDER;
	alien[i].aname = NAMEWD;
	alien[i].count = rand () % 32 + 16;
    }

    for (i = 0; i != NBASE - 1; i++)
    {
	sbase[i].xpos = ((rand () % 50) + 25 * (i + 1)) % 120 + 3;
	sbase[i].ypos = ((rand () % 50) + 25 * (i + 1)) % 120 + 3;
    }

    planet.planetname = PLANET;
    planet.places[0][0] = (rand () % 30) + 30;
    planet.places[0][1] = (rand () % 30) + 30;
    planet.places[0][0] =* -1;
    planet.places[0][1] =* -1;
    for (i = 0; i != 5; i++)
    {
	planet.places[i][1] = planet.places[0][1];
	planet.places[i][0] = planet.places[0][0] + i;
    }
    for (i = 5; i != 12; i++)
    {
	planet.places[i][1] = planet.places[0][1] - 1;
	planet.places[i][0] = planet.places[0][0] + i - 6;
    }
    for (i = 12; i != 17; i++)
    {
	planet.places[i][1] = planet.places[0][1] - 2;
	planet.places[i][0] = planet.places[0][0] + i - 12;
    }
    ppx = planet.places[8][0];
    ppy = planet.places[8][1];

    sbase[NBASE - 1].xpos = ppx - (rand () % 20) - 4;
    sbase[NBASE - 1].ypos = ppy - (rand () % 20) - 4;
    for (i = 0; i != nplayer; i++)
    {
	if (player[i].status == DEAD)
	    continue;
	initplayer (i);
    }
    sfcount = 500 + (rand () % 100);
    sfflag = OFF;

    while (1)
    {

	ptime++;
	if (--sfcount == 0)
	{
	    makeflare ();
	    NALIENS = NALIEN - nplayer;
	}
	if (ptime % 256 == 0)
	{
	    while (read (lfd, &fileform, sizeof fileform) == sizeof fileform)
	    {
		j = startup (MIDDLE);
		nplayer++;
		initplayer (j);
		for (i = 0; i != NPLAYER; i++)
		{
		    if (player[i].status == DEAD || i == j)
			continue;
		    cursor (MSDATA, i);
		    rite ("From scanners-- NEW PLAYERS                      ", 35);
		    bflush (i);
		}
	    }
	}
	for (j = 0; j != NPLAYER; j++)
	{
	    pply = &player[j];
	    if (pply -> status == DEAD)
		continue;
	    if(pply->aflg)
		apilot(j);
	    if(pply->orb)
		orbmove(j);
	    x = pply -> offx;
	    y = pply -> offy;
	    if (empty (pply -> fildes))
		goto constvel;
	    read (pply -> fildes, &c, 1);
	    if (pply -> cmdpend > 0)
	    {
		switch (pply -> cmd)
		{
		    case 'h': 
			home (j, c);
			break;
		    case '5': 
		    case 't': 
			pply -> aflg = 0;
			launch (j, c, POFFSET, x, y, 1, pply -> curx, pply -> cury, 0);
			break;
		    case 'g':
			groupm (j, c);
			break;
		    case 'q':
			quitg (j, c);
			break;
		    case 'n':
			nuke (j, c);
			break;
		    case 'j':
			joing (j, c);
			break;
		    case 's': 
			sethome (j, c);
			break;
		    case 'b': 
			bcast (j, c);
			break;
		    case 'a': 
			autopi (j, c);
			break;
		    case 'r': 
			rmsg (j, c);
			break;
		    case 'm': 
			magnif (j, c);
			break;
		}
		goto constvel;
	    }
	    switch (c)
	    {
		case 03: 
		    if (whoscab - POFFSET == j)
		    {
			cursor (STDATA, j);
			rite ("Sorry SCAB!!!                                    ", 38);
			bflush (j);
			break;
		    }
		    done (j);
		    break;
		case 'e': 
		    cursor (EGYDATA, j);
		    rite (itoa (pply -> energy), 4);
		    bflush (j);
		    break;
		case 'p': 
		    cursor (PTDATA, j);
		    rite (itoa (pply -> points), 5);
		    bflush (j);
		    break;
		case 'f': 
		    facts (j);
		    break;
		case 'l':
		    lookg (j);
		    break;
		case 'x': 
		    xploit (j);
		    break;
		case '0': 
		    if (pply -> whocent == 0)
			break;
		    if (pply -> whocent < POFFSET)
		    {
			pply -> whocent = pply -> whocent - AOFFSET;
			alien[pply -> whocent].hit++;
			if (alien[pply -> whocent].hit > 25)
			    alien[pply -> whocent].stats = DEAD;
			alien[pply -> whocent].cix = -1;
			alien[pply -> whocent].ciy = rand () % 3 - 1;
			pply -> points = pply -> points + 4;
			write (pply -> fildes, "\007", 1);
		    }
		    break;
		case 'i': 
		    invisible (j);
		    break;
		case 'c': 
		    cursor (TMDATA, j);
		    rite (itoa ((int)   ptime), 5);
		    bflush (j);
		    break;
		case 'o': 
		    orbit (j);
		    break;
		case 'z': 
		    seescab (j);
		    break;
		case 'd': 
		    dock (j);
		    break;
		case 'v': 
		    cursor (VLDATA, j);
		    rite (itoa (pply -> offx), 4);
		    rite (itoa (pply -> offy), 3);
		    bflush (j);
		    break;
		case 'w': 
		    whoisin (j);
		    break;
		case '@': 
		    fnode (pply -> plstp);
		    initplayer (j);
		    break;
		case 'h': 
		case 't': 
		case 'b': 
		case 'a': 
		case 'r': 
		case 'm': 
		case '5': 
		case 's': 
		case 'q':
		case 'j':
		case 'g':
		case 'n':
		    pply -> cmdpend++;
		    pply -> cmd = c;
		    break;
		default: 
		    goto vel;
	    }
	    goto constvel;
    vel: 
	    pply -> aflg = 0;
	    pply -> orb = 0;
	    switch (c)
	    {
		case UP: 
		    y--;
		    break;
		case NE: 
		    x++;
		    y--;
		    break;
		case EA: 
		    x++;
		    break;
		case SE: 
		    x++;
		    y++;
		    break;
		case SO: 
		    y++;
		    break;
		case SW: 
		    x--;
		    y++;
		    break;
		case WE: 
		    x--;
		    break;
		case NW: 
		    x--;
		    y--;
		    break;
		case STOP: 
		    break;
	    }
	    pply -> offx = x;
	    pply -> offy = y;
    constvel: pply -> curx = pply -> curx + x;
	    pply -> cury = pply -> cury + y;
	    if ((x > 1 || x < -1) ||
		    (y > 1 || y < -1))
		pply -> energy--;
	    if (x == 0 && y == 0)
		pply -> energy++;
	    if (pply -> mflg > 1)
		pply -> energy =- pply -> mflg;
	    if (pply -> status == INVIS)
		pply -> energy = pply -> energy - INCOST;
	    if (pply -> energy > pply -> maxe)
		pply -> energy = pply -> maxe;
	    if (pply -> energy <= 0)
		outgame (j);
	    if (pply -> crash > 0)
	    {
		cursor (STDATA, j);
		rite ("Crash into planet--all is lost!!         ", 35);
		bflush (j);
		if (pply -> points >= 50)
		    pply -> points =- 50;
		done (j);
	    }

	}
	moveburst ();
	for (j = 0; j != NALIENS; j++)
	{
	    if (alien[j].stats == DEAD)
		continue;
	    palpt = &alien[j];
	    if (palpt -> type == WANDER)
	    {
		palpt -> count--;
		if (palpt -> count == 0)
		{
		    palpt -> cix = rand () % 3 - 1;
		    palpt -> ciy = rand () % 3 - 1;
		    palpt -> count = rand () % 50 + 30;
		}
	    }
	    if (palpt -> type == SHANK && palpt -> aname == NAMESH)
		movealien (j);
	    else
	    {
		palpt -> cx = palpt -> cx + palpt -> cix;
		palpt -> cy = palpt -> cy + palpt -> ciy;
	    }
	}

	for (tpdoes)
	{
	    if (torps[pp].owner == 0)
		continue;
	    ptppt = &torps[pp];
	    ptppt -> torx =+ ptppt -> xinc;
	    ptppt -> tory =+ ptppt -> yinc;
	    ptppt -> tcount--;
	    for (j = 0; j != NALIENS; j++)
	    {
		if (alien[j].stats == DEAD)
		    continue;
		palpt = &alien[j];
		if (palpt -> cx == ptppt -> torx &&
			palpt -> cy == ptppt -> tory)
		{
		    palpt -> hit++;
		    if (palpt -> hit > 25)
		    {
			makeburst (palpt -> cx, palpt -> cy);
			palpt -> stats = DEAD;
		    }
		    if (ptppt -> owner > TOFFSET)
			continue;
		    is = ptppt -> owner - POFFSET;
		    player[is].points =+ 6;
		    player[is].ahits++;
		    if (palpt -> type == WANDER)
			player[is].points =+ 14;
		    palpt -> cix = rand () % 3 - 1;
		    palpt -> ciy = rand () % 3 - 1;
		    if (palpt -> cix == 0 && palpt -> ciy == 0)
			palpt -> cix = palpt -> ciy = 1;
		    if (palpt -> type == SHANK)
		    {
			palpt -> aname = NAMESH;
			palpt -> whotoget = ptppt -> owner;
			player[is].points =+ 14;
			palpt -> count = 0;
			cursor (MSDATA, is);
			rite ("From a shanker-- bonsai!                         ", 35);
		    }
		    cursor (STDATA, is);
		    rite ("\007Torpedo hit on alien!                    ", 35);
		    bflush (is);
		    ptppt -> owner = 0;
		}
	    }
	    if (ptppt -> tcount == 0)
		ptppt -> owner = 0;
	}

	for (j = 0; j != NPLAYER; j++)
	{
	    if (player[j].status == DEAD)
		continue;
	    pply = &player[j];
	    wholist = j;
	    x = pply -> curx;
	    y = pply -> cury;
	    mf = pply -> mflg;
	    pply -> whocent = 0;
	    dx = x - ppx;
	    dy = y - ppy;
	    if (dx > -18 && dx < 18 && dy > -10 && dy < 10)
	    {
		for (pp = 0; pp != 17; pp++)
		{
		    dx = planet.places[pp][0];
		    dy = planet.places[pp][1];
		    if (!onscreen (dx, dy, x, y))
			continue;
		    if (dx == x && dy == y)
			pply -> crash++;
		    xx = (dx - x) / mf + XWIND;
		    yy = (dy - y) / mf + YWIND;
		    enteritem (xx, yy, NAMEP);
		}
	    }
	    for (pp = 0; pp != NBURST; pp++)
	    {
		if (bursts[pp].cbactive == 0)
		    continue;
		dx = x - bursts[pp].cbx;
		dy = y - bursts[pp].cby;
		if (dx > -21 && dx < 21 && dy > -13 && dy < 13)
		{
		    for (is = 0; is != 9; is++)
		    {
			dx = bursts[pp].shrap[is][0];
			dy = bursts[pp].shrap[is][1];
			if (dx == 0 && dy == 0)
			    continue;
			if (!onscreen (dx, dy, x, y))
			    continue;
			if (dx == x && dy == y)
			    pply -> bur++;
			xx = (dx - x) / mf + XWIND;
			yy = (dy - y) / mf + YWIND;
			enteritem (xx, yy, '.');
		    }
		}
	    }
	    for (sbases)
	    {
		pbase = &sbase[is];
		if (!onscreen (pbase -> xpos, pbase -> ypos, x, y))
		    continue;
		if (pbase -> xpos == x && pbase -> ypos == y)
		    pply -> whocent = is + SOFFSET;
		xx = (pbase -> xpos - x) / mf + XWIND;
		yy = (pbase -> ypos - y) / mf + YWIND;
		enteritem (xx, yy, '*');
	    }
	    for (players)
	    {
		if (ip == j)
		    continue;
		splpnt = &player[ip];
		if (splpnt -> status == DEAD)
		    continue;
		if (visual[ip][j])
		{
		    v = 0;
		    goto v1;
		}
		v = 040;
		if (splpnt -> status == INVIS)
		    continue;
		v1:
		if (!onscreen (splpnt -> curx, splpnt -> cury, x, y))
		    continue;
		if (splpnt -> curx == x && splpnt -> cury == y)
		{
		    pply -> whocent = ip + POFFSET;
		    splpnt -> whocent = j + POFFSET;
		}
		xx = (splpnt -> curx - x) / mf + XWIND;
		yy = (splpnt -> cury - y) / mf + YWIND;
		enteritem (xx, yy, splpnt -> name - v);
	    }
	    for (aliens)
	    {
		if (alien[ia].stats == DEAD)
		    continue;
		palpt = &alien[ia];
		if (!onscreen (palpt -> cx, palpt -> cy, x, y))
		    continue;
		if (palpt -> cx == x && palpt -> cy == y)
		    pply -> whocent = ia + AOFFSET;
		xx = (palpt -> cx - x) / mf + XWIND;
		yy = (palpt -> cy - y) / mf + YWIND;
		enteritem (xx, yy, palpt -> aname);
		if (palpt -> type == SHANK && palpt -> whotoget == j + POFFSET)
		    palpt -> onscr = ON;
	    }
	    for (tpdoes)
	    {
		if (torps[pp].owner == 0)
		    continue;
		if (!onscreen (torps[pp].torx, torps[pp].tory, x, y))
		    continue;
		if (torps[pp].torx == x && torps[pp].tory == y)
		    pply -> whocent = pp + TOFFSET;
		xx = (torps[pp].torx - x) / mf + XWIND;
		yy = (torps[pp].tory - y) / mf + YWIND;
		enteritem (xx, yy, '+');
	    }
	    uplst ();
	    if (pply -> offx != 0)
	    {
		cursor (POS1DATA, j);
		rite (itoa (player[j].curx), 4);
	    }
	    if (pply -> offy != 0)
	    {
		cursor (POS2DATA, j);
		rite (itoa (player[j].cury), 4);
	    }
	    if (pply -> bur != 0)
	    {
		cursor (MSDATA, j);
		rite ("From damage control-- Shrapnel!!          ", 36);
		cursor (INDATA, j);
		rite ("off", 3);
		pply -> bur = 0;
		pply -> status = ALIVE;
		pply -> energy =- SHRAPCOST;
		if (pply -> energy < 0)
		    pply -> energy = 0;
		pply -> maxe =- SHRAPCOST;
	    }
	    bflush (j);
	    if (pply -> whocent < POFFSET)
	    {
		if (alien[pply -> whocent - AOFFSET].type == WANDER)
		{
		    pply -> maxe = pply -> maxe - 15;
		    if (pply -> energy > pply -> maxe)
			pply -> energy = pply -> maxe;
		    cursor (MSDATA, j);
		    rite ("From engine room-- Collision!!           ", 35);
		    bflush (j);
		}
	    }
	    if (pply -> whocent >= TOFFSET && pply -> orb == 0)
	    {
		x = y = pply -> whocent - TOFFSET;
		if (torps[x].owner > TOFFSET)
		{
		    is = torps[x].owner - KOFFSET;
		    torps[x].owner = 0;
		    alien[is].whotoget = 0;
		    alien[is].cix++;
		    alien[is].ciy = rand () % 4 - 2;
		    pply -> maxe = pply -> maxe - 50;
		    if (pply -> maxe < pply -> energy)
			pply -> energy = pply -> maxe;
		    cursor (MSDATA, j);
		    rite ("Enjoy the shankers revenge!                       ", 35);
		    bflush (j);
		    continue;
		}
		x = torps[x].owner - POFFSET;
		if (x == j)
		{
		    cursor (MSDATA, j);
		    torps[y].owner = 0;
		    rite ("From damage control-- SELF HIT!!!         ", 35);
		    bflush (j);
		    pply -> points =- 75;
		    if (pply -> points < 0)
			pply -> points = 0;
		    continue;
		}
		torps[y].owner = 0;
		cursor (STDATA, x);
		rite ("\007Hit on ", 8);
		c = pply -> name - 040;
		rite (&c, 1);
		rite ("                                    ", 25);
		bflush (x);
		cursor (STDATA, j);
		rite ("\007Torpedo damage suffered!             ", 35);
		bflush (j);
		player[x].points = player[x].points + 50;
		player[x].phits++;
		pply -> maxe = pply -> maxe - 50;
		if (pply -> energy > pply -> maxe)
		    pply -> energy = pply -> maxe;
		if (pply -> maxe <= 0)
		{
		    cursor (MSDATA, j);
		    rite ("From damage control-- FATAL HIT!        ", 40);
		    bflush (j);
		    curs (MSDATA, player[x].fildes, player[x].ttype);
		    write (player[x].fildes, "From navigation-- TOTAL DESTRUCTION!     ", 38);
		    player[x].points = player[x].points + 5;
		    player[x].pkill++;
		    if (player[x].pkill % 4 == 0)
			makescab (x);
		    makeburst (pply -> curx, pply -> cury);
		    done (j);
		}
	    }
	}

    }
}
