#
/*
 * search : multiplayer search and destroy game
 * Greg Ordy, Case Western Reserve University, 1978
 * Copyright (c) 1978 Greg Ordy
 *
 * This file contains code to manipulate orbiting of quartone
*/

#include	"stuff.h"
#include	"sgame.h"

char 	omods[16][2] {		/* orbit increments */
	{-1, 0},
	{-1, 0},
	{-1, 1},
	{-1, 1},
	{ 1, 1},
	{ 1, 1},
	{ 1, 0},
	{ 1, 0},
	{ 1, 0},
	{ 1, 0},
	{ 1,-1},
	{ 1,-1},
	{-1,-1},
	{-1,-1},
	{-1, 0},
	{-1, 0}};

char	oentry[45]	{	/* orbital entry calculations */
	0x2c,
	0x1b,
	0x0b,
	0x08,
	0x07,
	0x06,
	0x05,
	0,
	0,
	0x1c,
	0x0b,
	0,0,0,0,0,
	0x05,
	0,
	0x0c,
	0,0,0,0,0,0,0,
	0x04,
	0x4d,
	0x0d,
	0,0,0,0,0,
	0x03,
	0,
	0x8e,
	0x0e,
	0x0f,
	0x00,
	0x01,
	0x02,
	0,
	0 };

orbmove(who)
{
	register struct player	*pnt;
	register int	i;

	pnt = &player[who];
	if(pnt->ocnt % 4 == 6) /* DEBUG, not correct, but table wrong */
	{
		i = (pnt->ocnt >> 2) & 017;
		pnt -> offx = omods[i][0];
		pnt -> offy = omods[i][1];
	}
	else 
	{
		pnt -> offx = 0;
		pnt -> offy = 0;
	}
	pnt->ocnt++;
}

orbit (who)
{
    register char   dx,
                    dy;
    int i;
    register struct player *plpnt;
    plpnt = &player[who];
    cursor (STDATA, who);
    if (plpnt -> offx < -1 || plpnt -> offx > 1 || plpnt -> offy < -1 || plpnt -> offy > 1)
    {
	rite ("Velocity too great!!                    ", 37);
	bflush (who);
	return;
    }
    dx = planet.places[8][0] - plpnt -> curx;
    dy = planet.places[8][1] - plpnt -> cury;
    if (dx < -4 || dx > 4 || dy < -2 || dy > 2)
    {
	rite ("Too far from planet to orbit!      ", 30);
	bflush (who);
	return;
    }
    plpnt -> orb = 1;
    plpnt -> offx = 0;
    plpnt -> offy = 0;
    dx = dx + 4;
    dy = dy + 2;
    i = dx + (9*dy);
    i = oentry[i];
    plpnt->curx =+ (i>>6) & 03;
    plpnt->cury =+ (i>>4) &03;
    plpnt -> ocnt = (i & 017) << 2;
    rite ("In orbit around:   ", 17);
    rite (planet.planetname, 15);
    bflush (who);
}
