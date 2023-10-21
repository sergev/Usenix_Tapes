#
/*
 *
 * search: multiplayer chase and destroy game
 * Greg Ordy, Case Western Reserve University, 1978 
 * Copyright (c)
 *
 * This file contains some of the code to execute players
 * commands
*/

#include	"sgame.h"
#define		lock	"/tmp/slock"
int     TDIST TSIZE;
initplayer (who)
{
    register int    i,
                    j;
    char    c;

    i = who;

    eras (player[who].fildes, player[who].ttype);
    for (j = 0; j != 15; j++)
    {
	cursor (41, j, i);
	rite ("|", 1);
    }
    cursor (26, 7, i);
    rite ("--------------- ---------------", 31);
    cursor (POSTITLE, i);
    rite ("POSITION", 8);
    cursor (EGYTITLE, i);
    rite ("ENERGY", 6);
    cursor (EGYDATA, i);
    rite (itoa (player[i].energy), 4);
    cursor (HRTITLE, i);
    rite ("HOMING RADAR", 12);
    cursor (H1TITLE, i);
    rite ("1) ", 3);
    cursor (H2TITLE, i);
    rite ("2) ", 3);
    cursor (H3TITLE, i);
    rite ("3) ", 3);
    cursor (H1DATA, i);
    rite ("            ", 12);
    cursor (H2DATA, i);
    rite ("            ", 12);
    cursor (H3DATA, i);
    rite ("            ", 12);
    cursor (PTTITLE, i);
    rite ("POINTS", 6);
    cursor (PTDATA, i);
    rite (itoa (player[i].points), 4);
    cursor (STTITLE, i);
    rite ("STATUS: ", 8);
    cursor (MSTITLE, i);
    rite ("MESSAGE: ", 9);
    cursor (INTITLE, i);
    rite ("INVISIBILITY", 12);
    cursor (INDATA, i);
    if (player[i].status == INVIS)
	rite ("on ", 3);
    else
	rite ("off", 3);
    cursor (VLTITLE, i);
    rite ("VELOCITY", 8);
    cursor (TMTITLE, i);
    rite ("TIME", 4);
    cursor (MFTITLE, i);
    rite ("M-FACTOR", 8);
    cursor (MFDATA, i);
    c = player[i].mflg + '0';
    rite (&c, 1);
    bflush (i);
    player[i].plstp = 0;
}

outgame (who)
{
    player[who].status = DEAD;
    curs (STDATA, player[who].fildes, player[who].ttype);
    write (player[who].fildes, "Out of energy, out of game!   ", 30);
    done (who);
}

done (who)
{
    register int    i,
                    j;
    int     x,
            y,
            z;
    nplayer--;
    player[who].status = DEAD;
    curs (PTDATA, player[who].fildes, player[who].ttype);
    write (player[who].fildes, itoa (player[who].points), 4);
    kill (player[who].ppid, 9);
    for (x = 0; x != NPLAYER; x++)
    {
	visual[x][who] = 0;
	visual[who][x] = 0;
	if (player[x].status == DEAD)
	    continue;
	for (y = 0; y != 3; y++)
	{
	    if (player[x].home[y] == who + POFFSET)
		player[x].home[y] = 0;
	}
    }
    if (b_owner == who + POFFSET)
    {
	b_owner = 0;
	bbpnt = bbuffer;
    }
    if (whoscab - POFFSET == who)
	whoscab = 0;
    if (r_owner == who + POFFSET)
    {
	r_owner = 0;
	rmp = rmessb;
	r_dst = 0;
    }
    close (player[who].fildes);
    fnode (player[who].plstp);
    makeentry (who);
    i = 0;
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
    if (i != 0)
	return;
    for (i = 0; i != NPLAYER; i++)
	if (player[i].status != DEAD)
	    return;
    unlink (lock);
    exit ();
}
itoa (val)
{
    register char  *p;
    register int    j,
                    mflag;

    mflag = 1;
    for (j = 0; j < 11; j++)
	itoabuf[j] = ' ';
    p = itoabuf + 6;
    if (val == 0)
    {
	*p = '0';
	p--;
    }
    if (val < 0)
    {
	mflag = -1;
	val =* -1;
    }
    while (val != 0)
    {
	*p-- = val % 10 + 060;
	val =/ 10;
    }
    if (mflag == -1)
	*p-- = '-';
    return (++p);
}

launch (who, where, offset, ox, oy, mess, xl, yl,dd)

