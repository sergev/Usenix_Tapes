#
/*
 * search, Greg Ordy 
 * This file contains code to execute the "team" commands, initially
 * the l,q,j, and g commands
 * Copyright (c) 1979
*/

#include	"sgame.h"

lookg(who)
{
	register int	i;
	char		c;
	cursor(MSDATA,who);
	rite("From security-- group = ",24);
	for(i = 0; i != NPLAYER; i++)
	{
		if(player[i].status == DEAD)
		{
			rite(" ",1);
			continue;
		}
		if(visual[who][i])
		{
			c = player[i].name - 040;
			rite(&c,1);
		}
		else rite(" ",1);
	}
	rite("        ",6);
	bflush(who);
}

joing(who,person)
char	person;
{
	register int i;
	if(person == '\n')
	{
		player[who].cmdpend = 0;
		return;
	}
	for(i = 0; i != NPLAYER; i++)
	{
		if(player[i].status == DEAD || i == who)
			continue;
		if(player[i].name == person)
		{
			visual[who][i] = 1;
			return;
		}
	}
}

quitg(who,person)
char person;
{
	register int i;
	if(person == '\n')
	{
		player[who].cmdpend = 0;
		return;
	}
	if(person == '.')
	{
		for(i = 0; i != NPLAYER; i++)
			visual[who][i] = 0;
		player[who].cmdpend = 0;
		return;
	}
	for(i = 0; i != NPLAYER; i++)
	{
		if(player[i].status == DEAD || i == who)
			continue;
		if(player[i].name == person)
		{
			visual[who][i] = 0;
			return;
		}
	}
}

groupm(who,c)
char c;
{
	register struct player	*pnt;
	register int	i;
	char		cc;
	pnt = &player[who];
	if(c == '\n')
	{
		cc = pnt->name;
		for(i = 0; i != NPLAYER; i++)
		{
			if(visual[who][i] && player[i].status != DEAD)
			{
				cursor(MSDATA,i);
				rite("From ",5);
				rite(&cc,1);
				rite("-- ",3);
				rite(pnt->gmess,pnt->cmdpend - 1);
				bflush(i);
			}
		}
		pnt->cmdpend = 0;
		return;
	}
	if(c == SCABLETTER)
	{
		c = '^';
		makescab(who);
	}
	if(pnt->cmdpend > 35)
		return;
	pnt->gmess[pnt->cmdpend++ - 1] = c;
}
