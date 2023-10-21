#
/*
 * search: multiplayer search and destroy game
 * Greg Ordy, Case Western Reserve University, 1978
 * Copyright (c) 1978 Greg Ordy
 *
 * this file contains code involving quartone, initially the x command
*/

#include	"stuff.h"
#include	"sgame.h"

xploit(who)
{
	register struct player	*pnt;
	register int	i;
	pnt = &player[who];

	cursor(MSDATA,who);
	if(pnt->orb == 0)
	{
		rite("Must be in orbit around planet            ",35);
		bflush(who);
		return;
	}
	if(pnt->status != ALIVE)
	{
		rite("Must be visible to exploit                  ",35);
		bflush(who);
		return;
	}
	if(pnt->nkcnt >= 12)
	{
		rite("Maximum modifications are complete           ",35);
		bflush(who);
		return;
	}
	pnt->energy = pnt->energy - 20;
	pnt->nkcnt++;
	i = (rand() >> 4) % 40 + 60;
	pnt->qkill = pnt->qkill + i;
	rite("Dead: ",6);
	rite(itoa(i),3);
	rite("total: ",7);
	rite(itoa(pnt->qkill),5);
	rite("                   ",15);
	bflush(who);
	if(pnt->qkill > 8000)
		makescab(who);
}