char    where,xl,yl,ox,oy;

{

    int     i;
    register char   x,
                    y;
    register struct torps  *tp;

    for (i = 0; i != NTORPS; i++)
    {
	if (torps[i].owner == 0)
	{
	    if (offset == POFFSET)
	    {
		if (player[who].energy < TPCOST)
		{

		    if (mess)
		    {
			    cursor (STDATA, who);
			    rite ("Need more energy!                                ", 35);
			    bflush (who);
			    player[who].cmdpend = 0;
		    }
		    return;
		}
	    }
	    tp = &torps[i];
	    tp -> owner = who + offset;
	    tp -> torx = xl;
	    tp -> tory = yl;
	    x = ox; y = oy;
	    if (dd != 0)
		tp -> tcount = dd;
	    else
		tp -> tcount = TDIST;
	    switch (where)
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
	    if (offset != POFFSET)
	    {
		tp -> xinc = x;
		tp -> yinc = y;
		if (x == 0 && y == 0)
		    tp -> owner = 0;
		return;
	    }
	    if (x == 0 && y == 0)
	    {
		cursor (STDATA, who);
		rite ("Torpedos must move!                  ", 30);
		bflush (who);
		tp -> owner = 0;
		player[who].cmdpend = 0;
		return;
	    }
	    tp -> xinc = x;
	    tp -> yinc = y;
	    player[who].energy = player[who].energy - TPCOST;
	    player[who].cmdpend = 0;
	    if (mess)
	    {
		    cursor (STDATA, who);
		    rite ("Launch!                                    ", 30);
		    bflush (who);
	    }
	    return;
	}
    }
    if (offset != POFFSET)
	return;
    cursor (STDATA, who);
    rite ("Temporary malfunction!             ", 30);
    bflush (who);
    player[who].cmdpend = 0;
}
home (who, channel)
char    channel;
{
    register char   x,
                    y,
                    val;

    player[who].cmdpend = 0;
    if (channel < '1' || channel > '3')
    {
	cursor (STDATA, who);
	rite ("Invalid channel number!                      ", 30);
	bflush (who);
	return;
    };
    channel =- '1';
    if (player[who].home[channel] == 0)
    {
	cursor (STDATA, who);
	rite ("Channel not locked on object!             ", 30);
	bflush (who);
	return;
    };
    switch (channel)
    {
	case 0: 
	    cursor (H1DATA, who);
	    break;
	case 1: 
	    cursor (H2DATA, who);
	    break;
	case 2: 
	    cursor (H3DATA, who);
	    break;
    };
    val = player[who].home[channel];
    if (val < POFFSET)
    {
	val = val - AOFFSET;
	putpos (who, alien[val].cx, alien[val].cy);
	return;
    };
    if (val < SOFFSET)
    {
	val = val - POFFSET;
	putpos (who, player[val].curx, player[val].cury);
	return;
    };
    if (val < TOFFSET)
    {
	val = val - SOFFSET;
	putpos (who, sbase[val].xpos, sbase[val].ypos);
	return;
    };
    val = val - TOFFSET;
    putpos (who, torps[val].torx, torps[val].tory);
}

putpos (who, x, y)

char    x,
        y;

{

    rite (itoa (x), 4);
    rite ("   ", 2);
    rite (itoa (y), 4);
    bflush (who);
}
sethome (who, channel)
char    channel;
{
    cursor (STDATA, who);
    player[who].cmdpend = 0;
    if (channel < '1' || channel > '3')
    {
	rite ("Invalid channel number!               ", 30);
	bflush (who);
	return;
    };
    channel =- '1';
    if (player[who].whocent == 0)
    {
	rite ("Not over an object!                  ", 30);
	bflush (who);
	return;
    };

    player[who].home[channel] = player[who].whocent;
    rite ("Locked on!                                      ", 30);
    bflush (who);
}
invisible (who)
{
    cursor (INDATA, who);
    if (player[who].status == ALIVE)
    {
	if (sfflag == ON)
	{
	    rite ("off", 3);
	    cursor (MSDATA, who);
	    rite ("From radar-- Solar flare up!!               ", 35);
	    bflush (who);
	    return;
	};
	rite ("on ", 3);
	player[who].status = INVIS;
    }
    else
    {
	rite ("off", 3);
	player[who].status = ALIVE;
    };
    bflush (who);
}

whoisin (who)
{
    register int    i;
    char    c;
    cursor (STDATA, who);
    rite ("  Alive: ", 9);
    for (i = 0; i != NPLAYER; i++)
    {
	if (player[i].status != DEAD)
	    c = player[i].name - 040;
	else
	    c = ' ';
	rite (&c, 1);
    };
    rite ("                                  ", 30 - NPLAYER);
    bflush (who);
}
dock (who)
{
    register char   cent;
    register struct player *pnt;

    pnt = &player[who];
    cent = pnt -> whocent;
    cursor (STDATA, who);
    if (cent < SOFFSET || cent >= TOFFSET)
    {
	rite ("Not over a starbase!                  ", 30);
	bflush (who);
	return;
    };
    cent = cent - SOFFSET;
    rite ("Successful docking!                            ", 30);
    if (pnt -> maxe <= 225)
	pnt -> maxe = pnt -> maxe + 25;
    pnt -> energy = pnt -> maxe;
    cursor (EGYDATA, who);
    rite (itoa (pnt -> energy), 4);
    bflush (who);
}
autopi (who, channel)
char    channel;
{
    register char   val;
    register char  *x,
                   *y;
    player[who].cmdpend = 0;
    cursor (STDATA, who);
    if (channel < '1' || channel > '3')
    {
	rite ("Invalid channel number!                ", 30);
	bflush (who);
	return;
    };
    channel =- '1';
    val = player[who].home[channel];
    if (val == 0)
    {
	rite ("Channel not locked on object!          ", 30);
	bflush (who);
	return;
    };
    if (val < POFFSET)
    {
	val =- AOFFSET;
	x = &alien[val].cx;
	y = &alien[val].cy;
    }
    else
	if (val < SOFFSET)
	{
	    val =- POFFSET;
	    x = &player[val].curx;
	    y = &player[val].cury;
	}
	else
	    if (val < TOFFSET)
	    {
		val =- SOFFSET;
		x = &sbase[val].xpos;
		y = &sbase[val].ypos;
	    }
	    else
	    {
		x = &planet.places[0][0];
		y = &planet.places[0][1];
	    }

    player[who].apx = x;
    player[who].apy = y;
    player[who].aflg = 1;
    player[who].orb = 0;
    rite ("Autopilot process begun!                  ", 30);
    bflush (who);
}
apilot (who)
{
    register char   dx,
                    dy;
    int     xplus,
            yplus;
    register struct player *pnt;
    xplus = 2;
    yplus = 2;
    pnt = &player[who];
    dx = *pnt -> apx - pnt -> curx;
    dy = *pnt -> apy - pnt -> cury;

    if (dx == 0 && dy == 0)
    {
	write (pnt -> fildes, "\007", 1);
	pnt -> aflg = 0;
    };
    if (dx < 3 && dx > -3)
	xplus = 1;
    if (dy < 3 && dy > -3)
	yplus = 1;

    if (dx < 0)
	pnt -> offx = -xplus;
    else
	if (dx > 0)
	    pnt -> offx = xplus;
	else
	    pnt -> offx = 0;
    if (dy < 0)
	pnt -> offy = -yplus;
    else
	if (dy > 0)
	    pnt -> offy = yplus;
	else
	    pnt -> offy = 0;
    pnt -> energy++;
}
bcast (who, letter)
char    letter;
{
    register int    i,
                    j,
                    k;
    char    c;
    if (bbpnt != bbuffer && b_owner != (who + POFFSET))
    {
	cursor (STDATA, who);
	rite ("Wait for a clear channel!             ", 30);
	player[who].cmdpend = 0;
	bflush (who);
	return;
    };
    b_owner = who + POFFSET;
    if (letter != '\n')
    {
	if (letter == SCABLETTER)
	{
	    makescab (who);
	    letter = '^';
	};
	if(player[who].cmdpend > 35)
	    return;
	*bbpnt++ = letter;
	player[who].cmdpend++;
	return;
    };
    b_owner = 0;
    j = player[who].cmdpend - 1;
    k = 40 - j;
    if (k < 1)
	k = 1;

    c = player[who].name - 040;
    for (i = 0; i != NPLAYER; i++)
    {
	if (player[i].status == DEAD)
	    continue;
	cursor (MSDATA, i);
	rite ("\007From ", 6);
	rite (&c, 1);
	rite ("--", 2);
	rite (&bbuffer[0], j);
	rite ("                                              ", k);
	bflush (i);
    };
    player[who].cmdpend = 0;
    bbpnt = bbuffer;
}
